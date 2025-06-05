#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>
#include <stdarg.h>

typedef enum {
	LOG_LEVEL_DEBUG = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_ERROR
} LogLevel;

extern LogLevel CURRENT_LOG_LEVEL;

void log_message(LogLevel level, const char *file, int line, const char *fmt, ...);

#define log_debug(fmt, ...) log_message(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define log_info(fmt, ...) log_message(LOG_LEVEL_INFO, __FILE__, __LINE__, fmt, __VA_ARGS__);
#define log_error(fmt, ...) log_message(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, __VA_ARGS__);

#define LEVEL
#endif
