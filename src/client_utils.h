#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <stddef.h>

#define BUFFER_SIZE 1024

int send_command(int sockfd, const char *command);
int build_command_string(int argc, char *argv[], char *buffer, size_t buffer_size);
void read_response(int sockfd);
bool handle_char(char c, char *line_buffer, size_t *line_pos, bool *first_line);

#endif
