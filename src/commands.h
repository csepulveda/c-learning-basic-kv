#ifndef COMMANDS_H
#define COMMANDS_H

void cmd_ping(int clientfd);
void cmd_time(int clientfd);
void cmd_goodbye(int clientfd);
void cmd_set(int clientfd, const char *buffer);
void cmd_get(int clientfd, const char *buffer);
void cmd_del(int clientfd, const char *buffer);

#endif
