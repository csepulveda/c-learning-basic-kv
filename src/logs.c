#include <stdlib.h>

#include "logs.h"

LogLevel CURRENT_LOG_LEVEL = LOG_LEVEL_DEBUG;

/**
 * @brief Returns the string representation of a log level.
 *
 * Converts a LogLevel enum value to its corresponding string ("DEBUG", "INFO", "ERROR").
 * Returns "UNKNOWN" if the level is unrecognized.
 *
 * @param level The log level to convert.
 * @return const char* The string representation of the log level.
 */
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

/**
 * @brief Logs a formatted message with level, timestamp, color, and source location.
 *
 * Outputs a log message to stderr if the specified log level meets or exceeds the current threshold.
 * The message includes a timestamp, colored log level, source file name, line number, and the formatted message.
 *
 * @param level The severity level of the log message.
 * @param file The source file name where the log is generated.
 * @param line The line number in the source file.
 * @param fmt The format string for the log message.
 * @param args The variable argument list for formatting the message.
 */
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
