#ifndef HASHTABLE_H
#define HASHTABLE_H


#include <stddef.h>

typedef struct hashtable hashtable_t;


typedef enum {
    HT_OK = 0,
    HT_NOT_FOUND = 1,
    HT_ERR = -1
} ht_status_t;

hashtable_t *ht_create(size_t num_buckets);
void ht_destroy(hashtable_t *ht);
ht_status_t ht_set(hashtable_t *ht, const char *key, void *value);
char *ht_get(hashtable_t *ht, const char *key);
ht_status_t ht_del(hashtable_t *ht, const char *key);
size_t ht_size(hashtable_t *ht);


#endif // HASHTABLE_H