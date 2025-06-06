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

void log_message_va(LogLevel level, const char *file, int line, const char *fmt, va_list args);

static inline void log_message_str(LogLevel level, const char *file, int line, const char *fmt, ...) {
	if (level < CURRENT_LOG_LEVEL) return;

	va_list args;
	va_start(args, fmt);
	log_message_va(level, file, line, fmt, args);
	va_end(args);
}

#define log_debug(fmt, ...) log_message_str(LOG_LEVEL_DEBUG, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_info(fmt, ...)  log_message_str(LOG_LEVEL_INFO,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define log_error(fmt, ...) log_message_str(LOG_LEVEL_ERROR, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif