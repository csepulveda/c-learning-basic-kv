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

/**
 * @brief Returns the ANSI color escape code for a given log level.
 *
 * Maps the specified log level to its corresponding terminal color code for colored log output.
 *
 * @param level The log level to map.
 * @return const char* The ANSI escape code string for the log level's color, or the reset code for unknown levels.
 */
static const char *level_to_color(LogLevel level) {
	switch (level) {
		case LOG_LEVEL_DEBUG:	return "\033[36m"; //cyan
		case LOG_LEVEL_INFO:	return "\033[32m"; //green
		case LOG_LEVEL_ERROR:	return "\033[31m"; //red
		default:		return "\033[0m";
	}
}

/**
 * @brief Logs a formatted message to stderr with timestamp, log level, file, and line information.
 *
 * Outputs the message only if the specified log level is at or above the current global log level.
 * The log entry includes a color-coded log level, timestamp, source file, and line number.
 *
 * @param level The severity level of the log message.
 * @param file The name of the source file where the log is generated.
 * @param line The line number in the source file.
 * @param fmt The format string for the log message.
 * @param args The variable argument list for the format string.
 */
void log_message_va(LogLevel level, const char *file, int line, const char *fmt, va_list args) {
	if (level < CURRENT_LOG_LEVEL) return;
	
	time_t t = time(NULL);
	struct tm lt_data;
	localtime_r(&t, &lt_data);

	char timebuf[20];
	strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", &lt_data);

	fprintf(stderr, "%s %s[%s] %s:%d: ", timebuf, level_to_color(level), level_to_string(level), file, line);
	vfprintf(stderr, fmt, args);
	fprintf(stderr, "\033[0m\n");
}