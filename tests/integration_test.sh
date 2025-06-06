#!/bin/sh

SERVER_BIN=bin/server
CLIENT_BIN=bin/client

# Start server in background
$SERVER_BIN &
SERVER_PID=$!
sleep 1  # espera a que el servidor arranque

fail() {
  echo "❌ Test failed: $1"
  kill $SERVER_PID
  exit 1
}

# Test SET
$CLIENT_BIN "SET test=123"|grep -q "OK" || fail "SET did not return OK"

# Test GET
$CLIENT_BIN "GET test" | grep -q "123" || fail "GET did not return 123"

# Test DEL
$CLIENT_BIN "DEL test" | grep -q "DELETED" || fail "DEL did not return DELETED"

# Test GET after DEL
$CLIENT_BIN "GET test" | grep -q  "NOT FOUND" || fail "GET after DEL did not return NOT FOUND"

# Test TIME
$CLIENT_BIN "TIME" | grep -q "20" || fail "TIME did not return expected format"

# Test PING
$CLIENT_BIN "PING" | grep -q "PONG" || fail "PING did not return PONG"

# Test Very Long Command
LONG_CMD=$(head -c 1000 < /dev/zero | tr '\0' 'A')
output=$($CLIENT_BIN "SET long=$LONG_CMD")
echo "$output" | grep -q "ERROR" || fail "SET long command did not return ERROR"

# Test error on invalid command
$CLIENT_BIN "INVALID COMMAND" 2>&1 | grep -q "ERROR" || fail "Invalid command did not return ERROR"

# test defining host and port
HOST=127.0.0.1 PORT=8080 $CLIENT_BIN "SET test=123"|grep -q "OK" || fail "SET did not return OK"

# Test interactive mode
$CLIENT_BIN <<EOF | grep -q "OK"
SET interactive=123
GET interactive
DEL interactive
TIME
PING
exit
EOF

# test bad command definition
$CLIENT_BIN "SET test" 2>&1 | grep -q "ERROR" || fail "Bad command did not return ERROR"

# test invalid command
$CLIENT_BIN $(for i in {1..500}; do echo -n "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA "; done) 2>&1 | grep -q "Invalid command" || fail "Invalid command did not return ERROR"


kill $SERVER_PID

# Test Very Long Command with GET
$CLIENT_BIN "PING"  2>&1  | grep -q "Connection refused" || fail "Server did not shut down properly"

echo "✅ All integration tests passed!"