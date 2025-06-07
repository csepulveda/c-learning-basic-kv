#ifndef COMMANDS_H
#define COMMANDS_H

#include "protocol.h"

typedef void (*command_proc_t)(int clientfd, const char *message);

typedef struct {
    command_t cmd;
    command_proc_t proc;
} command_entry_t;

void handle_command(int clientfd, command_t cmd, const char *message);

#endif
