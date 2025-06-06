#include <string.h>
#include <stdio.h>
#include "protocol.h"

command_t parse_command(const char *message) {
    if (strncmp(message, "PING", 4) == 0) return CMD_PING;
    if (strncmp(message, "TIME", 4) == 0) return CMD_TIME;
    if (strncmp(message, "GOODBYE", 7) == 0) return CMD_GOODBYE;
    if (strncmp(message, "SET ", 4) == 0) return CMD_SET;
    if (strncmp(message, "GET ", 4) == 0) return CMD_GET;
    if (strncmp(message, "DEL ", 4) == 0) return CMD_DEL;
    return CMD_UNKNOWN;
}

int extract_key_value(const char *message, char *key, char *value, size_t key_size, size_t value_size) {
    const char *space = strchr(message, ' ');
    if (!space) return -1;

    const char *equal = strchr(space + 1, '=');
    if (!equal) return -1;

    size_t key_len = equal - (space + 1);
    size_t value_len = strnlen(equal + 1, value_size); //NOSONAR

    if (key_len >= key_size || value_len >= value_size) return -1;

    memcpy(key, space + 1, key_len);
    key[key_len] = '\0';

    snprintf(value, value_size, "%s", equal + 1);

    char *newline = strchr(value, '\n');
    if (newline) *newline = '\0';

    return 0;
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
