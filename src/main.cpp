#include <Windows.h>

/// gstreamer headers
#include <gio/gio.h>
#include <glib.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <gst/gst.h>
#include <gst/rtsp/gstrtspmessage.h>
#include <gst/sdp/gstsdpmessage.h>
#include <gst/video/video-info.h>

#include <chrono>
#include <csignal>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <string>

#include "camera/virtualcam.h"

// Logging
#include "local-debug.h"

int log_level = LOG_VERB;
gboolean log_timestamps = TRUE;
gboolean log_colors = TRUE, disable_colors = FALSE;

// Application context
struct App {
  GMainContext* context = nullptr;
  GstElement* pipeline = nullptr;
  GMainLoop* loop = nullptr;
  GstState pipeline_state = GST_STATE_NULL;

  // Audio sink
  GstElement* audio_sink = nullptr;
  // Video sink
  GstElement* video_sink = nullptr;
  void* virtualcam = nullptr;
  GstVideoInfo* video_info = nullptr;

  // thread for the app
  std::unique_ptr<std::thread> thread = nullptr;
};

static void printe_register_elements(GstRank rank,
                                     GstElementFactoryListType type) {
  GList *elements, *l;
  elements = gst_element_factory_list_get_elements(type, rank);
  for (l = elements; l != nullptr; l = l->next) {
    GstPluginFeature* feature = GST_PLUGIN_FEATURE(l->data);
    GstElementFactory* factory =
        GST_ELEMENT_FACTORY(gst_plugin_feature_load(feature));
    if (factory) {
      LOGD("%s", GST_OBJECT_NAME(factory));
      gst_object_unref(factory);
    }
  }
  gst_plugin_feature_list_free(elements);
}

static bool update_pipeline_state(App* app, GstState new_state) {
  if (app->pipeline_state == new_state) {
    LOGI(
        "the original state is the same as the new state, no need to change\n");
    return true;
  }
  if (app->pipeline == nullptr) {
    LOGE("pipeline is null, please check again\n");
    return false;
  }

  // set the state of the pipeline to the desired state
  auto ret = gst_element_set_state(app->pipeline, new_state);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    LOGE("unable to set the pipeline to the state: %d, old state: %d\n",
         new_state, app->pipeline_state);
    return false;
  }
  LOGI("pipeline state changed to %s\n", gst_element_state_get_name(new_state));

  // remember the current state
  app->pipeline_state = new_state;

  return true;
}

