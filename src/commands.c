#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "commands.h"
#include "kv_store.h"
#include "protocol.h"

#define BUFFER_SIZE 1024
#define MAX_VAL_SIZE 1024

static void cmd_ping(int clientfd, const char *message);
static void cmd_time(int clientfd, const char *message);
static void cmd_goodbye(int clientfd, const char *message);
static void cmd_set(int clientfd, const char *message);
static void cmd_get(int clientfd, const char *message);
static void cmd_del(int clientfd, const char *message);

static command_entry_t command_table[] = {
    { CMD_PING,    cmd_ping },
    { CMD_TIME,    cmd_time },
    { CMD_GOODBYE, cmd_goodbye },
    { CMD_SET,     cmd_set },
    { CMD_GET,     cmd_get },
    { CMD_DEL,     cmd_del },
    { CMD_UNKNOWN, NULL }  // Sentinel
};


void handle_command(int clientfd, command_t cmd, const char *message) {
    for (int i = 0; command_table[i].proc != NULL; i++) {
        if (command_table[i].cmd == cmd) {
            command_table[i].proc(clientfd, message);
            return;
        }
    }

    dprintf(clientfd, "ERR unknown command\n");
}

/**
 * @brief Sends the current server time to the client.
 *
 * Formats the current system time as a human-readable string and transmits it to the connected client.
 */
static void cmd_ping(int clientfd, const char *message) {
    (void)message;
    const char *response = "PONG\n";
    send(clientfd, response, strlen(response), 0); //NOSONAR
}

static void cmd_time(int clientfd, const char *message) {
    (void)message;
    time_t now = time(NULL);
    char timestr[BUFFER_SIZE];
    ctime_r(&now, timestr);
    send(clientfd, timestr, sizeof(timestr) -1 , 0);
}

static void cmd_goodbye(int clientfd, const char *message) {
    (void)message;
    const char *response = "Goodbye!\n";
    send(clientfd, response, strlen(response), 0); //NOSONAR
    close(clientfd);
}

void cmd_set(int clientfd, const char *buffer) {
    char key[128], value[128];
    if (extract_key_value(buffer, key, value, sizeof(key), sizeof(value)) == 0) {
        if (kv_set(key, value) == 0) {
            send(clientfd, "OK\n", 3, 0);
        } else {
            send(clientfd, "ERROR\n", 6, 0);
        }
    } else {
        send(clientfd, "ERROR\n", 6, 0);
    }
}

static void cmd_get(int clientfd, const char *buffer) {
    char key[128];
    if (extract_key(buffer, key, sizeof(key)) == 0) {
        const char *val = kv_get(key);
        if (val) {
	    size_t len = strnlen(val, MAX_VAL_SIZE);
            send(clientfd, val, len, 0);
            send(clientfd, "\n", 1, 0);
        } else {
            send(clientfd, "NOT FOUND\n", 10, 0);
        }
    } else {
        send(clientfd, "ERROR\n", 6, 0);
    }
}

/**
 * @brief Handles a delete command from the client to remove a key from the key-value store.
 *
 * Parses the key from the input buffer and attempts to delete it from the store. Sends "DELETED" if successful, "NOT FOUND" if the key does not exist, or "ERROR" if the key could not be parsed.
 */
static void cmd_del(int clientfd, const char *buffer) {
    char key[128];
    if (extract_key(buffer, key, sizeof(key)) == 0) { 
        if (kv_delete(key) == 0) {
            send(clientfd, "DELETED\n", 8, 0);
        } else {
            send(clientfd, "NOT FOUND\n", 10, 0);
        }
    } else {
        send(clientfd, "ERROR\n", 6, 0);
    }
}
