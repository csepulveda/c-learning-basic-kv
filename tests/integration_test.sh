#!/bin/bash

SERVER_BIN=bin/server
CLIENT_BIN=bin/client

echo "🚀 Starting integration tests..."

$SERVER_BIN &
SERVER_PID=$!
sleep 1
restart_server() {
  echo "🔄 Restarting server..."
  kill $SERVER_PID
  wait $SERVER_PID
  sleep 1
  $SERVER_BIN &
  SERVER_PID=$!
  sleep 1
}

fail() {
  echo "❌ Test failed: $1"
  kill $SERVER_PID
  wait $SERVER_PID
  exit 1
}

assert_contains() {
    echo "$1" | tr -d '\r' | grep -Fq "$2" || fail "$3"
}

# -------- TEST CASES DEFINITION --------

VERY_LONG_KEY=$(head -c 300 < /dev/zero | tr '\0' 'A')
VERY_LONG_VALUE=$(head -c 1025 < /dev/zero | tr '\0' 'B')

test_cases=(
    'SET test 123 | OK | SET did not return OK'
    'GET test | 123 | GET did not return 123'
    'DEL test | DELETED | DEL did not return DELETED'
    'GET test | not found | GET after DEL did not return not found'
    'PING | PONG | PING did not return PONG'
    'TIME | 20 | TIME did not return expected format'
    'INFO | Uptime: | INFO did not return uptime'
    'INFO | Memory: | INFO did not return memory'
    'INFO | Keys: | INFO did not return keys'
    'INFO | Version: | INFO did not return version'
    'MSET k1 v1 k2 v2 k3 v3 | OK | MSET did not return OK'
    'MGET k1 k2 k3 | 1) v1 | MGET k1 failed'
    'MGET k1 k2 k3 | 2) v2 | MGET k2 failed'
    'MGET k1 k2 k3 | 3) v3 | MGET k3 failed'
    'MGET k1 missing k3 | 2) (nil) | MGET missing key failed'
    'MSET q1 \"val 1\" q2 \"val 2\" q3 \"val 3\" | OK | MSET quoted values did not return OK'
    'MGET q1 q2 q3 | 1) val 1 | MGET quoted val1 failed'
    'MGET q1 q2 q3 | 2) val 2 | MGET quoted val2 failed'
    'MGET q1 q2 q3 | 3) val 3 | MGET quoted val3 failed'
    'SET type_test abc | OK | SET type_test failed'
    'TYPE type_test | string | TYPE existing key did not return string'
    'TYPE missing_type_key | (nil) | TYPE missing key did not return (nil)'
    'HSET myhash field1 value1 | 1 | HSET single field did not return 1'
    'TYPE myhash | hash | TYPE for hash did not return hash'
    'HGET myhash field1 | value1 | HGET existing field1 failed'
    'HGET myhash missing_field | (nil) | HGET missing field did not return (nil)'
    'HSET myhash field2 value2 field3 value3 | 2 | HSET multiple fields did not return 2'
    'HGET myhash field2 | value2 | HGET field2 failed'
    'HGET myhash field3 | value3 | HGET field3 failed'
    'HMGET myhash field1 missing_field field3 | 1) value1 | HMGET field1 failed'
    'HMGET myhash field1 missing_field field3 | 2) (nil) | HMGET missing field failed'
    'HMGET myhash field1 missing_field field3 | 3) value3 | HMGET field3 failed'
    'HSET myhash f1 \"hello world\" f2 \"val with multiple spaces\" f3 \"third val\" | 3 | HSET multiple quoted fields failed'
    'HGET myhash f1 | hello world | HGET f1 after quoted HSET failed'
    'HGET myhash f2 | val with multiple spaces | HGET f2 after quoted HSET failed'
    'HGET myhash f3 | third val | HGET f3 after quoted HSET failed'
    'DEL myhash | DELETED | DEL myhash failed'
    'HINCRBY myhash counter 5 | 5 | HINCRBY new field did not return 5'
    'HINCRBY myhash counter 3 | 8 | HINCRBY existing field did not return 8'
    'HINCRBY newhash counter 5 | 5 | HINCRBY new hash did not return 5'
    'HINCRBY newhash counter 1000 | 1005 | HINCRBY existing field in newhash did not return 1005'
    'HINCRBY newhash counter -5 | 1000 | HINCRBY negative increment did not return 1000'
    'DEL newhash | DELETED | DEL newhash after HINCRBY did not return DELETED'
    'GET myhash | not found | GET on hash key did not return not found'
    'SET myhash fail | ERROR parse error | SET on hash key did not return parse error'
    "SET $VERY_LONG_KEY value | ERROR | SET with very long key did not return ERROR"
    "SET test $VERY_LONG_VALUE | ERROR | SET with very long value did not return ERROR"
    'HSET myhash fieldx valx | 1 | HSET for TYPE test failed'
    'TYPE myhash | hash | TYPE on hash did not return hash'
    'DEL nonexistent_key | not found | DEL nonexistent key did not return not found'
    'HINCRBY newneg counter -42 | 42 | HINCRBY new field negative did not return -42'
    'HINCRBY newneg counter 0 | 42 | HINCRBY increment 0 did not return same value'
    'SET sss abc | OK | SET for HMGET test failed'
#    'HMGET sss field1 | ERROR parse error | HMGET on string key did not return parse error'
    'BLAH foo bar | ERROR | Unknown command did not return error'
    'MGET missing1 missing2 missing3\n | 1) (nil) | MGET all missing key1 failed'
    'MGET missing1 missing2 missing3\n | 2) (nil) | MGET all missing key2 failed'
    'MGET missing1 missing2 missing3\n | 3) (nil) | MGET all missing key3 failed'
)

