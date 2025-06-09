#ifndef KV_STORE_H
#define KV_STORE_H

#define HASH_TABLE_SIZE 256 
#define MAX_KEY_LEN 32
#define MAX_KV_PAIRS 100
#define MAX_VAL_LEN 128

typedef struct kv_node {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
    struct kv_node* next;
} kv_node;

typedef struct {
    char key[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
} kv_pair;

void kv_init();
int kv_set(const char *key, const char *value);
const char* kv_get(const char *key);
int kv_delete(const char *key);

#endif
