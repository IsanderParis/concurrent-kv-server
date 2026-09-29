#include "hashtable.h"

#include <stdlib.h> 
#include <string.h>

typedef struct ht_entry {
    char *key;
    void *value;
    struct ht_entry *next;
} ht_entry_t;

struct hashtable {
    ht_entry_t **buckets;
    size_t num_buckets;
    size_t size;
};

static unsigned long djb2(const char *str) {
    unsigned long hash = 5381;
    unsigned char c;

    while ((c = (unsigned char)*str++) != 0) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

static size_t bucket_index(const hashtable_t *ht, const char *key) {
    return djb2(key) % ht->num_buckets;
}

hashtable_t *ht_create(size_t num_buckets) {
    if(num_buckets == 0) {
        return NULL;
    }

    hashtable_t *ht = malloc(sizeof(*ht));
    if(!ht) {
        return NULL;
    }

    ht->buckets = calloc(num_buckets, sizeof *ht->buckets);
    if(!ht->buckets) {
        free(ht);
        return NULL;
    }

    ht->num_buckets = num_buckets;
    ht->size = 0;

    return ht;
}

void ht_destroy(hashtable_t *ht) {
    if(!ht) {
        return;
    }

    for(size_t i = 0; i < ht->num_buckets; ++i) {
        ht_entry_t *e = ht->buckets[i];
        while(e) {
            ht_entry_t *next = e->next;
            free(e->key);
            free(e->value);
            free(e);
            e = next;
        }
    }

    free(ht->buckets);
    free(ht);
}

char *ht_get(hashtable_t *ht, const char *key) {
    if (!ht || !key) {
        return NULL;
    }

    size_t index = bucket_index(ht, key);
    ht_entry_t *e = ht->buckets[index];        

    while (e) {
        if (strcmp(e->key, key) == 0) {
            return strdup(e->value);        
        }
        e = e->next;
    }

    return NULL;
}                                          

size_t ht_size(const hashtable_t *ht) {
    if (!ht) {
        return 0;
    }
    return ht->size;
}

ht_status_t ht_set(hashtable_t *ht, const char *key, const char *value) {  
    if (!ht || !key || !value) {
        return HT_ERR;
    }

    size_t index = bucket_index(ht, key);
    ht_entry_t *e = ht->buckets[index];

    
    while (e) {
        if (strcmp(e->key, key) == 0) {
            char *new_value = strdup(value);   
            if (!new_value) {
                return HT_ERR;                 
            }
            free(e->value);                   
            e->value = new_value;              
            return HT_OK;
        }
        e = e->next;
    }

    ht_entry_t *new_entry = malloc(sizeof(*new_entry));
    if (!new_entry) {
        return HT_ERR;
    }

    new_entry->key = strdup(key);
    if (!new_entry->key) {
        free(new_entry);
        return HT_ERR;
    }

    new_entry->value = strdup(value);          
    if (!new_entry->value) {
        free(new_entry->key);                  
        free(new_entry);
        return HT_ERR;
    }

    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;
    ht->size++;

    return HT_OK;
}

ht_status_t ht_del(hashtable_t *ht, const char *key) {
    if (!ht || !key) {
        return HT_ERR;
    }

    size_t index = bucket_index(ht, key);
    ht_entry_t *e = ht->buckets[index];
    ht_entry_t *prev = NULL;

    while (e) {
        if (strcmp(e->key, key) == 0) {
            if (prev) {
                prev->next = e->next;
            } else {
                ht->buckets[index] = e->next;
            }
            free(e->key);
            free(e->value);
            free(e);
            ht->size--;
            return HT_OK;
        }
        prev = e;
        e = e->next;
    }

    return HT_NOT_FOUND;
}