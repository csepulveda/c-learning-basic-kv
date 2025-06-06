#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "protocol.h"

#include "logs.h"
#include "client_utils.h"

int main(int argc, char *argv[]) {
    int sockfd, status;
    ssize_t status_r;
    struct sockaddr_in addr;
    char buffer[BUFFER_SIZE];

    const char *server_ip = getenv("HOST") ? getenv("HOST") : "127.0.0.1";
    int server_port = getenv("PORT") ? atoi(getenv("PORT")) : 8080;

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons((uint16_t)server_port);
    inet_aton(server_ip, &addr.sin_addr);

    status = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if (status) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    log_info("Connected to %s:%d\n", server_ip, server_port);

    // Modo argumento por línea de comandos
    if (argc > 1) {
        char command[BUFFER_SIZE] = {0};

        if (build_command_string(argc, argv, command, sizeof(command)) != 0) {
            log_error("Failed to construct command\n");
            close(sockfd);
            return 1;
        }

        if (parse_command(command) == CMD_UNKNOWN) {
            log_error("Invalid command: %s\n", command);
            close(sockfd);
            return 1;
        }

        send_command(sockfd, command);
        memset(buffer, 0, sizeof(buffer));
        recv(sockfd, buffer, sizeof(buffer), 0);
        printf("Response: %s", buffer);
        close(sockfd);
        return 0;
    }

    // Modo interactivo
    bool running = true;

    while (running) {
        printf("Enter command (or 'exit' to quit): ");
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);

        if (strncmp(buffer, "exit", 4) == 0) {
            running = false;
            continue;
        }

        send_command(sockfd, buffer);

        memset(buffer, 0, sizeof(buffer));
        status_r = recv(sockfd, buffer, sizeof(buffer), 0);

        if (status_r > 0) {
            printf("Server: %s", buffer);
        } else {
            perror("recv");
            running = false;
        }
    }

    close(sockfd);
    return 0;
}
