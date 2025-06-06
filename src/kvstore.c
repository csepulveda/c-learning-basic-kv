#include <stdio.h>
#include <string.h>

#include "kv_store.h"

static kv_pair store[MAX_KV_PAIRS];

void kv_init() {
    for (int i = 0; i < MAX_KV_PAIRS; i++) {
        store[i].key[0] = '\0';
    }
}

int kv_set(const char *key, const char *value) {
    for (int i = 0; i < MAX_KV_PAIRS; i++) {
        if (strcmp(store[i].key, key) == 0 || store[i].key[0] == '\0') {
            snprintf(store[i].key, MAX_KEY_LEN, "%s", key);
            snprintf(store[i].value, MAX_VAL_LEN, "%s", value);
            return 0;
        }
    }
    return -1;
}

const char* kv_get(const char *key) {
    for (int i = 0; i < MAX_KV_PAIRS; i++) {
        if (strcmp(store[i].key, key) == 0) {
            return store[i].value;
        }
    }
    return NULL;
}

int kv_delete(const char *key) {
    for (int i = 0; i < MAX_KV_PAIRS; i++) {
        if (strcmp(store[i].key, key) == 0) {
            store[i].key[0] = '\0';
            return 0;
        }
    }
    return -1;
}
