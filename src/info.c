#include <string.h>
#include <time.h>
#include <sys/resource.h>

#include "info.h"
#include "kvstore.h"

#ifndef VERSION
#define VERSION "dev"
#endif

server_info_t fill_data(int mem, int keys, long uptime, const char *version) {
    server_info_t r;
    r.mem = mem;
    r.keys = keys;
    r.uptime = uptime;
    strncpy(r.version, version, sizeof(r.version) - 1);
    r.version[sizeof(r.version) - 1] = '\0';
    return r;
}

server_info_t get_info(time_t start_time) {
    time_t now = time(NULL);
    long uptime = now - start_time;

    struct rusage usage;
    getrusage(RUSAGE_SELF, &usage);

    int mem_mb = 0;
#if defined(__APPLE__)
    mem_mb = (int)(usage.ru_maxrss / 1024 / 1024);
#else
    mem_mb = (int)(usage.ru_maxrss / 1024);
#endif

    int keys = kv_count_keys();
    return fill_data(mem_mb, keys, uptime, VERSION);
}