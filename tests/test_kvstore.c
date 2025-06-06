#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../src/kv_store.h"

int main() {
    kv_init();
    assert(kv_set("key1", "value1") == 0);
    assert(strcmp(kv_get("key1"), "value1") == 0);
    assert(kv_delete("key1") == 0);
    assert(kv_get("key1") == NULL);
    assert(kv_delete("key1") == -1);
    /// fill to the maximum number of key-value pairs
    for (int i = 0; i < MAX_KV_PAIRS; i++) {
        char key[32], value[128];
        snprintf(key, sizeof(key), "key%d", i);
        snprintf(value, sizeof(value), "value%d", i);
        assert(kv_set(key, value) == 0);
    }
    /// this new add should fail
    assert(kv_set("key_overflow", "value_overflow") == -1);

    printf("✅ Simple kvstore tests passed\n");
    return 0;
}