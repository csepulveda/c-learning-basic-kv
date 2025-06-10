.DEFAULT_GOAL := all

GIT_VERSION := $(shell git describe --tags --always)
CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -DVERSION=\"$(GIT_VERSION)\"
CFLAGS_TEST := -Wall -Wextra -O0 -g -fprofile-arcs -ftest-coverage
LDFLAGS  := -lpthread
LDFLAGS_TEST := --coverage

SRC_DIR     := src
BIN_DIR     := bin
TEST_DIR    := tests

SERVER_SRC   := $(SRC_DIR)/server.c
CLIENT_SRC   := $(SRC_DIR)/client.c
KVSTORE_SRC  := $(SRC_DIR)/kvstore.c
PROTOCOL_SRC := $(SRC_DIR)/protocol.c
COMMANDS_SRC := $(SRC_DIR)/commands.c
LOGS_SRC     := $(SRC_DIR)/logs.c
INFO_SRC     := $(SRC_DIR)/info.c
CLIENT_UTILS_SRC := $(SRC_DIR)/client_utils.c
SERVER_UTILS_SRC := $(SRC_DIR)/server_utils.c

SERVER_BIN := $(BIN_DIR)/server
CLIENT_BIN := $(BIN_DIR)/client

TEST_KV_SRC       := $(TEST_DIR)/test_kvstore.c
TEST_PROTOCOL_SRC := $(TEST_DIR)/test_protocol.c
TEST_LOGS_SRC := $(TEST_DIR)/test_logs.c
TEST_CLIENT_SRC := $(TEST_DIR)/test_client.c
TEST_SERVER_SRC := $(TEST_DIR)/test_server.c
TEST_COMMANDS_SRC := $(TEST_DIR)/test_commands.c

TEST_KV_BIN       := $(BIN_DIR)/test_kvstore
TEST_PROTOCOL_BIN := $(BIN_DIR)/test_protocol
TEST_LOGS_BIN := $(BIN_DIR)/test_logs
TEST_CLIENT_BIN := $(BIN_DIR)/test_client
TEST_SERVER_BIN := $(BIN_DIR)/test_server
TEST_COMMANDS_BIN := $(BIN_DIR)/test_commands

all: $(SERVER_BIN) $(CLIENT_BIN)

$(BIN_DIR):
	mkdir -p $@

$(SERVER_BIN): $(SERVER_SRC) $(SERVER_UTILS_SRC) $(KVSTORE_SRC) $(PROTOCOL_SRC) $(COMMANDS_SRC) $(LOGS_SRC) $(INFO_SRC)| $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_SRC) $(KVSTORE_SRC) $(PROTOCOL_SRC) $(LOGS_SRC) $(CLIENT_UTILS_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(TEST_KV_BIN): $(TEST_KV_SRC) $(KVSTORE_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_TEST) $(LDFLAGS_TEST) -o $@ $^

$(TEST_PROTOCOL_BIN): $(TEST_PROTOCOL_SRC) $(PROTOCOL_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_TEST) $(LDFLAGS_TEST) -o $@ $^

$(TEST_COMMANDS_BIN): $(TEST_COMMANDS_SRC) $(COMMANDS_SRC) $(PROTOCOL_SRC) $(KVSTORE_SRC) $(INFO_SRC)| $(BIN_DIR)
	$(CC) $(CFLAGS_TEST) $(LDFLAGS_TEST) -o $@ $^

$(TEST_LOGS_BIN): $(TEST_LOGS_SRC) $(LOGS_SRC) | $(BIN_DIR)
	$(CC) $(CFLAGS_TEST) $(LDFLAGS_TEST) -o $@ $^

$(TEST_CLIENT_BIN): $(TEST_CLIENT_SRC) $(CLIENT_UTILS_SRC) $(LOGS_SRC) $(PROTOCOL_SRC)| $(BIN_DIR)
	$(CC) $(CFLAGS_TEST) $(LDFLAGS_TEST) -o $@ $^

$(TEST_SERVER_BIN): $(TEST_SERVER_SRC) $(PROTOCOL_SRC) $(SERVER_UTILS_SRC) $(KVSTORE_SRC) $(COMMANDS_SRC) $(LOGS_SRC) $(INFO_SRC)| $(BIN_DIR)
	$(CC) $(CFLAGS_TEST) $(LDFLAGS_TEST) -o $@ $^

test: $(TEST_KV_BIN) $(TEST_PROTOCOL_BIN) $(TEST_LOGS_BIN) $(TEST_CLIENT_BIN) $(TEST_SERVER_BIN) $(TEST_COMMANDS_BIN)
	@echo "Running kvstore tests..."
	@$(TEST_KV_BIN)
	@echo "Running protocol tests..."
	@$(TEST_PROTOCOL_BIN)
	@echo "Running logs tests..."
	@$(TEST_LOGS_BIN)
	@echo "Running client tests..."
	@$(TEST_CLIENT_BIN)
	@echo "Running server tests..."
	@$(TEST_SERVER_BIN)
	@echo "Running commands tests..."
	@$(TEST_COMMANDS_BIN)

integration-test:
	@echo "Running integration tests..."
	@tests/integration_test.sh

clean:
	rm -rf $(BIN_DIR) *.gcda *.gcno *.info *.gcov coverage.xml

coverage-build:
	$(MAKE) clean
	$(MAKE) all CFLAGS="$(CFLAGS_TEST)" LDFLAGS="$(LDFLAGS_TEST)"

local-coverage: coverage-build
	@echo "Running tests with coverage..."
	@$(MAKE) test
	@$(MAKE) integration-test
	@echo "Generating coverage report..."
	@gcovr $(BIN_DIR)/*.gcda
	@$(MAKE) clean

.PHONY: all test integration-test clean coverage-build
