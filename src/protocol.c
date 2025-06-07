#include <string.h>
#include <stdio.h>

#include "protocol.h"

/**
 * @brief Determines the command type from a protocol message string.
 *
 * Analyzes the beginning of the input message to identify if it matches a known command
 * ("SET", "GET", "DEL", "PING", "TIME", or "GOODBYE") followed by a valid delimiter.
 *
 * @param message The input message string to parse.
 * @return command_t The corresponding command type, or CMD_UNKNOWN if no match is found.
 */
command_t parse_command(const char *message) {
    if (strncmp(message, "SET", 3) == 0 && (message[3] == ' ' )) return CMD_SET;
    if (strncmp(message, "GET", 3) == 0 && (message[3] == ' ' )) return CMD_GET;
    if (strncmp(message, "DEL", 3) == 0 && (message[3] == ' ' )) return CMD_DEL;
    
    if (strncmp(message, "PING", 4) == 0 && (message[4] == ' ' || message[4] == '\0' || message[4] == '\n')) return CMD_PING;
    if (strncmp(message, "TIME", 4) == 0 && (message[4] == ' ' || message[4] == '\0' || message[4] == '\n')) return CMD_TIME;
    
    if (strncmp(message, "GOODBYE", 7) == 0 && (message[7] == ' ' || message[7] == '\0' || message[7] == '\n')) return CMD_GOODBYE;
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
