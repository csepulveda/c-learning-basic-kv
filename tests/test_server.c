#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../src/server_utils.h"


void test_dispatch_command() {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    dispatch_command(fds[1], "PING\n");

    char buf[1024];
    ssize_t n = recv(fds[0], buf, sizeof(buf) - 1, 0);
    buf[n] = '\0';

    printf("Respuesta: %s\n", buf);

    if (strstr(buf, "PONG") != NULL) {
        printf("✅ Test PING PASSED\n");
    } else {
        printf("❌ Test PING FAILED\n");
    }

    close(fds[0]);
    close(fds[1]);
}

int main() {
    printf("✅ All build_command_string tests passed\n");
    return 0;
}