# -------- TEST RUNNERS --------

run_cmd_tests() {
    echo "🔷 Running CMD tests..."
    for entry in "${test_cases[@]}"; do
        IFS="|" read -r cmd expected desc <<< "$entry"
        cmd=$(echo "$cmd" | xargs)
        expected=$(echo "$expected" | xargs)
        desc=$(echo "$desc" | xargs)

        echo "👉 CMD: $cmd → expecting: '$expected'"

        read -ra tokens <<< "$cmd"
        output=$($CLIENT_BIN "${tokens[@]}" 2>&1)
        echo "Output: $output"
        assert_contains "$output" "$expected" "$desc (client cmd)"

    done
}

run_nc_tests() {
    echo "🔷 Running NC tests..."
    for entry in "${test_cases[@]}"; do
        IFS="|" read -r cmd expected desc <<< "$entry"
        cmd=$(echo "$cmd" | xargs)
        expected=$(echo "$expected" | xargs)
        desc=$(echo "$desc" | xargs)

        echo "👉 NC: $cmd → expecting: '$expected'"

        echo -ne "$cmd\n" | nc -q0 127.0.0.1 8080 > nc_out.txt
        output=$(cat nc_out.txt)
        assert_contains "$output" "$expected" "$desc (nc)"
    done
}

SKIP_INTERACTIVE_FOR=(
    "SET test $VERY_LONG_VALUE"
    "BLAH foo bar"
)

run_interactive_tests() {
    echo "🔷 Running INTERACTIVE tests..."
    for entry in "${test_cases[@]}"; do
        IFS="|" read -r cmd expected desc <<< "$entry"
        cmd=$(echo "$cmd" | xargs)
        expected=$(echo "$expected" | xargs)
        desc=$(echo "$desc" | xargs)

        skip=false
        for skip_cmd in "${SKIP_INTERACTIVE_FOR[@]}"; do
            if [[ "$cmd" == "$skip_cmd" ]]; then
                echo "⚠️  Skipping interactive for: $cmd"
                skip=true
                break
            fi
        done
        if $skip; then
            continue
        fi

        echo "👉 INTERACTIVE: $cmd → expecting: '$expected'"

        output=$($CLIENT_BIN <<EOF
$cmd
exit
EOF
)
        assert_contains "$output" "$expected" "$desc (interactive)"
    done
}

# -------- EXECUTE TESTS --------

run_cmd_tests
restart_server
run_nc_tests
restart_server
run_interactive_tests

# Done
kill $SERVER_PID
wait $SERVER_PID
rm -f nc_out.txt

echo "✅ All integration tests passed!"
