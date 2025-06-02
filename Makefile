.Default: all

CC := gcc
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -lpthread

SRC_DIR := src
BIN_DIR := bin

SERVER_SRC := $(SRC_DIR)/server.c
CLIENT_SRC := $(SRC_DIR)/client.c
KV_SRC := $(SRC_DIR)/kvstore.c
PROTOCOL_SRC := $(SRC_DIR)/protocol.c
COMMANDS_SRC := $(SRC_DIR)/commands.c

SERVER_BIN := $(BIN_DIR)/server
CLIENT_BIN := $(BIN_DIR)/client

all: $(SERVER_BIN) $(CLIENT_BIN)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(SERVER_BIN): $(SERVER_SRC) $(KV_SRC) $(PROTOCOL_SRC) $(COMMANDS_SRC)| $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_SRC) $(KV_SRC) $(PROTOCOL_SRC) $(COMMANDS_SRC)| $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean


