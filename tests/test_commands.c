#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../src/commands.h"
#include "../src/kv_store.h"
#include "../src/protocol.h"
#include "../src/errors.h"

#define BUF_SIZE 1024

void test_cmd_set(const char *cmd_buffer, const char *expected_resp) {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    // Reset the kv store (optional, to make tests independent)
    kv_init();

    // Call cmd_set
    cmd_set(fds[1], cmd_buffer);

    // Read response
    char buf[BUF_SIZE];
    ssize_t n = recv(fds[0], buf, sizeof(buf) - 1, 0);
    buf[n] = '\0';

    printf("cmd_set('%s') -> '%s'\n", cmd_buffer, buf);

    // Assert
    assert(strcmp(buf, expected_resp) == 0);

    close(fds[0]);
    close(fds[1]);
}

int main() {
    // Test OK
    test_cmd_set("SET foo bar\n", "OK\n");

    // Test key too long
    char long_key[MAX_KEY_LEN + 100];
    memset(long_key, 'A', sizeof(long_key) - 1);
    long_key[sizeof(long_key) - 1] = '\0';
    char buffer1[BUF_SIZE];
    snprintf(buffer1, sizeof(buffer1), "SET %s value\n", long_key);
    test_cmd_set(buffer1, ERR_KEY_TOO_LONG);

    // Test value too long
    char long_value[MAX_VAL_LEN + 100];
    memset(long_value, 'B', sizeof(long_value) - 1);
    long_value[sizeof(long_value) - 1] = '\0';
    char buffer2[BUF_SIZE];
    snprintf(buffer2, sizeof(buffer2), "SET foo %s\n", long_value);
    test_cmd_set(buffer2, ERR_VALUE_TOO_LONG);

    // Test parse error
    test_cmd_set("SET foo\n", ERR_PARSE_ERROR);

    printf("✅ All cmd_set tests passed!\n");
    return 0;
}