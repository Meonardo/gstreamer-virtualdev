#pragma once

#include <glib.h>
#include <glib/gprintf.h>

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

extern int log_level;
extern gboolean log_timestamps;
extern gboolean log_colors;

// Log colors
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

// Log levels
#define LOG_NONE (0)
#define LOG_FATAL (1)
#define LOG_ERR (2)
#define LOG_WARN (3)
#define LOG_INFO (4)
#define LOG_VERB (5)
#define LOG_HUGE (6)
#define LOG_DBG (7)
#define LOG_MAX LOG_DBG

// Coloured prefixes for errors and warnings logging.
static const char* log_prefix[] = {
    /* no colors */
    "", "[FATAL] ", "[ERR] ", "[WARN] ", "", "", "", "",
    /* with colors */
    "", ANSI_COLOR_MAGENTA "[FATAL]" ANSI_COLOR_RESET " ",
    ANSI_COLOR_RED "[ERR]" ANSI_COLOR_RESET " ",
    ANSI_COLOR_YELLOW "[WARN]" ANSI_COLOR_RESET " ", "", "", "", ""};
static const char* name_prefix[] = {
    /* no colors */
    "[VIRDEV] ",
    /* with colors */
    ANSI_COLOR_CYAN "[VIRDEV]" ANSI_COLOR_RESET " "};

// Get current time with millsecond
static std::string print_current_time() {
  auto now = std::chrono::system_clock::now();
  auto now_c = std::chrono::system_clock::to_time_t(now);
  auto local_time = std::localtime(&now_c);
  auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;
  auto time_str = std::put_time(local_time, "%Y-%m-%d %H:%M:%S");

  std::stringstream ss;

  ss << "[" << std::put_time(local_time, "%Y-%m-%d %H:%M:%S");
  ss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count()
     << "] ";

  return ss.str();
}

// Simple wrapper to g_print/printf
#define PRINT g_print
/* Logger based on different levels, which can either be displayed
 * or not according to the configuration of the gateway.
 * The format must be a string literal. */
#define LOG(level, format, ...)                                         \
  do {                                                                  \
    if (level > LOG_NONE && level <= LOG_MAX && level <= log_level) {   \
      char log_ts[64] = "";                                             \
      char log_src[128] = "";                                           \
      if (log_timestamps) {                                             \
        strcpy(log_ts, print_current_time().c_str());                   \
      }                                                                 \
      if (level == LOG_FATAL || level == LOG_ERR || level == LOG_DBG) { \
        snprintf(log_src, sizeof(log_src), "[%s:%s:%d] ", __FILE__,     \
                 __FUNCTION__, __LINE__);                               \
      }                                                                 \
      g_print("%s%s%s" format, log_ts,                                  \
              log_prefix[level | ((int)log_colors << 3)], log_src,      \
              ##__VA_ARGS__);                                           \
    }                                                                   \
  } while (0)

// Same as above, but with a [VIRDEV] prefix
#define LOG_PREFIX(level, format, ...)                                  \
  do {                                                                  \
    if (level > LOG_NONE && level <= LOG_MAX && level <= log_level) {   \
      char log_ts[64] = "";                                             \
      char log_src[128] = "";                                           \
      if (log_timestamps) {                                             \
        strcpy(log_ts, print_current_time().c_str());                   \
      }                                                                 \
      if (level == LOG_FATAL || level == LOG_ERR || level == LOG_DBG) { \
        snprintf(log_src, sizeof(log_src), "[%s:%s:%d] ", __FILE__,     \
                 __FUNCTION__, __LINE__);                               \
      }                                                                 \
      g_print("%s%s%s%s" format, name_prefix[log_colors], log_ts,       \
              log_prefix[level | ((int)log_colors << 3)], log_src,      \
              ##__VA_ARGS__);                                           \
    }                                                                   \
  } while (0)

// Log macros
#define LOGI(format, ...) LOG(LOG_INFO, format, ##__VA_ARGS__)
#define LOGV(format, ...) LOG(LOG_VERB, format, ##__VA_ARGS__)
#define LOGD(format, ...) LOG(LOG_DBG, format, ##__VA_ARGS__)
#define LOGW(format, ...) LOG(LOG_WARN, format, ##__VA_ARGS__)
#define LOGE(format, ...) LOG(LOG_ERR, format, ##__VA_ARGS__)
#define LOGF(format, ...) LOG(LOG_FATAL, format, ##__VA_ARGS__)
