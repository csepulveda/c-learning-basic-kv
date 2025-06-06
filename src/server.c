// server.c
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "commands.h"
#include "kv_store.h"
#include "logs.h"
#include "protocol.h"

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
    int status;
    int SERVER_PORT = getenv("PORT") ? atoi(getenv("PORT")) : 8080;


    int serverfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(SERVER_PORT),
        .sin_addr.s_addr = INADDR_ANY
    };

    status = bind(serverfd, (struct sockaddr*)&addr, sizeof(addr));
    if (status != 0 ) {
        log_error("Error binding the port: %s", strerror(errno));
        return 1;
    }

    status = listen(serverfd, 5);
    if (status != 0) {
        log_error("Error listening: %s", strerror(errno));
        return 1;
    }

    log_info("Server listening on port %d...\n", SERVER_PORT);

    while (1) {
        int clientfd = accept(serverfd, NULL, NULL);
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, (void *)(intptr_t)clientfd);
        pthread_detach(tid);
    }

    close(serverfd);
    return 0;
}
