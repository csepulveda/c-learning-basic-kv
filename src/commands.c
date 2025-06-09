#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "commands.h"
#include "kv_store.h"
#include "protocol.h"
#include "errors.h"

#define BUFFER_SIZE 1024

static command_entry_t command_table[] = {
    { CMD_PING,    cmd_ping },
    { CMD_TIME,    cmd_time },
    { CMD_SET,     cmd_set },
    { CMD_GET,     cmd_get },
    { CMD_MSET,    cmd_mset },
    { CMD_MGET,    cmd_mget },
    { CMD_DEL,     cmd_del },
    { CMD_UNKNOWN, NULL }  // Sentinel
};

void send_response_header(int clientfd, const char *type) {
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "RESPONSE %s\n", type);
    int len = snprintf(header, sizeof(header), "RESPONSE %s\n", type);
    send(clientfd, header, len, 0);
}

void send_response_footer(int clientfd) {
    send(clientfd, "END\n", 4, 0);
}

void send_error_response(int clientfd, int res) {
    send_response_header(clientfd, "ERROR");

    const char *msg;

    switch (res) {
        case EXTRACT_ERR_KEY_TOO_LONG:
            msg = ERR_KEY_TOO_LONG;
            break;
        case EXTRACT_ERR_VALUE_TOO_LONG:
            msg = ERR_VALUE_TOO_LONG;
            break;
        case EXTRACT_ERR_KEY_NOT_FOUND:
            msg = ERR_NOT_FOUND;
            break;
        case EXTRACT_ERR_PARSE:
        default:
            msg = ERR_PARSE_ERROR;
            break;
    }

    send(clientfd, msg, strlen(msg), 0); //NOSONAR
    send_response_footer(clientfd);
}

int extract_key_from_ptr(const char **p, char *key, size_t key_size) {
    const char *key_start = *p;
    while (**p != ' ' && **p != '\0' && **p != '\n') (*p)++;
    size_t key_len = *p - key_start;
    if (key_len >= key_size) return EXTRACT_ERR_KEY_TOO_LONG;
    memcpy(key, key_start, key_len);
    key[key_len] = '\0';

    while (**p == ' ') (*p)++;
    if (**p == '\0' || **p == '\n') return EXTRACT_ERR_PARSE;

    return EXTRACT_OK;
}

int extract_value_from_ptr(const char **p, char *value, size_t value_size) {
    if (**p == '"') {
        (*p)++;
        const char *value_start = *p;
        const char *quote_end = strchr(*p, '"');
        if (!quote_end) return EXTRACT_ERR_PARSE;

        size_t value_len = quote_end - value_start;
        if (value_len >= value_size) return EXTRACT_ERR_VALUE_TOO_LONG;

        memcpy(value, value_start, value_len);
        value[value_len] = '\0';
        *p = quote_end + 1;
    } else {
        const char *value_start = *p;
        while (**p != ' ' && **p != '\0' && **p != '\n') (*p)++;
        size_t value_len = *p - value_start;
        if (value_len >= value_size) return EXTRACT_ERR_VALUE_TOO_LONG;

        memcpy(value, value_start, value_len);
        value[value_len] = '\0';
    }

    while (**p == ' ') (*p)++;
    return EXTRACT_OK;
}

static void send_simple_ok_string(int clientfd, const char *msg) {
    send_response_header(clientfd, "OK STRING");
    send(clientfd, msg, strnlen(msg, BUFFER_SIZE), 0);
    send_response_footer(clientfd);
}

void handle_command(int clientfd, command_t cmd, const char *message) {
    for (int i = 0; command_table[i].proc != NULL; i++) {
        if (command_table[i].cmd == cmd) {
            command_table[i].proc(clientfd, message);
            return;
        }
    }
}

void cmd_ping(int clientfd, const char *message) {
    (void)message;
    send_response_header(clientfd, "OK STRING");

    const char *response = "PONG\n";
    send(clientfd, response, 5, 0);

    send_response_footer(clientfd);
}

