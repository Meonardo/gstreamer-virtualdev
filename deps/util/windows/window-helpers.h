#pragma once

#include <util/c99defs.h>
#include <util/dstr.h>
#include <Windows.h>

#ifdef __cplusplus
extern "C" {
#endif

EXPORT void ms_build_window_strings(const char *str, char **window_class,
				    char **title, char **exe);

EXPORT void ms_build_window_strings(const char *str, char **window_class,
				    char **title, char **exe);

#ifdef __cplusplus
}
#endif
