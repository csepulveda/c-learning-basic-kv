#include <assert.h>
#include <string.h>
#include <stdio.h>
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

int main() {
    test_single_argument();
    test_multiple_arguments();
    test_long_input_truncates();
    test_empty_buffer();
    test_send_command_too_long();
    printf("✅ All build_command_string tests passed\n");
    return 0;
}
