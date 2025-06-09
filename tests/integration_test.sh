#!/bin/bash
set -euo pipefail

# …rest of tests/integration_test.sh…
SERVER_BIN=bin/server
CLIENT_BIN=bin/client

echo "🚀 Starting integration tests..."

# Start server in background
$SERVER_BIN &
SERVER_PID=$!
sleep 1  # espera a que el servidor arranque

fail() {
  echo "❌ Test failed: $1"
  kill $SERVER_PID
  wait $SERVER_PID
  exit 1
}

# Wrapper para simplificar asserts
assert_contains() {
    echo "$1" | grep -q "$2" || fail "$3"
}

# Test SET
output=$($CLIENT_BIN "SET test 123")
assert_contains "$output" "OK" "SET did not return OK"

# Test GET
output=$($CLIENT_BIN "GET test")
assert_contains "$output" "123" "GET did not return 123"

# Test DEL
output=$($CLIENT_BIN "DEL test")
assert_contains "$output" "DELETED" "DEL did not return DELETED"

# Test GET after DEL
output=$($CLIENT_BIN "GET test")
assert_contains "$output" "not found" "GET after DEL did not return not found"

# Test TIME
output=$($CLIENT_BIN "TIME")
assert_contains "$output" "20" "TIME did not return expected format"

# Test PING
output=$($CLIENT_BIN "PING")
assert_contains "$output" "PONG" "PING did not return PONG"

# Test Very Long Command
MAX_VAL_LEN=$(grep '^#define MAX_VAL_LEN' src/protocol.h | awk '{print $3}')
LONG_CMD=$(head -c $((MAX_VAL_LEN + 1)) < /dev/zero | tr '\0' 'A')
output=$($CLIENT_BIN "SET long $LONG_CMD")
assert_contains "$output" "ERROR value too long" "SET long command did not return ERROR value too long"

# Test error on invalid command
output=$($CLIENT_BIN "INVALID COMMAND" 2>&1)
assert_contains "$output" "Invalid command: INVALID" "Invalid command did not return ERROR"

# test defining host and port
output=$(HOST=127.0.0.1 PORT=8080 $CLIENT_BIN "SET test 123")
assert_contains "$output" "OK" "SET did not return OK"

# Test interactive mode
$CLIENT_BIN <<EOF | grep -q "OK" || fail "Interactive SET failed"
SET interactive 123
GET interactive
DEL interactive
TIME
PING
exit
EOF

# test bad command definition
output=$($CLIENT_BIN "SET test" 2>&1)
assert_contains "$output" "ERROR" "Bad command did not return ERROR"

# test invalid command
LONG_CMD=$(head -c 1000 < /dev/zero | tr '\0' 'A')
output=$($CLIENT_BIN "$LONG_CMD" 2>&1)
assert_contains "$output" "Invalid command" "Very long command did not return Invalid command"

# Determine buffer size from the header
BUFFER_SIZE=$(grep '^#define BUFFER_SIZE' src/client_utils.h | awk '{print $3}')

# Construct command string failure
LONG_CMD=$(head -c $((BUFFER_SIZE + 1)) < /dev/zero | tr '\0' 'A')
output=$($CLIENT_BIN SET $LONG_CMD $LONG_CMD $LONG_CMD 2>&1 || true)
assert_contains "$output" "Failed to construct command" "Expected build_command_string failure not triggered"

# Test Very Long Command with SET
LONG_CMD=$(head -c $((BUFFER_SIZE + 1)) < /dev/zero | tr '\0' 'A')
output=$($CLIENT_BIN SET foo $LONG_CMD 2>&1 || true)
assert_contains "$output" "Failed to construct command" "Expected build_command_string failure not triggered"

# Test MSET
output=$($CLIENT_BIN "MSET k1 v1 k2 v2 k3 v3")
assert_contains "$output" "OK" "MSET did not return OK"

# Test MGET (existing keys)
output=$($CLIENT_BIN "MGET k1 k2 k3")
assert_contains "$output" "1) v1" "MGET k1 failed"
assert_contains "$output" "2) v2" "MGET k2 failed"
assert_contains "$output" "3) v3" "MGET k3 failed"

# Test MGET with missing key
output=$($CLIENT_BIN "MGET k1 missing k3")
assert_contains "$output" "1) v1" "MGET k1 failed"
assert_contains "$output" "2) (nil)" "MGET missing key failed"
assert_contains "$output" "3) v3" "MGET k3 failed"

# Test SET with quoted value
$CLIENT_BIN 'SET quoted "hello world with spaces"' | grep -q "OK" || fail "SET quoted value did not return OK"

# Test GET quoted value
$CLIENT_BIN "GET quoted" | grep -q "hello world with spaces" || fail "GET quoted value did not return correct value"

# Test MSET with quoted values
$CLIENT_BIN 'MSET q1 "val 1" q2 "val 2" q3 "val 3"' | grep -q "OK" || fail "MSET quoted values did not return OK"

# Test concurrent clients
echo "Testing concurrent clients..."
seq 1 10 | parallel -j10 --bar "$CLIENT_BIN \"SET concurrent{} value{}\""

# Validate results
seq 1 10 | parallel -j10 --bar "$CLIENT_BIN \"GET concurrent{}\" | grep -q \"value{}\" || fail \"Concurrent GET concurrent{} failed\""

### EXTRA: TEST raw server with NC
echo "Testing raw server with nc..."


# Raw test: SET via nc
echo -ne "SET nc_test 42\n" | nc -w1 127.0.0.1 8080 > nc_out.txt
assert_contains "$(cat nc_out.txt)" "RESPONSE OK STRING" "NC SET failed"
assert_contains "$(cat nc_out.txt)" "OK" "NC SET failed (OK line)"

# Raw test: GET via nc
echo -ne "GET nc_test\n" | nc -w1 127.0.0.1 8080 > nc_out.txt
assert_contains "$(cat nc_out.txt)" "RESPONSE OK STRING" "NC GET failed"
assert_contains "$(cat nc_out.txt)" "42" "NC GET failed (value)"

# Raw test: MGET via nc
echo -ne "MGET nc_test unknown\n" | nc -w1 127.0.0.1 8080 > nc_out.txt
assert_contains "$(cat nc_out.txt)" "1) 42" "NC MGET failed (existing key)"
assert_contains "$(cat nc_out.txt)" "2) (nil)" "NC MGET failed (missing key)"

# Test MGET quoted values
output=$($CLIENT_BIN "MGET q1 q2 q3")
echo "$output" | grep -q "1) val 1" || fail "MGET did not return val 1"
echo "$output" | grep -q "2) val 2" || fail "MGET did not return val 2"
echo "$output" | grep -q "3) val 3" || fail "MGET did not return val 3"

# Shutdown server
kill $SERVER_PID
wait $SERVER_PID

# Delete files created during tests
rm nc_out.txt

# Validate server is down
$CLIENT_BIN "PING" 2>&1 | grep -q "Connection refused" || fail "Server did not shut down properly"

echo "✅ All integration tests passed!"