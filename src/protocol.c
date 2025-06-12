#include <string.h>
#include <stdio.h>

#include "protocol.h"
#include "kvstore.h"
#include "errors.h"

typedef struct {
    const char *name;
    size_t len;
    bool simple_terminator;
    command_t cmd;
} command_def_t;

command_t parse_command(const char *message) {
    static const command_def_t commands[] = {
        { "SET",     3, true,  CMD_SET },
        { "GET",     3, true,  CMD_GET },
        { "DEL",     3, true,  CMD_DEL },
        { "HSET",    4, true,  CMD_HSET },
        { "HGET",    4, true,  CMD_HGET },
        { "HMGET",   5, true,  CMD_HMGET },
        { "HINCRBY", 7, true,  CMD_HINCRBY },
        { "TYPE",    4, true,  CMD_TYPE },
        { "MSET",    4, true,  CMD_MSET },
        { "MGET",    4, true,  CMD_MGET },
        { "PING",    4, false, CMD_PING },
        { "INFO",    4, false, CMD_INFO },
        { "TIME",    4, false, CMD_TIME },
    };

    const size_t num_commands = sizeof(commands) / sizeof(commands[0]);

    for (size_t i = 0; i < num_commands; i++) {
        const command_def_t *c = &commands[i];

        if (strncmp(message, c->name, c->len) != 0) continue;

        char terminator = message[c->len];

        bool is_valid_terminator = c->simple_terminator
            ? IS_SIMPLE_CMD_TERMINATOR(terminator)
            : IS_CMD_TERMINATOR(terminator);

        if (is_valid_terminator) return c->cmd;
    }

    return CMD_UNKNOWN;
}

int extract_key_value(const char *message, char *key, char *value, size_t key_size, size_t value_size) {
    const char *space1 = strchr(message, ' ');
    if (!space1) return EXTRACT_ERR_PARSE;

    const char *space2 = strchr(space1 + 1, ' ');
    if (!space2) return EXTRACT_ERR_PARSE;

    // Extract key
    size_t key_len = space2 - (space1 + 1);
    if (key_len >= key_size || key_len >= MAX_KEY_LEN - 1) return EXTRACT_ERR_KEY_TOO_LONG;

    memcpy(key, space1 + 1, key_len);
    key[key_len] = '\0';

    // Skip spaces after key
    const char *value_start = space2 + 1;
    while (*value_start == ' ') value_start++;
    if (*value_start == '\0') return EXTRACT_ERR_PARSE;

    // Check if value is quoted
    if (*value_start == '"') {
        value_start++;
        const char *quote_end = strchr(value_start, '"');
        if (!quote_end) return EXTRACT_ERR_PARSE;

        size_t value_len = quote_end - value_start;
        if (value_len >= value_size || value_len >= MAX_VAL_LEN - 1) return EXTRACT_ERR_VALUE_TOO_LONG;

        memcpy(value, value_start, value_len);
        value[value_len] = '\0';
    } else {
        // Unquoted value: take until newline
        size_t value_len = strnlen(value_start, value_size);
        if (value_len >= value_size - 1 || value_len >= MAX_VAL_LEN - 1) return EXTRACT_ERR_VALUE_TOO_LONG;

        snprintf(value, value_size, "%s", value_start);
        char *newline = strchr(value, '\n');
        if (newline) *newline = '\0';
    }

    return EXTRACT_OK;
}

int extract_key(const char *message, char *key, size_t key_size) {
    const char *space = strchr(message, ' ');
    if (!space) return -1;

    space++;
    while (*space == ' ') space++;
    if (*space == '\0') return -1;

    snprintf(key, key_size, "%s", space);

    char *newline = strchr(key, '\n');
    if (newline) *newline = '\0';

    return 0;
}
