#include <stdlib.h>

#include "logs.h"

LogLevel CURRENT_LOG_LEVEL = LOG_LEVEL_DEBUG;

const char *level_to_string(LogLevel level) {
	switch (level) {
		case LOG_LEVEL_DEBUG: return "DEBUG";
		case LOG_LEVEL_INFO:  return "INFO";
		case LOG_LEVEL_ERROR: return "ERROR";
		default:              return "UNKNOWN";
	}
}

const char *level_to_color(LogLevel level) {
	switch (level) {
		case LOG_LEVEL_DEBUG: return "\033[36m"; // Cyan
		case LOG_LEVEL_INFO:  return "\033[32m"; // Green
		case LOG_LEVEL_ERROR: return "\033[31m"; // Red
		default:              return "\033[0m";  // Reset
	}
}

void log_message_str(LogLevel level, const char *file, int line, const char *message) {
	if (level < CURRENT_LOG_LEVEL) return;

	time_t t = time(NULL);
	struct tm lt_data;
	localtime_r(&t, &lt_data);

	char timebuf[20];
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &lt_data);

	fprintf(stderr, "%s %s[%s] %s:%d: %s\033[0m\n",
	        timebuf,
	        level_to_color(level),
	        level_to_string(level),
	        file,
	        line,
	        message);
}

void log_message_va(LogLevel level, const char *file, int line, const char *fmt, va_list args) {
	if (level < CURRENT_LOG_LEVEL) return;

	time_t t = time(NULL);
	struct tm lt_data;
	localtime_r(&t, &lt_data);

	char timebuf[20];
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &lt_data);

	fprintf(stderr, "%s %s[%s] %s:%d: ", timebuf, level_to_color(level), level_to_string(level), file, line);
	vfprintf(stderr, fmt, args); //NOSONAR
	fprintf(stderr, "\033[0m\n");
}
