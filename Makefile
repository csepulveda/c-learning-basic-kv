.DEFAULT_GOAL := all

CC       := gcc
CFLAGS   := -Wall -Wextra -O2
LDFLAGS  := -lpthread

SRC_DIR     := src
BIN_DIR     := bin
TEST_DIR    := tests

SERVER_SRC   := $(SRC_DIR)/server.c
CLIENT_SRC   := $(SRC_DIR)/client.c
KVSTORE_SRC  := $(SRC_DIR)/kvstore.c
PROTOCOL_SRC := $(SRC_DIR)/protocol.c
COMMANDS_SRC := $(SRC_DIR)/commands.c
LOGS_SRC     := $(SRC_DIR)/logs.c

SERVER_BIN := $(BIN_DIR)/server
CLIENT_BIN := $(BIN_DIR)/client

TEST_KV_SRC       := $(TEST_DIR)/test_kvstore.c
TEST_PROTOCOL_SRC := $(TEST_DIR)/test_protocol.c

TEST_KV_BIN       := $(BIN_DIR)/test_kvstore
TEST_PROTOCOL_BIN := $(BIN_DIR)/test_protocol

all: $(SERVER_BIN) $(CLIENT_BIN) test integration-test

$(BIN_DIR):
	mkdir -p $@

$(SERVER_BIN): $(SERVER_SRC) $(KVSTORE_SRC) $(PROTOCOL_SRC) $(COMMANDS_SRC) $(LOGS_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_SRC) $(KVSTORE_SRC) $(PROTOCOL_SRC) $(COMMANDS_SRC) $(LOGS_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_KV_BIN): $(TEST_KV_SRC) $(KVSTORE_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_PROTOCOL_BIN): $(TEST_PROTOCOL_SRC) $(PROTOCOL_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

test: $(TEST_KV_BIN) $(TEST_PROTOCOL_BIN)
	@echo "Running kvstore tests..."
	@$(TEST_KV_BIN)
	@echo "Running protocol tests..."
	@$(TEST_PROTOCOL_BIN)

integration-test:
	@echo "Running integration tests..."
	@tests/integration_test.sh

clean:
	rm -rf $(BIN_DIR) *.gcda *.gcno *.info *.gcov coverage.info

# Recompilación para cobertura
coverage-build:
	$(MAKE) clean
	$(MAKE) all CFLAGS="-Wall -Wextra -O0 -g -fprofile-arcs -ftest-coverage" LDFLAGS="--coverage"

.PHONY: all test integration-test clean coverage-build
