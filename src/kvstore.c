#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "kvstore.h"

static kv_node* hash_table[HASH_TABLE_SIZE];

int kv_count_keys(void) {
    int count = 0;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        kv_node* node = hash_table[i];
        while (node != NULL) {
            count++;
            node = node->next;
        }
    }
    return count;
}

static unsigned int hash(const char* key) {
    unsigned int hash = 5381;
    int c;
    while ((c = (unsigned char)*key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_TABLE_SIZE;
}

static void free_hash_fields(kv_field_node *field) {
    while (field) {
        kv_field_node *next_field = field->next;
        free(field);
        field = next_field;
    }
}

static kv_field_node* find_field_node(kv_field_node *field_node, const char *field) {
    while (field_node) {
        if (strcmp(field_node->field, field) == 0) {
            return field_node;
        }
        field_node = field_node->next;
    }
    return NULL;
}

void kv_init() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        kv_node* node = hash_table[i];

        while (node) {
            kv_node* next = node->next;

            if (node->type == KV_HASH) {
                free_hash_fields(node->hash_fields);
            }

            free(node);
            node = next;
        }

        hash_table[i] = NULL;
    }
}

static kv_node* find_node(const char* key) {
    unsigned int index = hash(key);
    kv_node* node = hash_table[index];

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            return node;
        }
        node = node->next;
    }
    return NULL;
}

bool kv_is_hash(const char *key) {
    const kv_node* node = find_node(key);
    if (!node) return false;
    return (node->type == KV_HASH);
}

int kv_set(const char* key, const char* value) {
    unsigned int index = hash(key);
    kv_node* node = hash_table[index];

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            // enforce type safety
            if (node->type != KV_STRING) return -1;

            snprintf(node->value, MAX_VAL_LEN, "%s", value);
            return 0;
        }
        node = node->next;
    }

    kv_node* new_node = (kv_node*)malloc(sizeof(kv_node));
    if (!new_node) return -1;

    snprintf(new_node->key, MAX_KEY_LEN, "%s", key);
    new_node->type = KV_STRING;
    snprintf(new_node->value, MAX_VAL_LEN, "%s", value);
    new_node->next = hash_table[index];
    hash_table[index] = new_node;

    return 0;
}

const char* kv_get(const char* key) {
    const kv_node* node = find_node(key);
    if (!node) return NULL;
    if (node->type != KV_STRING) return NULL; // enforce type safety
    return node->value;
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

            if (node->type == KV_HASH) {
                free_hash_fields(node->hash_fields);
            }

            free(node);
            return 0;
        }

        prev = node;
        node = node->next;
    }

    return -1;
}

int kv_get_type(const char *key) {
    const kv_node* node = find_node(key);
    if (!node) return -1; // not found
    return node->type;
}


int kv_hset(const char *key, const char *field, const char *value) {
    unsigned int index = hash(key);
    kv_node* node = hash_table[index];

    while (node != NULL) {
        if (strcmp(node->key, key) == 0) {
            if (node->type != KV_HASH) return -1;

            kv_field_node* field_node = find_field_node(node->hash_fields, field);
            if (field_node) {
                snprintf(field_node->value, MAX_VAL_LEN, "%s", value);
                return 0;
            }

            kv_field_node* new_field = (kv_field_node*)malloc(sizeof(kv_field_node));
            if (!new_field) return -1;

            snprintf(new_field->field, MAX_KEY_LEN, "%s", field);
            snprintf(new_field->value, MAX_VAL_LEN, "%s", value);
            new_field->next = node->hash_fields;
            node->hash_fields = new_field;
            return 0;
        }

        node = node->next;
    }

    // create new hash key
    kv_node* new_node = (kv_node*)malloc(sizeof(kv_node));
    if (!new_node) return -1;

    snprintf(new_node->key, MAX_KEY_LEN, "%s", key);
    new_node->type = KV_HASH;
    new_node->hash_fields = NULL;

    kv_field_node* new_field = (kv_field_node*)malloc(sizeof(kv_field_node));
    if (!new_field) {
        free(new_node);
        return -1;
    }

    snprintf(new_field->field, MAX_KEY_LEN, "%s", field);
    snprintf(new_field->value, MAX_VAL_LEN, "%s", value);
    new_field->next = NULL;

    new_node->hash_fields = new_field;
    new_node->next = hash_table[index];
    hash_table[index] = new_node;

    return 0;
}

const char* kv_hget(const char *key, const char *field) {
    kv_node* node = find_node(key);
    if (!node || node->type != KV_HASH) return NULL;

    kv_field_node* field_node = node->hash_fields;
    while (field_node != NULL) {
        if (strcmp(field_node->field, field) == 0) {
            return field_node->value;
        }
        field_node = field_node->next;
    }

    return NULL;
}

double kv_hincrby(const char *key, const char *field, double increment) {
    kv_node* node = find_node(key);
    if (node) {
        if (node->type != KV_HASH) return -1;

        kv_field_node* field_node = node->hash_fields;
        while (field_node != NULL) {
            if (strcmp(field_node->field, field) == 0) {
                double value = strtod(field_node->value, NULL);
                value += increment;
                snprintf(field_node->value, MAX_VAL_LEN, "%.17g", value);
                return value;
            }
            field_node = field_node->next;
        }

        // field does not exist → set it to increment
        double value = increment;
        kv_field_node* new_field = (kv_field_node*)malloc(sizeof(kv_field_node));
        if (!new_field) return -1;

        snprintf(new_field->field, MAX_KEY_LEN, "%s", field);
        snprintf(new_field->value, MAX_VAL_LEN, "%.17g", value);
        new_field->next = node->hash_fields;
        node->hash_fields = new_field;
        return value;
    }

    // key does not exist → create hash and field
    kv_node* new_node = (kv_node*)malloc(sizeof(kv_node));
    if (!new_node) return -1;

    snprintf(new_node->key, MAX_KEY_LEN, "%s", key);
    new_node->type = KV_HASH;
    new_node->hash_fields = NULL;

    kv_field_node* new_field = (kv_field_node*)malloc(sizeof(kv_field_node));
    if (!new_field) {
        free(new_node);
        return -1;
    }

    snprintf(new_field->field, MAX_KEY_LEN, "%s", field);
    snprintf(new_field->value, MAX_VAL_LEN, "%.17g", increment);
    new_field->next = NULL;

    new_node->hash_fields = new_field;
    unsigned int index = hash(key);
    new_node->next = hash_table[index];
    hash_table[index] = new_node;

    return increment;
}

