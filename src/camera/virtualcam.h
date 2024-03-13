#pragma once

#include <stdio.h>

#ifdef _MSC_VER
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void virtualcam_destroy(void* data);
EXPORT void* virtualcam_create();
EXPORT bool virtualcam_start(void* data);
EXPORT void virtualcam_stop(void* data, uint64_t ts);
EXPORT void virtual_video(void* data,
                          uint8_t** v_data,
                          uint32_t* linesize,
                          uint64_t timestamp);

#ifdef __cplusplus
}
#endif