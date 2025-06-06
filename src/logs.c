#include "logs.h"

#include <stdlib.h>
#include <time.h>

LogLevel CURRENT_LOG_LEVEL = LOG_LEVEL_DEBUG;

static const char *level_to_string(LogLevel level) {
	switch (level) {
		case LOG_LEVEL_DEBUG:	return "DEBUG";
		case LOG_LEVEL_INFO:	return "INFO";
		case LOG_LEVEL_ERROR:	return "ERROR";
		default:		return "UNKNOWN";
	}
}

static const char *level_to_color(LogLevel level) {
	switch (level) {
		case LOG_LEVEL_DEBUG:	return "\033[36m"; //cyan
		case LOG_LEVEL_INFO:	return "\033[32m"; //green
		case LOG_LEVEL_ERROR:	return "\033[31m"; //red
		default:		return "\033[0m";
	}
}

void log_message(LogLevel level, const char *file, int line, const char *fmt, ...) {
	if (level < CURRENT_LOG_LEVEL) return;

	time_t t = time(NULL);
	struct tm lt_data;
	struct tm *lt = localtime_r(&t, &lt_data);

	char timebuf[20];
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", lt);

	fprintf(stderr, "%s[%s] %s:%d: ", level_to_color(level), level_to_string(level), file, line);
    
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);

	fprintf(stderr, "\033[0m\n");
}
