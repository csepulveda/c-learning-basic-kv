#include <string.h>
#include <stdio.h>

#include "protocol.h"
#include "kvstore.h"
#include "errors.h"

/**
 * @brief Determines the command type from a protocol message string.
 *
 * Analyzes the beginning of the input message to identify if it matches a known command
 * ("SET", "GET", "DEL", "PING", "TIME" ) followed by a valid delimiter.
 *
 * @param message The input message string to parse.
 * @return command_t The corresponding command type, or CMD_UNKNOWN if no match is found.
 */
command_t parse_command(const char *message) {
    if (strncmp(message, "SET", 3) == 0 && IS_SIMPLE_CMD_TERMINATOR(message[3])) return CMD_SET;
    if (strncmp(message, "GET", 3) == 0 && IS_SIMPLE_CMD_TERMINATOR(message[3])) return CMD_GET;
    if (strncmp(message, "DEL", 3) == 0 && IS_SIMPLE_CMD_TERMINATOR(message[3])) return CMD_DEL;

    if (strncmp(message, "MSET", 4) == 0 && IS_SIMPLE_CMD_TERMINATOR(message[4])) return CMD_MSET;
    if (strncmp(message, "MGET", 4) == 0 && IS_SIMPLE_CMD_TERMINATOR(message[4])) return CMD_MGET;

    if (strncmp(message, "PING", 4) == 0 && IS_CMD_TERMINATOR(message[4])) return CMD_PING;
    if (strncmp(message, "TIME", 4) == 0 && IS_CMD_TERMINATOR(message[4])) return CMD_TIME;
    
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
