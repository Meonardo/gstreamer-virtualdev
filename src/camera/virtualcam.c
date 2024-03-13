#include "shared-memory-queue.h"

#include "util/bmem.h"
#include "util/platform.h"
#include "util/threading.h"

#include "virtualcam.h"

struct virtualcam_data {
  video_queue_t* vq;
  volatile bool active;
  volatile bool stopping;
};

static void virtualcam_deactive(struct virtualcam_data* vcam) {
  video_queue_close(vcam->vq);
  vcam->vq = NULL;

  os_atomic_set_bool(&vcam->active, false);
  os_atomic_set_bool(&vcam->stopping, false);
}

void virtualcam_destroy(void* data) {
  struct virtualcam_data* vcam = (struct virtualcam_data*)data;
  video_queue_close(vcam->vq);
  bfree(data);
}

void* virtualcam_create() {
  struct virtualcam_data* vcam =
      (struct virtualcam_data*)bzalloc(sizeof(*vcam));
  return vcam;
}

bool virtualcam_start(void* data, uint32_t w, uint32_t h, uint16_t fps) {
  if (w == 0 || h == 0 || fps == 0) {
    blog(LOG_ERROR, "Invalid resolution or fps");
    return false;
  }

  struct virtualcam_data* vcam = (struct virtualcam_data*)data;
  uint64_t interval = 10000000ULL / fps;

  char res[64];
  snprintf(res, sizeof(res), "%dx%dx%lld", (int)w, (int)h, (long long)interval);

  char* res_file = os_get_config_path_ptr("obs-virtualcam.txt");
  os_quick_write_utf8_file_safe(res_file, res, strlen(res), false, "tmp", NULL);
  bfree(res_file);

  vcam->vq = video_queue_create(w, h, interval);
  if (!vcam->vq) {
    return false;
  }

  os_atomic_set_bool(&vcam->active, true);
  os_atomic_set_bool(&vcam->stopping, false);
  blog(LOG_INFO, "Virtual output started");

  return true;
}

void virtualcam_stop(void* data, uint64_t ts) {
  struct virtualcam_data* vcam = (struct virtualcam_data*)data;
  os_atomic_set_bool(&vcam->stopping, true);

  UNUSED_PARAMETER(ts);
}

void virtual_video(void* data, VideoFrame* frame) {
  struct virtualcam_data* vcam = (struct virtualcam_data*)data;

  if (!vcam->vq)
    return;

  if (!os_atomic_load_bool(&vcam->active))
    return;

  if (os_atomic_load_bool(&vcam->stopping)) {
    virtualcam_deactive(vcam);
    return;
  }

  video_queue_write(vcam->vq, frame->data, frame->linesize, frame->timestamp);
}
