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

void test_cmd_ping() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    cmd_ping(fds[1], "");

    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));

    printf("cmd_ping() -> '%s'\n", buf);
    assert(response_contains(buf, "PONG"));

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_time() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    cmd_time(fds[1], "");

    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));

    printf("cmd_time() -> '%s'\n", buf);
    assert(response_contains(buf, ":")); // e.g. time string

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_get() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    kv_init();
    kv_set("foo", "bar");

    cmd_get(fds[1], "GET foo");
    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_get() -> '%s'\n", buf);
    assert(response_contains(buf, "bar"));

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_del() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    kv_init();
    kv_set("foo", "bar");

    cmd_del(fds[1], "DEL foo");
    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_del() -> '%s'\n", buf);
    assert(response_contains(buf, "DELETED"));

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_mset_mget() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    kv_init();
    cmd_mset(fds[1], "MSET k1 v1 k2 v2 k3 v3");
    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_mset() -> '%s'\n", buf);
    assert(response_contains(buf, "OK"));

    // Now test MGET
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    cmd_mget(fds[1], "MGET k1 k2 k3");

    char buf2[BUF_SIZE];
    recv_until_end(fds[0], buf2, sizeof(buf2));
    printf("cmd_mget() -> '%s'\n", buf2);

    assert(response_contains(buf2, "1) v1"));
    assert(response_contains(buf2, "2) v2"));
    assert(response_contains(buf2, "3) v3"));

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_hset_hget() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    kv_init();
    cmd_hset(fds[1], "HSET myhash field1 value1");
    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_hset() -> '%s'\n", buf);
    assert(response_contains(buf, "1"));

    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    cmd_hget(fds[1], "HGET myhash field1");
    char buf2[BUF_SIZE];
    recv_until_end(fds[0], buf2, sizeof(buf2));
    printf("cmd_hget() -> '%s'\n", buf2);
    assert(response_contains(buf2, "value1"));

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_hincrby() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    kv_init();
    cmd_hincrby(fds[1], "HINCRBY myhash counter 5");

    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_hincrby() -> '%s'\n", buf);
    assert(response_contains(buf, "5"));

    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    cmd_hincrby(fds[1], "HINCRBY myhash counter 3");
    char buf2[BUF_SIZE];
    recv_until_end(fds[0], buf2, sizeof(buf2));
    printf("cmd_hincrby() second -> '%s'\n", buf2);
    assert(response_contains(buf2, "8"));

    close(fds[0]);
    close(fds[1]);
}

void test_cmd_hmget() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    kv_init();
    kv_hset("myhash", "field1", "val1");
    kv_hset("myhash", "field2", "val2");

    cmd_hmget(fds[1], "HMGET myhash field1 field2 missing");

    char buf[BUF_SIZE];
    recv_until_end(fds[0], buf, sizeof(buf));
    printf("cmd_hmget() -> '%s'\n", buf);

    assert(response_contains(buf, "1) val1"));
    assert(response_contains(buf, "2) val2"));
    assert(response_contains(buf, "3) (nil)"));

    close(fds[0]);
    close(fds[1]);
}

void test_extract_key_from_ptr() {
    char key[128];
    const char *input;

    // Test 1: Quoted token normal
    input = "\"mykey\" rest_of_input";
    const char *p = input;
    int ret = extract_key_from_ptr(&p, key, sizeof(key));
    printf("extract_key_from_ptr() -> '%s' ret=%d\n", key, ret);
    assert(ret == EXTRACT_OK);
    assert(strcmp(key, "mykey") == 0);
    assert(strcmp(p, "rest_of_input") == 0);

    // Test 2: Quoted token, unterminated (parse error)
    input = "\"unterminated rest_of_input";
    p = input;
    ret = extract_key_from_ptr(&p, key, sizeof(key));
    printf("extract_key_from_ptr() unterminated -> ret=%d\n", ret);
    assert(ret == EXTRACT_ERR_PARSE);

    // Test 3: Quoted token, key too long
    char long_input[300];
    memset(long_input, 'A', 200);  // fill with 'A's
    long_input[0] = '"';
    long_input[199] = '"';
    long_input[200] = ' ';
    long_input[201] = '\0';
    p = long_input;
    ret = extract_key_from_ptr(&p, key, 10);  // force small key_size
    printf("extract_key_from_ptr() too long -> ret=%d\n", ret);
    assert(ret == EXTRACT_ERR_KEY_TOO_LONG);

    // Test 4: Unquoted token (still test unquoted for completeness)
    input = "plain_key next_token";
    p = input;
    ret = extract_key_from_ptr(&p, key, sizeof(key));
    printf("extract_key_from_ptr() unquoted -> '%s' ret=%d\n", key, ret);
    assert(ret == EXTRACT_OK);
    assert(strcmp(key, "plain_key") == 0);
    assert(strcmp(p, "next_token") == 0);

    printf("✅ All extract_key_from_ptr tests passed!\n");
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

    test_cmd_ping();
    test_cmd_time();
    test_cmd_get();
    test_cmd_del();
    test_cmd_mset_mget();
    test_cmd_hset_hget();
    test_cmd_hmget();
    test_cmd_hincrby();

    test_extract_key_from_ptr();

    printf("✅ All cmd_set tests passed!\n");
    return 0;
}
