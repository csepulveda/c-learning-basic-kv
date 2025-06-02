// server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#include "kv_store.h"
#include "protocol.h"
#include "commands.h"  // nuevo

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void* handle_client(void *arg) {
    int clientfd = (int)(intptr_t)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;

        command_t cmd = parse_command(buffer);

        switch (cmd) {
            case CMD_PING:
                cmd_ping(clientfd);
                break;
            case CMD_TIME:
                cmd_time(clientfd);
                break;
            case CMD_SET:
                cmd_set(clientfd, buffer);
                break;
            case CMD_GET:
                cmd_get(clientfd, buffer);
                break;
            case CMD_DEL:
                cmd_del(clientfd, buffer);
                break;
            case CMD_GOODBYE:
                cmd_goodbye(clientfd);
                return NULL;
            case CMD_UNKNOWN:
            default:
                send(clientfd, "Invalid command\n", 17, 0);
                break;
        }
    }

    close(clientfd);
    return NULL;
}

int main() {
    kv_init();

    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    bind(serverfd, (struct sockaddr*)&addr, sizeof(addr));
    listen(serverfd, 5);

    printf("Server listening on port %d...\n", SERVER_PORT);

    while (1) {
        int clientfd = accept(serverfd, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, (void *)(intptr_t)clientfd);
        pthread_detach(tid);
    }

    close(serverfd);
    return 0;
}