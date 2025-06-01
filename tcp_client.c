#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    int sockfd, status;
    struct sockaddr_in addr;
    char text[255];

    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return sockfd;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(8080);
    inet_aton("127.0.0.1", &addr.sin_addr);

    status = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr));
    if (status) {
        perror("connect");
        close(sockfd);
        return status;
    }

    printf("Connected to server\n");

    while (1) {
        memset(text, 0, sizeof(text));
        printf("Enter message to send to server (or 'exit' to quit): ");
        
        memset(text, 0, sizeof(text));
        fgets(text, sizeof(text), stdin);
        if (strncmp(text, "exit", 4) == 0) {
            break;
        }

        send(sockfd, text, strlen(text), 0);

        memset(text, 0, sizeof(text));
        status = recv(sockfd, text, sizeof(text), 0);
        if (status > 0) {
            printf("Response from server: %s", text);
        } else {
            perror("recv");
            break;
        }
    }

    close(sockfd);
    return 0;
}


