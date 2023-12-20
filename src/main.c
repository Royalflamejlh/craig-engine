#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include "util.h"
#include "mempool.h"
#include "tree.h"
#include "tests/bbtests.h"
#include "bitboard/bitboard.h"

#define NUM_SEARCH_THREADS 5

void *io_thread_entry(void *arg) {
    printf("IO Thread running...\n");
    return NULL;
}

void *search_thread_entry(void *arg) {
    printf("Search Thread running...\n");
    return NULL;
}

int main(void) {
    pthread_t io_thread;
    pthread_t search_threads[NUM_SEARCH_THREADS];
    int i;

    // generateMasks();
    // generateMagics();
    // initZobrist();
    // initPST();
    // if(initTT()){
    //     printf("WARNING FAILED TO ALLOCATED SPACE FOR TRANSPOSITION TABLE\n");
    //     return -1;
    // }

    // Create IO thread
    if (pthread_create(&io_thread, NULL, io_thread_entry, NULL)) {
        fprintf(stderr, "Error creating IO thread\n");
        return 1;
    }

    // Create search threads
    for (i = 0; i < NUM_SEARCH_THREADS; i++) {
        if (pthread_create(&search_threads[i], NULL, search_thread_entry, NULL)) {
            fprintf(stderr, "Error creating search thread %d\n", i);
            return 1;
        }
    }

    pthread_join(io_thread, NULL);
    for (i = 0; i < NUM_SEARCH_THREADS; i++) {
        pthread_join(search_threads[i], NULL);
    }

    printf("All threads have finished.\n");
    return 0;
}

 

