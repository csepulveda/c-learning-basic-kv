#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#include "../src/commands.h"
#include "../src/kvstore.h"
#include "../src/protocol.h"
#include "../src/errors.h"

#define BUF_SIZE 1024
time_t start_time = 0;

void recv_until_end(int fd, char *buf, size_t buf_size) {
    size_t total_read = 0;
    while (total_read < buf_size - 1) {
        ssize_t n = recv(fd, buf + total_read, buf_size - 1 - total_read, 0);
        if (n <= 0) break;
        total_read += n;
        buf[total_read] = '\0';

        if (strstr(buf, "END\n")) break;
    }
}

int response_contains(const char *buf, const char *expected_resp) {
    const char *body = strstr(buf, "\n");
    if (!body) return 0;
    body++;  // Skip header line

    const char *end_marker = strstr(body, "END");
    size_t body_len = end_marker ? (size_t)(end_marker - body) : strlen(body);

    char body_copy[BUF_SIZE];
    snprintf(body_copy, sizeof(body_copy), "%.*s", (int)body_len, body);

    return strstr(body_copy, expected_resp) != NULL;
}

void test_cmd_set(const char *cmd_buffer, const char *expected_resp) {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    // Reset the kv store
    kv_init();

    // Call cmd_set
    cmd_set(fds[1], cmd_buffer);

    // Read response
    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));

    printf("cmd_set('%s') -> '%s'\n", cmd_buffer, buf);

    // Assert
    assert(response_contains(buf, expected_resp));

    close(fds[0]);
    close(fds[1]);
}

void test_send_error_response(int error_code, const char *expected_msg) {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    // Call send_error_response
    send_error_response(fds[1], error_code);

    // Read response
    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));

    printf("send_error_response(%d) -> '%s'\n", error_code, buf);

    // Assert header
    assert(strstr(buf, "RESPONSE ERROR") != NULL);

    // Assert error message
    assert(strstr(buf, expected_msg) != NULL);

    // Assert footer
    assert(strstr(buf, "END") != NULL);

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_info() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    cmd_info(fds[1], "");

    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));

    printf("cmd_info() -> '%s'\n", buf);

    assert(response_contains(buf, "Uptime:"));
    assert(response_contains(buf, "Memory:"));
    assert(response_contains(buf, "Keys:"));
    assert(response_contains(buf, "Version:"));

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_type() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    // Setup: SET a key
    kv_init();
    kv_set("foo", "bar");

    // Test TYPE existing key
    cmd_type(fds[1], "TYPE foo");
    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_type() existing key -> '%s'\n", buf);
    assert(response_contains(buf, "string"));

    // Test TYPE missing key
    cmd_type(fds[1], "TYPE missing_key");
    memset(buf, 0, sizeof(buf));
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_type() missing key -> '%s'\n", buf);

    // Make the test more robust
    assert(strstr(buf, "(nil)") != NULL);

    close(fds[0]);
    close(fds[1]);
}

int main() {
    // Test OK
    test_cmd_set("SET foo bar\n", "OK");

    // Test key too long
    char long_key[MAX_KEY_LEN + 100];
    memset(long_key, 'A', sizeof(long_key) - 1);
    long_key[sizeof(long_key) - 1] = '\0';
    char buffer1[BUF_SIZE];
    snprintf(buffer1, sizeof(buffer1), "SET %s value\n", long_key);
    test_cmd_set(buffer1, "ERROR key too long");

    // Test value too long
    char long_value[MAX_VAL_LEN + 100];
    memset(long_value, 'B', MAX_VAL_LEN + 50);  // Ensure it exceeds MAX_VAL_LEN
    long_value[MAX_VAL_LEN + 50] = '\0';
    char buffer2[BUF_SIZE];
    snprintf(buffer2, sizeof(buffer2), "SET foo %s\n", long_value);
    test_cmd_set(buffer2, "ERROR value too long");

    // Test parse error
    test_cmd_set("SET foo\n", "ERROR parse error");

    // Test send_error_response
    test_send_error_response(EXTRACT_ERR_PARSE, ERR_PARSE_ERROR);
    test_send_error_response(EXTRACT_ERR_KEY_TOO_LONG, ERR_KEY_TOO_LONG);
    test_send_error_response(EXTRACT_ERR_VALUE_TOO_LONG, ERR_VALUE_TOO_LONG);
    test_send_error_response(EXTRACT_ERR_KEY_NOT_FOUND, ERR_NOT_FOUND);

    // Test INFO
    test_cmd_info();

    // Test TYPE
    test_cmd_type();

    printf("✅ All cmd_set tests passed!\n");
    return 0;
}