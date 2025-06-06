#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
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


void test_handle_client(const char *cmd, const char *expected_resp) {
    int fds[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fds);

    // Lanzar handle_client en un hilo
    pthread_t thread;
    pthread_create(&thread, NULL, handle_client, (void*)(intptr_t)fds[1]);

    // Enviar el comando
    write(fds[0], cmd, strlen(cmd));

    // Cerrar el lado de escritura para que handle_client salga de loop
    shutdown(fds[0], SHUT_WR);

    // Leer la respuesta
    char buf[1024];
    ssize_t n = recv(fds[0], buf, sizeof(buf) - 1, 0);
    buf[n] = '\0';

    printf("Response: %s\n", buf);
    assert(strcmp(buf, expected_resp) == 0);

    // Esperar a que el hilo termine
    pthread_join(thread, NULL);

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

    test_handle_client("PING\n", "PONG\n");
    test_handle_client("SET test=test1\n", "OK\n");
    test_handle_client("GET test\n", "test1\n");
    test_handle_client("DEL test\n", "DELETED\n");
    test_handle_client("GOODBYE\n", "Goodbye!\n");
    test_handle_client("NOEXIST\n", "Invalid command\n");



    printf("✅ All build_command_string tests passed\n");
    return 0;
}
