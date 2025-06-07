#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "commands.h"
#include "protocol.h"
#include "server_utils.h"

/**
 * @brief Parses and dispatches a client command to the appropriate handler.
 *
 * Determines the command type from the input buffer and invokes the corresponding handler function for the client socket. Sends an "Invalid command" message if the command is unrecognized.
 *
 * @param clientfd File descriptor for the client socket.
 * @param buffer Null-terminated string containing the client's command.
 */
void dispatch_command(int clientfd, const char *buffer) {
    command_t cmd = parse_command(buffer);

    switch (cmd) {
        case CMD_PING:
            handle_command(clientfd, CMD_PING, "");
            break;
        case CMD_TIME:
            handle_command(clientfd, CMD_TIME, "");
            break;
        case CMD_SET:
            handle_command(clientfd, CMD_SET, buffer);
            break;
        case CMD_GET:
            handle_command(clientfd, CMD_GET, buffer);
            break;
        case CMD_DEL:
            handle_command(clientfd, CMD_DEL, buffer);
            break;
        case CMD_GOODBYE:
            handle_command(clientfd, CMD_GOODBYE, "");
            break;
        case CMD_UNKNOWN:
        default:
            send(clientfd, "Invalid command\n", 17, 0);
            break;
    }
}

/**
 * @brief Handles communication with a connected client over a socket.
 *
 * Continuously receives commands from the client, dispatches each command for processing,
 * and closes the connection when the client disconnects or an error occurs.
 *
 * @param arg Pointer to the client socket file descriptor (cast from void*).
 * @return Always returns NULL upon client disconnection or error.
 */
void* handle_client(void *arg) {
    int clientfd = (int)(intptr_t)arg;
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes = recv(clientfd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;

        dispatch_command(clientfd, buffer);
    }

    close(clientfd);
    return NULL;
}

