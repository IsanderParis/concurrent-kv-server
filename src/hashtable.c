#include "hashtable.h"

#include <stdlib.h> 
#include <string.h>
#include <pthread.h>

typedef struct ht_entry {
    char *key;
    void *value;
    struct ht_entry *next;
} ht_entry_t;

struct hashtable {
    ht_entry_t **buckets;
    size_t num_buckets;
    size_t size;
    pthread_mutex_t lock;
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

    if(pthread_mutex_init(&ht->lock, NULL) != 0) {
        free(ht->buckets);
        free(ht);
        return NULL;
    }

    ht->num_buckets = num_buckets;
    ht->size = 0;

    return ht;
}

/*
 * NO es thread-safe: quien llama debe garantizar que ningún otro thread
 * esté usando la tabla (por ejemplo, haciendo pthread_join antes).
 */
void ht_destroy(hashtable_t *ht) {
    if (!ht) {
        return;
    }

    for (size_t i = 0; i < ht->num_buckets; ++i) {
        ht_entry_t *e = ht->buckets[i];
        while (e) {
            ht_entry_t *next = e->next;
            free(e->key);
            free(e->value);
            free(e);
            e = next;
        }
    }

    pthread_mutex_destroy(&ht->lock);
    free(ht->buckets);
    free(ht);
}

char *ht_get(hashtable_t *ht, const char *key) {
    if (!ht || !key) {
        return NULL;
    }

    char *result = NULL;
    pthread_mutex_lock(&ht->lock);

    size_t index = bucket_index(ht, key);
    ht_entry_t *e = ht->buckets[index];        

    while (e) {
        if (strcmp(e->key, key) == 0) {
            result = strdup(e->value);
            break;
        }
        e = e->next;
    }

    pthread_mutex_unlock(&ht->lock);
    return result;
}                                          

size_t ht_size(hashtable_t *ht) {
    if (!ht) {
        return 0;
    }
    pthread_mutex_lock(&ht->lock);
    size_t size = ht->size;
    pthread_mutex_unlock(&ht->lock);
    return size;
}

ht_status_t ht_set(hashtable_t *ht, const char *key, const char *value) {  
    if (!ht || !key || !value) {
        return HT_ERR;
    }

    pthread_mutex_lock(&ht->lock);

    size_t index = bucket_index(ht, key);
    ht_entry_t *e = ht->buckets[index];

    
    while (e) {
        if (strcmp(e->key, key) == 0) {
            char *new_value = strdup(value);   
            if (!new_value) {
                pthread_mutex_unlock(&ht->lock);
                return HT_ERR;                 
            }
            free(e->value);                   
            e->value = new_value;  
            pthread_mutex_unlock(&ht->lock);            
            return HT_OK;
        }
        e = e->next;
    }

    ht_entry_t *new_entry = malloc(sizeof(*new_entry));
    if (!new_entry) {
        pthread_mutex_unlock(&ht->lock);
        return HT_ERR;
    }

    new_entry->key = strdup(key);
    if (!new_entry->key) {
        free(new_entry);
        pthread_mutex_unlock(&ht->lock);
        return HT_ERR;
    }

    new_entry->value = strdup(value);          
    if (!new_entry->value) {
        free(new_entry->key);                  
        free(new_entry);
        pthread_mutex_unlock(&ht->lock);
        return HT_ERR;
    }

    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;
    ht->size++;

    pthread_mutex_unlock(&ht->lock);

    return HT_OK;
}

ht_status_t ht_del(hashtable_t *ht, const char *key) {
    if (!ht || !key) {
        return HT_ERR;
    }

    pthread_mutex_lock(&ht->lock);

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
            pthread_mutex_unlock(&ht->lock);
            return HT_OK;
        }
        prev = e;
        e = e->next;
    }

    pthread_mutex_unlock(&ht->lock);
    return HT_NOT_FOUND;
}