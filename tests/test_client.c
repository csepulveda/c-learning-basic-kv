#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "../src/client_utils.h"

#define BUFFER_SIZE 1024

void test_single_argument() {
    char *argv[] = { "client", "PING" };
    char buffer[1024] = {0};
    int result = build_command_string(2, argv, buffer, sizeof(buffer));
    assert(result == 0);
    assert(strcmp(buffer, "PING") == 0);
}

void test_multiple_arguments() {
    char *argv[] = { "client", "SET", "key=value" };
    char buffer[1024] = {0};
    int result = build_command_string(3, argv, buffer, sizeof(buffer));
    assert(result == 0);
    assert(strcmp(buffer, "SET key=value") == 0);
}

void test_long_input_truncates() {
    char *argv[100];
    argv[0] = "client";
    for (int i = 1; i < 99; i++) {
        argv[i] = "A";
    }
    argv[99] = NULL;

    char buffer[64] = {0};
    int result = build_command_string(99, argv, buffer, sizeof(buffer));
    assert(result == -1);
}

void test_empty_buffer() {
    char *argv[] = { "client", "PING" };
    int result = build_command_string(2, argv, NULL, 0);
    assert(result == -1);
}

void test_send_command_too_long() {
    int dummy_fd = -1;
    char long_cmd[BUFFER_SIZE + 100];
    memset(long_cmd, 'A', sizeof(long_cmd) - 1);
    long_cmd[sizeof(long_cmd) - 1] = '\0';

    int result = send_command(dummy_fd, long_cmd);
    assert(result == -1);
}

void capture_stdout_start(FILE **original_stdout, FILE **tmp_file) {
    *original_stdout = stdout;
    *tmp_file = tmpfile();
    stdout = *tmp_file;
}

void capture_stdout_stop(FILE *original_stdout, FILE *tmp_file, char *output, size_t output_size) {
    fflush(stdout);
    rewind(tmp_file);
    fread(output, 1, output_size - 1, tmp_file);
    output[output_size - 1] = '\0';
    fclose(tmp_file);
    stdout = original_stdout;
}

void test_handle_char() {
    char line_buffer[BUFFER_SIZE];
    size_t line_pos = 0;
    bool first_line = true;

    FILE *orig_stdout, *tmp;
    char captured[BUFFER_SIZE];

    const char *input = "RESPONSE OK\nHello World\nEND\n";
    bool found_end = false;

    capture_stdout_start(&orig_stdout, &tmp);

    for (size_t i = 0; input[i] != '\0'; i++) {
        bool end = handle_char(input[i], line_buffer, &line_pos, &first_line);
        if (end) {
            found_end = true;
            break;
        }
    }

    capture_stdout_stop(orig_stdout, tmp, captured, sizeof(captured));

    assert(strstr(captured, "Hello World") != NULL);
    assert(strstr(captured, "RESPONSE") == NULL); 
    assert(found_end == true);
    assert(first_line == false);
    assert(line_pos == 0);

    printf("✅ test_handle_char passed\n");
}

int main() {
    test_single_argument();
    test_multiple_arguments();
    test_long_input_truncates();
    test_empty_buffer();
    test_send_command_too_long();
    test_handle_char();
    printf("✅ All build_command_string tests passed\n");
    return 0;
}