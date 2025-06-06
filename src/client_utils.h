#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <stddef.h>

int build_command_string(int argc, char *argv[], char *buffer, size_t buffer_size);
int send_command(int sockfd, const char *command);

#endif