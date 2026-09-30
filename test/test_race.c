#include "../src/hashtable.h"
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 4
#define KEYS_PER_THREAD 10000

static hashtable_t *global_ht = NULL;

void *worker (void *args){
    int id = *((int *)args);
    char key[32];

    for(int i = 0; i< KEYS_PER_THREAD; i++){
        snprintf(key, sizeof(key), "key_%d_%d", id, i);
        ht_set(global_ht, key, "x");
        
    }
    return NULL;
}

int main(void){
    global_ht = ht_create(10);
    if (!global_ht) {
        fprintf(stderr, "Failed to create hashtable\n");
        return 1;
    }
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    for(int i = 0; i < NUM_THREADS; i++){
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, worker, &thread_ids[i]);
    }

    for(int i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }
    printf("expected size: %d, actual size: %zu\n", NUM_THREADS * KEYS_PER_THREAD, ht_size(global_ht));



    ht_destroy(global_ht);
    return 0;


}

