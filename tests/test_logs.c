#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../src/logs.h"

char *capture_stderr_output(void (*log_fn)(void)) {
    int pipefd[2];
    pipe(pipefd);

    int saved_stderr = dup(STDERR_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[1]);

    log_fn();

    fflush(stderr);
    dup2(saved_stderr, STDERR_FILENO);
    close(saved_stderr);

    char *buffer = calloc(1, 4096);
    read(pipefd[0], buffer, 4095);
    close(pipefd[0]);

    return buffer;
}

void log_fn_debug() {
    log_message_str(LOG_LEVEL_DEBUG, "test_logs.c", 10, "Debug test");
}

void log_fn_info() {
    log_message_str(LOG_LEVEL_INFO, "test_logs.c", 20, "Info test");
}

void log_fn_error() {
    log_message_str(LOG_LEVEL_ERROR, "test_logs.c", 30, "Error test");
}

// 🧪 Tests
void test_debug_log() {
    CURRENT_LOG_LEVEL = LOG_LEVEL_DEBUG;
    char *output = capture_stderr_output(log_fn_debug);
    assert(strstr(output, "DEBUG"));
    assert(strstr(output, "Debug test"));
    free(output);
}

void test_info_log() {
    CURRENT_LOG_LEVEL = LOG_LEVEL_DEBUG;
    char *output = capture_stderr_output(log_fn_info);
    assert(strstr(output, "INFO"));
    assert(strstr(output, "Info test"));
    free(output);
}

void test_error_log() {
    CURRENT_LOG_LEVEL = LOG_LEVEL_DEBUG;
    char *output = capture_stderr_output(log_fn_error);
    assert(strstr(output, "ERROR"));
    assert(strstr(output, "Error test"));
    free(output);
}

void test_va_wrapper(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    log_message_va(LOG_LEVEL_INFO, "va_test.c", 77, fmt, args);
    va_end(args);
}

void log_fn_va() {
    test_va_wrapper("VA log: %s = %d", "value", 42);
}

void test_va_log() {
    CURRENT_LOG_LEVEL = LOG_LEVEL_DEBUG;

    char *output = capture_stderr_output(log_fn_va);
    assert(strstr(output, "INFO"));
    assert(strstr(output, "va_test.c"));
    assert(strstr(output, "VA log: value = 42"));
    free(output);
}

int main() {
    test_va_log();
    test_debug_log();
    test_info_log();
    test_error_log();

    printf("✅ Simple log level tests passed\n");
    return 0;
}