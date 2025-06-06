#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <inttypes.h>
#include "logs.h"

#include "protocol.h"

#define BUFFER_SIZE 1024

int send_command(int sockfd, const char *command) {
    struct timeval start, end;
    gettimeofday(&start, NULL); 
    
    int n = send(sockfd, command, strlen(command), 0);
   
    gettimeofday(&end, NULL);
    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000ULL + (end.tv_usec - start.tv_usec);
    if (strcmp(command, "PING") == 0) {
        printf("Ping response time: %" PRIu64 " microseconds\n", delta_us);
    }
    
    return n;
}

int main(int argc, char *argv[]) {
    int sockfd, status;
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
    addr.sin_port = htons(server_port);
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
        for (int i = 1; i < argc; i++) {
            strncat(command, argv[i], sizeof(command) - strlen(command) - 1);
            if (i < argc - 1) strncat(command, " ", sizeof(command) - strlen(command) - 1);
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
    while (1) {
        printf("Enter command (or 'exit' to quit): ");
        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        send_command(sockfd, buffer);

        memset(buffer, 0, sizeof(buffer));
        status = recv(sockfd, buffer, sizeof(buffer), 0);
        if (status > 0) {
            printf("Server: %s", buffer);
        } else {
            perror("recv");
            break;
        }
    }

    close(sockfd);
    return 0;
}
