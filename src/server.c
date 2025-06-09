#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "kvstore.h"
#include "logs.h"
#include "server_utils.h"

volatile sig_atomic_t running = 1;
int serverfd;

/**
 * @brief Handles SIGTERM to gracefully shut down the server.
 *
 * Sets the running flag to 0, logs the shutdown event, and closes the server socket.
 *
 * @param sig The signal number received (expected to be SIGTERM).
 */
void handle_sigterm(int sig) {
    running = 0;
    log_info("Shuting Down Server by signal: %d", sig);
    close(serverfd);
}


/**
 * @brief Entry point for the multi-threaded TCP server.
 *
 * Initializes the key-value store, sets up the server socket, and listens for incoming client connections on a configurable port. Handles each client connection in a separate detached thread. Supports graceful shutdown on SIGTERM.
 *
 * @return int Returns 0 on normal termination, or 1 if socket binding or listening fails.
 */
int main() {
    kv_init();
    int status;
    int SERVER_PORT = getenv("PORT") ? atoi(getenv("PORT")) : 8080;

    signal(SIGTERM, handle_sigterm);

    serverfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons((uint16_t)SERVER_PORT),
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

    while (running) {
        int clientfd = accept(serverfd, NULL, NULL);
        if (clientfd < 0) {
            if (running) {
                log_error("Error accepting connection: %s", strerror(errno));
            }
            continue;
        }
        pthread_t tid;
        pthread_create(&tid, NULL, handle_client, (void *)(intptr_t)clientfd);
        pthread_detach(tid);
    }

    close(serverfd);
    return 0;
}
