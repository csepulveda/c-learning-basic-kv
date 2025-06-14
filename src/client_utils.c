#include <inttypes.h>
#include <stdint.h>
#include <stdio.h> 
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdbool.h>

#include "client_utils.h"
#include "logs.h"
#include "errors.h"

/**
 * @brief Constructs a command string from command-line arguments.
 *
 * Joins command-line arguments from argv[1] to argv[argc-1] into a single space-separated string,
 * storing the result in the provided buffer. Ensures the buffer is not overflowed and is properly null-terminated.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of argument strings.
 * @param buffer Destination buffer for the constructed command string.
 * @param buffer_size Size of the destination buffer in bytes.
 * @return 0 on success, or -1 if the buffer is invalid or too small.
 */
int build_command_string(int argc, char *argv[], char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;

    size_t offset = 0;

    for (int i = 1; i < argc; i++) {
        const char *word = argv[i];
        size_t word_len = strnlen(word, buffer_size);

        if (i > 1) {
            if (offset + 1 >= buffer_size) {
                log_error("Buffer overflow while building command string");
                return -1;
            }
            buffer[offset++] = ' ';
        }

        bool needs_quotes = strchr(word, ' ') != NULL;
        if (needs_quotes) {
            if (offset + 1 >= buffer_size) return -1;
            buffer[offset++] = '"';
        }
        if (offset + word_len >= buffer_size) {
            log_error("Command string too long: %s", word);
            return -1;
        }

        for (size_t j = 0; j < word_len; ++j) {
            if (needs_quotes && word[j] == '"') {
                if (offset + 2 >= buffer_size) return -1;
                buffer[offset++] = '\\';
            }
            if (offset + 1 >= buffer_size) return -1;
            buffer[offset++] = word[j];
        }
        if (needs_quotes) {
            if (offset + 1 >= buffer_size) return -1;
            buffer[offset++] = '"';
        }
    }

    buffer[offset < buffer_size ? offset : buffer_size - 1] = '\0';
    return 0;
}

/**
 * @brief Sends a command string over a socket and measures transmission time.
 *
 * Sends the specified command over the given socket file descriptor. If the command is "PING", prints the round-trip time in microseconds. Returns the number of bytes sent, or -1 if the command is too long.
 *
 * @param sockfd Socket file descriptor to send the command through.
 * @param command Null-terminated command string to send.
 * @return int Number of bytes sent, or -1 on error.
 */
int send_command(int sockfd, const char *command) {
    struct timeval start;
    struct timeval end;
    gettimeofday(&start, NULL); 

    size_t cmd_len = strnlen(command, BUFFER_SIZE);
    if (cmd_len >= BUFFER_SIZE) {
        log_error("Command too long: %s", command);
        return -1;
    }
    ssize_t bytes_sent = send(sockfd, command, cmd_len, 0);

    gettimeofday(&end, NULL);
    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000ULL + (end.tv_usec - start.tv_usec);
    if (strcmp(command, "PING") == 0) {
        printf("Ping response time: %" PRIu64 " microseconds\n", delta_us);
    }

    return (int)bytes_sent;
}


bool handle_char(char c, char *line_buffer, size_t *line_pos, bool *first_line) {
    if (c == '\n') {
        line_buffer[*line_pos] = '\0';

        if (strcmp(line_buffer, "END") == 0) {
            *line_pos = 0; 
            return true;  
        }

        if (!(*first_line && strncmp(line_buffer, "RESPONSE", 8) == 0)) {
            printf("%s\n", line_buffer);
        }

        *first_line = false;
        *line_pos = 0; 
    } else if (*line_pos < BUFFER_SIZE - 1) {
        line_buffer[(*line_pos)++] = c;
    }

    return false; 
}

void read_response(int sockfd) {
    ssize_t status_r;
    char buffer[BUFFER_SIZE];
    char line_buffer[BUFFER_SIZE] = {0};
    size_t line_pos = 0;
    bool found_end = false;
    bool first_line = true;

    while ((status_r = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[status_r] = '\0';

        for (int i = 0; i < status_r; i++) {
            if (handle_char(buffer[i], line_buffer, &line_pos, &first_line)) {
                found_end = true;
                break;
            }
        }

        if (found_end) break;
    }

    if (status_r <= 0 && !found_end) {
        perror("recv");
        log_error(ERR_INTERNAL_ERROR);
    }
}