void cmd_time(int clientfd, const char *message) {
    (void)message;
    send_response_header(clientfd, "OK STRING");

    time_t now = time(NULL);
    char timestr[BUFFER_SIZE];
    ctime_r(&now, timestr);
    send(clientfd, timestr, strlen(timestr), 0); //NOSONAR

    send_response_footer(clientfd);
}

void cmd_set(int clientfd, const char *buffer) {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
    int res = extract_key_value(buffer, key, value, sizeof(key), sizeof(value));

    if (res != EXTRACT_OK) {
        send_error_response(clientfd, res);
        return;
    }

    if (kv_set(key, value) == 0) {
        send_simple_ok_string(clientfd, "OK\n");
    } else {
        send_error_response(clientfd, EXTRACT_ERR_INTERNAL);
    }
}

void cmd_get(int clientfd, const char *buffer) {
    char key[MAX_KEY_LEN];
    if (extract_key(buffer, key, sizeof(key)) != 0) {
        send_error_response(clientfd, EXTRACT_ERR_PARSE);
        return;
    }

    const char *val = kv_get(key);

    send_response_header(clientfd, "OK STRING");

    if (val) {
        size_t len = strnlen(val, MAX_VAL_LEN);
        send(clientfd, val, len, 0);
        send(clientfd, "\n", 1, 0);
    } else {
        send(clientfd, ERR_NOT_FOUND, strlen(ERR_NOT_FOUND), 0);
    }

    send_response_footer(clientfd);
}

void cmd_del(int clientfd, const char *buffer) {
    char key[MAX_KEY_LEN];
    if (extract_key(buffer, key, sizeof(key)) != 0) {
        send_error_response(clientfd, EXTRACT_ERR_PARSE);
        return;
    }

    if (kv_delete(key) == 0) {
        send_simple_ok_string(clientfd, "DELETED\n");
    } else {
        send_error_response(clientfd, EXTRACT_ERR_KEY_NOT_FOUND);
    }
}

void cmd_mset(int clientfd, const char *buffer) {
    char copy[BUFFER_SIZE];
    snprintf(copy, sizeof(copy), "%s", buffer);

    const char *p = copy + 5;  // Skip "MSET "
    while (*p == ' ') p++;

    while (*p != '\0' && *p != '\n') {
        char key[MAX_KEY_LEN];
        char value[MAX_VAL_LEN];

        int key_res = extract_key_from_ptr(&p, key, MAX_KEY_LEN);
        if (key_res != EXTRACT_OK) {
            send_error_response(clientfd, key_res);
            return;
        }

        int value_res = extract_value_from_ptr(&p, value, MAX_VAL_LEN);
        if (value_res != EXTRACT_OK) {
            send_error_response(clientfd, value_res);
            return;
        }

        if (kv_set(key, value) != 0) {
            send_error_response(clientfd, EXTRACT_ERR_INTERNAL);
            return;
        }
    }

    send_response_header(clientfd, "OK STRING");
    send(clientfd, "OK\n", 3, 0);
    send_response_footer(clientfd);
}

void cmd_mget(int clientfd, const char *buffer) {
    char copy[BUFFER_SIZE];
    snprintf(copy, sizeof(copy), "%s", buffer);

    char *newline = strchr(copy, '\n');
    if (newline) *newline = '\0';

    char *saveptr = NULL;
    char *token = strtok_r(copy + 5, " ", &saveptr); //NOSONAR

    send_response_header(clientfd, "OK MULTI");

    int index = 1;
    while (token != NULL) {
        const char *key = token;
        const char *val = kv_get(key);

        char line[BUFFER_SIZE];
        int len = (val && val[0] != '\0')
            ? snprintf(line, sizeof(line), "%d) %s\n", index, val)
            : snprintf(line, sizeof(line), "%d) (nil)\n", index);

        send(clientfd, line, len, 0);

        index++;
        token = strtok_r(NULL, " ", &saveptr);
    }

    send_response_footer(clientfd);
}