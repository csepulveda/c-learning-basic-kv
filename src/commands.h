#ifndef COMMANDS_H
#define COMMANDS_H

#include "protocol.h"

typedef void (*command_proc_t)(int clientfd, const char *message);

typedef struct {
    command_t cmd;
    command_proc_t proc;
} command_entry_t;

void handle_command(int clientfd, command_t cmd, const char *message);
void cmd_set(int clientfd, const char *buffer);
void cmd_get(int clientfd, const char *buffer);
void cmd_mset(int clientfd, const char *buffer);
void cmd_mget(int clientfd, const char *buffer);
void cmd_del(int clientfd, const char *buffer);
void cmd_ping(int clientfd, const char *message);
void cmd_time(int clientfd, const char *message);
void cmd_info(int clientfd, const char *buffer);
void cmd_type(int clientfd, const char *message);
void cmd_hset(int clientfd, const char *buffer);
void cmd_hget(int clientfd, const char *buffer);
void cmd_hmget(int clientfd, const char *buffer);
void cmd_hincrby(int clientfd, const char *buffer);

void send_response_header(int clientfd, const char *type);
void send_response_footer(int clientfd);
void send_error_response(int clientfd, int res);

int extract_key_from_ptr(const char **p, char *key, size_t key_size);
int extract_value_from_ptr(const char **p, char *value, size_t value_size);
int extract_key_value(const char *message, char *key, char *value, size_t key_size, size_t value_size);
int extract_key_field(const char *message, char *key, size_t key_size, char *field, size_t field_size);


#endif
