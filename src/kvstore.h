#ifndef kvstore_H
#define kvstore_H

#define HASH_TABLE_SIZE 256 
#define MAX_KEY_LEN 32
#define MAX_VAL_LEN 128
#include <stdbool.h>

typedef enum {
    KV_STRING,
    KV_HASH
} kv_type_t;

typedef struct kv_field_node {
    char field[MAX_KEY_LEN];
    char value[MAX_VAL_LEN];
    struct kv_field_node *next;
} kv_field_node;

typedef struct kv_node {
    char key[MAX_KEY_LEN];
    kv_type_t type;
    union {
        char value[MAX_VAL_LEN];
        kv_field_node *hash_fields;
    };
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
int kv_count_keys(void);

int kv_hset(const char *key, const char *field, const char *value);
const char* kv_hget(const char *key, const char *field);
double kv_hincrby(const char *key, const char *field, double increment);
kv_type_t kv_get_type(const char *key);
bool kv_is_hash(const char *key);

#endif