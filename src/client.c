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
#include "errors.h"

#ifndef VERSION
#define VERSION "dev"
#endif

/**
 * @brief Entry point for the TCP client application.
 *
 * Connects to a server using IP and port from environment variables or defaults, then sends commands either from command-line arguments or interactively. Prints server responses and handles connection errors gracefully.
 *
 * @return 0 on success, 1 on failure.
 */
int main(int argc, char *argv[]) {
    log_info("Version: %s\n", VERSION);
    int sockfd;
    int status;
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

    // Line command mode
    if (argc > 1) {
        char command[BUFFER_SIZE] = {0};
        if (build_command_string(argc, argv, command, sizeof(command)) != 0) {
            log_error("Failed to construct command\n");
            close(sockfd);
            return 1;
        }

        command_t cmd = parse_command(command);
        if (cmd == CMD_UNKNOWN) {
            char command_name[64];
            sscanf(command, "%s", command_name);
            log_error("Invalid command: %s\n", command_name);
            close(sockfd);
            return 1;
        }

        if (build_command_string(argc, argv, command, sizeof(command)) != 0) {
            log_error("Failed to construct command\n");
            close(sockfd);
            return 1;
        }

        send_command(sockfd, command);
        read_response(sockfd);

        close(sockfd);
        return 0;
    }

    // Interactive mode
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
        read_response(sockfd);
    }

    close(sockfd);
    return 0;
}