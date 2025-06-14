#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../src/kvstore.h"

int main() {
    kv_init();

    // Test basic set/get/delete
    assert(kv_set("key1", "value1") == 0);
    assert(strcmp(kv_get("key1"), "value1") == 0);
    assert(kv_delete("key1") == 0);
    assert(kv_get("key1") == NULL);
    assert(kv_delete("key1") == -1);

    // Test inserting multiple keys
    const int num_keys = 2000; // Large number to test hash collisions + chaining
    for (int i = 0; i < num_keys; i++) {
        char key[MAX_KEY_LEN], value[MAX_VAL_LEN];
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, sizeof(value), "value%d", i);
        assert(kv_set(key, value) == 0);
    }

    // Test retrieving the inserted keys
    for (int i = 0; i < num_keys; i++) {
        char key[MAX_KEY_LEN], expected_value[MAX_VAL_LEN];
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(expected_value, sizeof(expected_value), "value%d", i);
        const char* actual_value = kv_get(key);
        assert(actual_value != NULL);
        assert(strcmp(actual_value, expected_value) == 0);
    }

    // Test deleting a subset of keys
    for (int i = 0; i < num_keys; i += 2) { // Delete every second key
        char key[MAX_KEY_LEN];
        snprintf(key, sizeof(key), "key%d", i);
        assert(kv_delete(key) == 0);
        assert(kv_get(key) == NULL);
    }

    // Ensure the other half still exists
    for (int i = 1; i < num_keys; i += 2) {
        char key[MAX_KEY_LEN], expected_value[MAX_VAL_LEN];
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(expected_value, sizeof(expected_value), "value%d", i);
        const char* actual_value = kv_get(key);
        assert(actual_value != NULL);
        assert(strcmp(actual_value, expected_value) == 0);
    }

    assert(kv_hset("myhash", "field1", "val1") == 0);
    assert(strcmp(kv_hget("myhash", "field1"), "val1") == 0);
    assert(kv_hincrby("myhash", "counter", 5) == 5);
    assert(kv_hincrby("myhash", "counter", 2) == 7);
    assert(kv_is_hash("myhash") == true);
    assert(kv_get("myhash") == NULL); // type safety

    printf("✅ Hash table kvstore tests passed\n");
    return 0;
}
