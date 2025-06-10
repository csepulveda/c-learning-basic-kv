#include <string.h>
#include <time.h>

#include "info.h"

#ifndef VERSION
#define VERSION "dev"
#endif

server_info_t fill_data(int mem, int keys, long uptime, const char *version) {
    server_info_t r;
    r.mem = mem;
    r.keys = keys;
    r.uptime = uptime;
    strncpy(r.version, version, strlen(version));
    return r;
}

server_info_t get_info(void) {
    time_t now = time(NULL);
    long uptime = now - start_time;
    server_info_t r = fill_data(30, 150, uptime, VERSION);
    return r;
}
