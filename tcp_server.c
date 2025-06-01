#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

int parse_message(char *message) {
    if (strncmp(message, "PING", 4) == 0){
        return 1;
    } else if (strncmp(message, "TIME", 4) == 0) {
        return 2;
    } else if (strncmp(message, "GOODBYE", 7) == 0) {
        return 3;
    } else {
        printf("Unknown message: %s\n", message);
        return -1;
    }
    return 0;
}

void* handle_client(void *arg) {

    int clientfd = (int)(long)arg;
    char text[255];
    int status, req_type;

    memset(text, 0, sizeof(text));
    printf("Client connected, waiting for message... (fd:  %d)\n", clientfd);


    while (1) {

        if (clientfd < 0) {
            printf("Invalid client file descriptor.\n");
            return (void *)(long)clientfd;
        }

        status = recv(clientfd, text, sizeof(text), 0);
        if (status < 0) {
            perror("recv");
            close(clientfd);
            return (void *)(long)status;
        }

        req_type = parse_message(text);
    
        if (req_type == 1) {
            memset(text, 0, sizeof(text));
            sprintf(text, "PONG!\n");
            send(clientfd, text, strlen(text), 0);
        }
        else if (req_type == 2) {
            memset(text, 0, sizeof(text));
            sprintf(text, "Current time is: %ld\n", time(NULL));
            send(clientfd, text, strlen(text), 0);
        }
        else if (req_type == 3) {
            printf("Client requested to close connection.\n");
            close(clientfd);
            return (void *)0;
        }
        else if (req_type == -1) {
            printf("Invalid request received: %s\n", text);
            memset(text, 0, sizeof(text));
            sprintf(text, "Invalid request. Please send PING, TIME, or GOODBYE.\n");
            send(clientfd, text, strlen(text), 0);
        }
        else {
            printf("Unknown request type: %d\n", req_type);
            close(clientfd);
            break;
        }
    }

    return (void *)0;
}


int main() {
    int sockfd, clientfd, status, opt = 1;
    struct sockaddr_in addr, client_addr;
    char text[255];
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t thread_id;
    void *ret;

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return sockfd;
    }

    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;


    status = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if (status) {
        perror("bind");
        close(sockfd);
        return status;
    }

    status = listen(sockfd, 1);
    if (status) {
        perror("listen");
        close(sockfd);
        return status;
    }

    printf("Server is running...\n");
    printf("Server IP: %s\n", inet_ntoa(addr.sin_addr));
    printf("Listening on port %d\n", ntohs(addr.sin_port));


    while (1) {
        clientfd = accept(sockfd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (clientfd < 0) {
            perror("accept");
            continue;
        }

        printf("Client connected (fd: %d)\n", clientfd);
        printf("Client IP: %s\n", inet_ntoa(client_addr.sin_addr));
        printf("Client Port: %d\n", ntohs(client_addr.sin_port));

        if (pthread_create(&thread_id, NULL, handle_client, (void *)(long)clientfd) != 0) {
            perror("pthread_create");
            close(clientfd);
            continue;
        }
        pthread_detach(thread_id);
    }

    close(sockfd);
    return 0;
}



