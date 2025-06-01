.Default: all

CC=gcc
SERVER_LIB=-lpthread

all: server client
server: tcp_server.c
	$(CC) -o bin/tcp_server tcp_server.c $(SERVER_LIB)
client: tcp_client.c
	$(CC) -o bin/tcp_client tcp_client.c
clean:
	rm -f bin/tcp_server bin/tcp_client

.PHONY: all clean server client


