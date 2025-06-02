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

echo "✅ All integration tests passed!"
kill $SERVER_PID