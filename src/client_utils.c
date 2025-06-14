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

static bool append_char(char *buffer, size_t *offset, size_t buffer_size, char c) {
    if (*offset + 1 >= buffer_size) return false;
    buffer[(*offset)++] = c;
    return true;
}

static bool append_quoted_word(const char *word, char *buffer, size_t *offset, size_t buffer_size) {
    if (!append_char(buffer, offset, buffer_size, '"')) return false;

    for (size_t j = 0; word[j] != '\0'; j++) {
        if ((word[j] == '"' && !append_char(buffer, offset, buffer_size, '\\')) ||
            !append_char(buffer, offset, buffer_size, word[j])) {
            return false;
        }
    }

    return append_char(buffer, offset, buffer_size, '"');
}

static bool append_unquoted_word(const char *word, char *buffer, size_t *offset, size_t buffer_size) {
    for (size_t j = 0; word[j] != '\0'; j++) {
        if (!append_char(buffer, offset, buffer_size, word[j])) return false;
    }
    return true;
}

int build_command_string(int argc, char *argv[], char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;

    size_t offset = 0;

    for (int i = 1; i < argc; i++) {
        const char *word = argv[i];
        if (i > 1 && !append_char(buffer, &offset, buffer_size, ' ')) {
            log_error("Buffer overflow while building command string");
            return -1;
        }

        bool needs_quotes = strchr(word, ' ') != NULL;
        bool success = needs_quotes
            ? append_quoted_word(word, buffer, &offset, buffer_size)
            : append_unquoted_word(word, buffer, &offset, buffer_size);

        if (!success) {
            log_error("Command string too long or invalid: %s", word);
            return -1;
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