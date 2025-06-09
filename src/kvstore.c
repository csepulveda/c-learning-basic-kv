#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "kvstore.h"

static kv_node* hash_table[HASH_TABLE_SIZE];

static unsigned int hash(const char* key) {
    unsigned int hash = 5381;
    int c;
    while ((c = (unsigned char)*key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

void kv_init() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        hash_table[i] = NULL;
    }
}

int kv_set(const char* key, const char* value) {
    unsigned int index = hash(key);
    kv_node* node = hash_table[index];

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            snprintf(node->value, MAX_VAL_LEN, "%s", value);
            return 0;
        }
        node = node->next;
    }

    kv_node* new_node = (kv_node*)malloc(sizeof(kv_node));
    if (!new_node) return -1;

    snprintf(new_node->key, MAX_KEY_LEN, "%s", key);
    snprintf(new_node->value, MAX_VAL_LEN, "%s", value);
    new_node->next = hash_table[index];
    hash_table[index] = new_node;

    return 0;
}

const char* kv_get(const char* key) {
    unsigned int index = hash(key);
    kv_node* node = hash_table[index];

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return node->value;
        }
        node = node->next;
    }
    return NULL;
}
int kv_delete(const char* key) {
    unsigned int index = hash(key);
    kv_node* node = hash_table[index];
    kv_node* prev = NULL;

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            if (prev) {
                prev->next = node->next;
            } else {
                hash_table[index] = node->next;
            }
            free(node);
            return 0;
        }
        prev = node;
        node = node->next;
    }

    return -1;
}
