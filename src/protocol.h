#ifndef PROTOCOL_H
#define PROTOCOL_H
#define BUFFER_SIZE 1024

typedef enum {
    CMD_PING,
    CMD_TIME,
    CMD_GOODBYE,
    CMD_SET,
    CMD_GET,
    CMD_DEL,
    CMD_UNKNOWN = -1
} command_t;

command_t parse_command(const char *message); 
int extract_key_value(const char *message, char *key, char *value, size_t key_size, size_t value_size);
int extract_key(const char *message, char *key, size_t key_size);

#endif
