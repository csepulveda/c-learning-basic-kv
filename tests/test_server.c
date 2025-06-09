#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../src/server_utils.h"
#include "../src/errors.h"

#define BUF_SIZE 1024

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
/**
 * @brief Utility function to check if response body contains expected string.
 *
 * Skips protocol header line, and searches for expected_resp in body.
 */
int response_contains(const char *buf, const char *expected_resp) {
    const char *body = strstr(buf, "\n");
    if (!body) return 0;
    body++;  // Skip header line

    // Check if expected_resp is anywhere in the body (before END)
    const char *end_marker = strstr(body, "END");
    size_t body_len = end_marker ? (size_t)(end_marker - body) : strlen(body);

    char body_copy[BUF_SIZE];
    snprintf(body_copy, sizeof(body_copy), "%.*s", (int)body_len, body);

    return strstr(body_copy, expected_resp) != NULL;
}

/**
 * @brief Tests the dispatch_command function by sending a command and verifying the response.
 */
void test_dispatch_command(const char *cmd, const char *expected_resp) {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    dispatch_command(fds[1], cmd);
    shutdown(fds[1], SHUT_WR);

    char buf[1024] = {0};
    recv_until_end(fds[0], buf, sizeof(buf));

    printf("Response:\n%s\n", buf);

    assert(strstr(buf, expected_resp) != NULL);

    close(fds[0]);
    close(fds[1]); 
}

/**
 * @brief Tests the handle_client function by sending a command and verifying the response.
 */
void test_handle_client(const char *cmd, const char *expected_resp) {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    pthread_t thread;
    pthread_create(&thread, NULL, handle_client, (void*)(intptr_t)fds[1]);

    write(fds[0], cmd, strlen(cmd));
    shutdown(fds[0], SHUT_WR);

    char buf[1024] = {0};
    recv_until_end(fds[0], buf, sizeof(buf));

    printf("Response:\n%s\n", buf);

    assert(strstr(buf, expected_resp) != NULL);

    pthread_join(thread, NULL);

    close(fds[0]);
    close(fds[1]); 
}

/**
 * @brief Runs all server command and client handler tests.
 */
int main() {
    test_dispatch_command("PING\n", "PONG");
    test_dispatch_command("SET test test1\n", "OK");
    test_dispatch_command("GET test\n", "test1");
    test_dispatch_command("DEL test\n", "DELETED");
    test_dispatch_command("NOEXIST\n", "ERROR unknown command");

    test_handle_client("PING\n", "PONG");
    test_handle_client("SET test test1\n", "OK");
    test_handle_client("GET test\n", "test1");
    test_handle_client("DEL test\n", "DELETED");
    test_handle_client("NOEXIST\n", "ERROR unknown command");

    printf("✅ All server tests passed!\n");
    return 0;
}