#include <stdio.h> 
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/socket.h>
#include <inttypes.h>

#include "logs.h"
#include "client_utils.h"
#include "protocol.h"
#include "commands.h"

#define BUFFER_SIZE 1024

int build_command_string(int argc, char *argv[], char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;

    size_t offset = 0;
    for (int i = 1; i < argc && offset < buffer_size - 1; i++) {
        size_t arg_len = strnlen(argv[i], buffer_size - offset - 1);
        if (offset + arg_len >= buffer_size - 1) {
            return -1;
        }

        memcpy(buffer + offset, argv[i], arg_len);
        offset += arg_len;

        if (i < argc - 1) {
            if (offset + 1 >= buffer_size - 1) {
                return -1;
            }
            buffer[offset++] = ' ';
        }
    
    }

    buffer[offset] = '\0';
    return 0;
}

int send_command(int sockfd, const char *command) {
    struct timeval start, end;
    gettimeofday(&start, NULL); 

    size_t cmd_len = strnlen(command, BUFFER_SIZE);
    if (cmd_len >= BUFFER_SIZE) {
        log_error("Command too long: %s", command);
        return -1;
    }
    int n = send(sockfd, command, cmd_len, 0);

    gettimeofday(&end, NULL);
    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000ULL + (end.tv_usec - start.tv_usec);
    if (strcmp(command, "PING") == 0) {
        printf("Ping response time: %" PRIu64 " microseconds\n", delta_us);
    }
    
    return n;
}