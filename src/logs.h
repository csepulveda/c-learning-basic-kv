#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

typedef enum {
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_ERROR
} LogLevel;

extern LogLevel CURRENT_LOG_LEVEL;

// Declaraciones públicas
void log_message_va(LogLevel level, const char *file, int line, const char *fmt, va_list args);
void log_message_str(LogLevel level, const char *file, int line, const char *message);
const char *level_to_color(LogLevel level);
const char *level_to_string(LogLevel level);

// Macros de logging
#define log_debug(fmt, ...) do { \
    char _msg[1024]; \
    snprintf(_msg, sizeof(_msg), fmt, ##__VA_ARGS__); \
    log_message_str(LOG_LEVEL_DEBUG, __FILE__, __LINE__, _msg); \
} while(0)

#define log_info(fmt, ...) do { \
    char _msg[1024]; \
    snprintf(_msg, sizeof(_msg), fmt, ##__VA_ARGS__); \
    log_message_str(LOG_LEVEL_INFO, __FILE__, __LINE__, _msg); \
} while(0)

#define log_error(fmt, ...) do { \
    char _msg[1024]; \
    snprintf(_msg, sizeof(_msg), fmt, ##__VA_ARGS__); \
    log_message_str(LOG_LEVEL_ERROR, __FILE__, __LINE__, _msg); \
} while(0)

#endif