#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stddef.h>

#define IS_CMD_TERMINATOR(c) ((c) == ' ' || (c) == '\0' || (c) == '\n' || (c) == '\r')
#define IS_SIMPLE_CMD_TERMINATOR(c) ((c) == ' ')

typedef enum {
    CMD_PING,
    CMD_TIME,
    CMD_INFO,
    CMD_SET,
    CMD_GET,
    CMD_DEL,
    CMD_MSET,
    CMD_MGET,
    CMD_TYPE,
    CMD_UNKNOWN = -1
} command_t;

command_t parse_command(const char *message); 
int extract_key_value(const char *message, char *key, char *value, size_t key_size, size_t value_size);
int extract_key(const char *message, char *key, size_t key_size);

#endif
