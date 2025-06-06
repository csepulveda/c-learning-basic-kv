#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../src/server_utils.h"

void test_dispatch_command(const char *cmd, const char *resp) {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    dispatch_command(fds[1], cmd);

    char buf[1024];
    ssize_t n = recv(fds[0], buf, sizeof(buf) - 1, 0);
    buf[n] = '\0';

    printf("Response: %s\n", buf);
    assert(strcmp(buf, resp) == 0);

    close(fds[0]);
    close(fds[1]);
}

int main() {
    test_dispatch_command("PING\n", "PONG\n");
    test_dispatch_command("SET test=test1\n", "OK\n");
    test_dispatch_command("GET test\n", "test1\n");
    test_dispatch_command("DEL test\n", "DELETED\n");
    test_dispatch_command("GOODBYE\n", "Goodbye!\n");
    test_dispatch_command("NOEXIST\n", "Invalid command\n");
    
    printf("✅ All build_command_string tests passed\n");
    return 0;
}
