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

    printf("✅ Simple kvstore tests passed\n");
    return 0;
}