#include "client_utils.h"
#include <string.h>

int build_command_string(int argc, char *argv[], char *buffer, size_t buffer_size) {
    if (!buffer || buffer_size == 0) return -1;

    size_t offset = 0;
    for (int i = 1; i < argc && offset < buffer_size - 1; i++) {
        size_t arg_len = strnlen(argv[i], buffer_size - offset - 1);
        if (offset + arg_len >= buffer_size - 1) {
            return -1;
        }

        memcpy(buffer + offset, argv[i], arg_len);
        offset += arg_len;

        if (i < argc - 1) {
            if (offset + 1 >= buffer_size - 1) {
                return -1;
            }
            buffer[offset++] = ' ';
        }
    
    }

    buffer[offset] = '\0';
    return 0;
}