static void on_bus_message(App* app, GstBus* bus, GstMessage* msg) {
  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR: {  // error message
      GError* err;
      gchar* debug_info;
      gchar* message_string;

      gst_message_parse_error(msg, &err, &debug_info);
      message_string = g_strdup_printf("error received from element %s: %s",
                                       GST_OBJECT_NAME(msg->src), err->message);
      g_clear_error(&err);
      g_free(debug_info);

      LOGE("app playing with error: %s\n", message_string);

      g_free(message_string);
      update_pipeline_state(app, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_EOS: {  // end-of-stream
      LOGW("end of stream reached.\n");
      update_pipeline_state(app, GST_STATE_NULL);

      break;
    }
    case GST_MESSAGE_STATE_CHANGED: {
      GstState old_state, new_state, pending_state;
      gst_message_parse_state_changed(msg, &old_state, &new_state,
                                      &pending_state);
      // only pay attention to messages coming from the pipeline, not its
      // children
      if (GST_MESSAGE_SRC(msg) == GST_OBJECT(app->pipeline)) {
        gchar* message = g_strdup_printf("pipeline state changed to %s",
                                         gst_element_state_get_name(new_state));
        LOGW("App %s [cb]\n", message);
        g_free(message);
      }
      break;
    }
    default:
      break;
  }
}

void get_video_info(App* app, GstSample* sample) {
  if (app->video_info != nullptr) {
    return;
  }

  GstVideoInfo video_info_;
  gst_video_info_init(&video_info_);

  GstCaps* caps = gst_sample_get_caps(sample);
  if (gst_video_info_from_caps(&video_info_, caps)) {
    LOGI("The video size of the buffer is %dx%d.\n", video_info_.width,
         video_info_.height);
  } else {
    LOGE("Could not get video info from caps.\n");
  }

  gst_caps_unref(caps);

  app->video_info = gst_video_info_copy(&video_info_);

  GstVideoFormat gst_format = GST_VIDEO_INFO_FORMAT(app->video_info);
  LOGI("The video format of the buffer is %s.\n",
       gst_video_format_to_string(gst_format));
}

// Video buffer callback from appsink element
static GstFlowReturn on_new_video_sample(GstElement* sink, App* app) {
  GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
  if (sample == nullptr) {
    LOGE("failed to get sample from appsink\n");
    return GST_FLOW_ERROR;
  }

  GstBuffer* buffer = gst_sample_get_buffer(sample);
  if (buffer == nullptr) {
    LOGE("failed to get buffer from sample\n");
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
    LOGE("failed to map buffer\n");
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  // get video info
  get_video_info(app, sample);

  guint8* data = map.data;
  gint width = app->video_info->width;
  gint height = app->video_info->height;
  guint stride_Y = width;        // Typically, stride for Y is equal to width
  guint stride_UV = height / 2;  // Typically, stride for UV is half of width

  // Access Y plane
  guint8* Y_plane = data;
  // Access UV plane
  guint8* UV_plane = data + stride_Y * height;
  // get PTS
  GstClockTime pts = GST_BUFFER_PTS(buffer);

  VideoFrame vf = {0};
  vf.data[0] = Y_plane;
  vf.data[1] = UV_plane;
  vf.linesize[0] = stride_Y;
  vf.linesize[1] = stride_UV;
  vf.timestamp = pts;

  // write to virtual camera module
  virtual_video(app->virtualcam, &vf);

  gst_buffer_unmap(buffer, &map);
  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

// Audio buffer callback from appsink element
static GstFlowReturn on_new_audio_sample(GstElement* sink, App* app) {
  GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
  if (sample == nullptr) {
    LOGE("failed to get sample from appsink\n");
    return GST_FLOW_ERROR;
  }

  GstBuffer* buffer = gst_sample_get_buffer(sample);
  if (buffer == nullptr) {
    LOGE("failed to get buffer from sample\n");
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  GstMapInfo map;
  if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
    LOGE("failed to map buffer\n");
    gst_sample_unref(sample);
    return GST_FLOW_ERROR;
  }

  gst_buffer_unmap(buffer, &map);
  gst_sample_unref(sample);

  return GST_FLOW_OK;
}

static int init(App* app) {
  gchar* video_pipeline = g_strdup_printf(
      "rtspsrc location=rtsp://172.16.30.55/1 latency=50 protocols=4 ! queue "
      "! rtph264depay ! h264parse ! queue ! d3d11h264dec ! queue ! "
      "video/x-raw,format=NV12,width=1920,height=1080,framerate=30/1 ! queue ! "
      "appsink name=videosink");
  const gchar* audio_pipeline =
      "audiotestsrc is-live=true wave=sine ! audioconvert ! queue ! appsink "
      "name=audiosink";

  auto pipeline_desc = g_strdup_printf("%s %s", video_pipeline, audio_pipeline);
  g_free(video_pipeline);

  // create the pipeline
  GError* error = nullptr;
  app->pipeline = gst_parse_launch(pipeline_desc, &error);
  if (app->pipeline == nullptr || error != nullptr) {
    g_free(pipeline_desc);
    LOGE("failed to create pipeline, erorr: %s\n", error->message);
    return -11;
  }
  LOGI("Running pipeline:\n\n%s\n\n", pipeline_desc);
  g_free(pipeline_desc);

  // register appsink callback
  // video
  app->video_sink = gst_bin_get_by_name(GST_BIN(app->pipeline), "videosink");
  if (app->video_sink == nullptr) {
    LOGE("app sink not found\n");
    return -12;
  }
  g_object_set(app->video_sink, "emit-signals", TRUE, nullptr);
  g_object_set(app->video_sink, "max-buffers", 1, NULL);
  g_object_set(app->video_sink, "drop", TRUE, NULL);
  g_signal_connect(app->video_sink, "new-sample",
                   G_CALLBACK(on_new_video_sample), app);

  // audio
  app->audio_sink = gst_bin_get_by_name(GST_BIN(app->pipeline), "audiosink");
  if (app->audio_sink == nullptr) {
    LOGE("app sink not found\n");
    return -12;
  }
  g_object_set(app->audio_sink, "emit-signals", TRUE, nullptr);
  g_object_set(app->audio_sink, "max-buffers", 30, NULL);
  g_object_set(app->audio_sink, "drop", TRUE, NULL);
  g_signal_connect(app->audio_sink, "new-sample",
                   G_CALLBACK(on_new_audio_sample), app);

  // set the pipeline to READY and return
  if (!update_pipeline_state(app, GST_STATE_READY)) {
    LOGE("failed to set pipeline to ready state\n");
    return -13;
  }
  return 0;  // success
}

static void deinit(App* app) {
  // quit main loop
  g_main_loop_quit(app->loop);

  if (app->thread != nullptr && app->thread->joinable()) {
    app->thread->join();
  }

  // reset the pipeline state to NULL
  update_pipeline_state(app, GST_STATE_NULL);
  // free resources
  g_main_loop_unref(app->loop);
  app->loop = nullptr;
  gst_object_unref(app->video_sink);
  app->video_sink = nullptr;
  gst_object_unref(app->pipeline);
  app->pipeline = nullptr;
}

static void run(App* app) {
  // init thread context for gstreamer
  app->context = g_main_context_new();
  if (app->context == nullptr) {
    LOGE("failed to create gstreamer thread context\n");
    return;
  }
  g_main_context_push_thread_default(app->context);

  GstBus* bus = gst_element_get_bus(app->pipeline);
  gst_bus_add_signal_watch(bus);
  g_signal_connect(
      G_OBJECT(bus), "message",
      G_CALLBACK(+[](GstBus* bus, GstMessage* message, App* ctx) -> void {
        on_bus_message(ctx, bus, message);
      }),
      app);

  // create the main loop thread
  LOGI("===============> App entering main loop...\n");
  app->loop = g_main_loop_new(nullptr, FALSE);

  // start to receive data
  update_pipeline_state(app, GST_STATE_PLAYING);

  // run the main loop
  g_main_loop_run(app->loop);

  LOGI("<=============== App exited main loop\n");
  g_main_context_pop_thread_default(app->context);
  // clean up
  gst_bus_remove_signal_watch(bus);
  gst_object_unref(bus);
}

// app
static App* main_app = nullptr;

// signal handler
static volatile gint stop = 0;
void handle_signal(int signum) {
  LOGW("signal %d received.\n", signum);
  LOG(LOG_INFO, "Stopping...\n");

  // cleanup and close up stuff here
  // terminate program
  if (g_atomic_int_compare_and_exchange(&stop, 0, 1)) {
    deinit(main_app);
  } else {
    g_atomic_int_inc(&stop);
    if (g_atomic_int_get(&stop) > 2)
      exit(signum);
  }
}

int main(int argc, char* argv[]) {
  // handle SIGINT (CTRL-C), SIGTERM
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  LOGI("GStreamer version: %s\n", gst_version_string());
  LOGI("GStreamer init...\n");

  // init gstreamer
  gst_init(nullptr, nullptr);
  LOGI("GStreamer initialized\n");

  // create app
  App app;
  main_app = &app;

  LOGI("App init...\n");
  // init app
  int ret = init(&app);
  if (ret != 0) {
    // can not init the app, exit it.
    return ret;
  }
  LOGI("App initialized\n");

  // init virtualcam
  app.virtualcam = virtualcam_create();
  virtualcam_start(app.virtualcam, 1920, 1080, 30);

  // create mainloop and run it
  run(&app);

  // release app
  deinit(&app);
  LOGI("App shutdown\n");

  // deinit gstreamer
  gst_deinit();
  LOGI("GStreamer destoried\n");

  return 0;
}