#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#define BUFFER_SIZE 1024

void* handle_client(void *arg);
void dispatch_command(int clientfd, const char *buffer);

#endif
