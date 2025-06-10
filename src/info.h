#ifndef INFO_H
#define INFO_H

#include <time.h>

extern time_t start_time;

typedef struct {
    int  mem;
    int  keys;
    long uptime;
    char version[50];
} server_info_t;

server_info_t get_info(time_t start_time);

#endif
