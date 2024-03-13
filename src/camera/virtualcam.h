#pragma once

#include <stdio.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

#define MAX_AV_PLANES 8

#ifdef __cplusplus
extern "C" {
#endif

typedef struct video_data {
  uint8_t* data[MAX_AV_PLANES];
  uint32_t linesize[MAX_AV_PLANES];
  uint64_t timestamp;
} VideoFrame;

EXPORT void virtualcam_destroy(void* data);
EXPORT void* virtualcam_create();
EXPORT bool virtualcam_start(void* data, uint32_t w, uint32_t h, uint16_t fps);
EXPORT void virtualcam_stop(void* data, uint64_t ts);
EXPORT void virtual_video(void* data, VideoFrame* frame);

#ifdef __cplusplus
}
#endif