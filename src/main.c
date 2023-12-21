#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#define NUM_SEARCH_THREADS 5

#if defined(__unix__) || defined(__APPLE__)

#include <pthread.h>

void *io_thread_entry(void *arg) {
    (void)arg;
    printf("IO Thread running...\n");
    //Call start io function or something
    return NULL;
}

void *search_thread_entry(void *arg) {
    (void)arg;
    printf("Search Thread running...\n");
    //Call start search function or something
    return NULL;
}

static int launch_threads(void){
    pthread_t io_thread;
    pthread_t search_threads[NUM_SEARCH_THREADS];
    int i;
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
    return 0;
}

#elif defined(_WIN32) || defined(_WIN64)

#include <windows.h>

static DWORD WINAPI io_thread_entry(LPVOID lpParam) {
    (void)lpParam;
    printf("IO Thread running...\n");
    // Implementation of your IO thread entry function
    return 0;
}

static DWORD WINAPI search_thread_entry(LPVOID lpParam) {
    (void)lpParam;
    printf("Search Thread running...\n");
    //Call start search function or something
    return 0;
}

static int launch_threads(void){
    HANDLE io_thread;
    HANDLE search_threads[NUM_SEARCH_THREADS];
    int i;

    // Create IO thread
    io_thread = CreateThread(NULL, 0, io_thread_entry, NULL, 0, NULL);
    if (io_thread == NULL) {
        fprintf(stderr, "Error creating IO thread\n");
        return 1;
    }

    // Create search threads
    for (i = 0; i < NUM_SEARCH_THREADS; i++) {
        search_threads[i] = CreateThread(NULL, 0, search_thread_entry, NULL, 0, NULL);
        if (search_threads[i] == NULL) {
            fprintf(stderr, "Error creating search thread %d\n", i);
            return 1;
        }
    }

    // Wait for IO thread to terminate
    WaitForSingleObject(io_thread, INFINITE);
    CloseHandle(io_thread);

    // Wait for search threads to terminate
    for (i = 0; i < NUM_SEARCH_THREADS; i++) {
        WaitForSingleObject(search_threads[i], INFINITE);
        CloseHandle(search_threads[i]);
    }

    return 0;
}

#endif

int main(void) {
    // generateMasks();
    // generateMagics();
    // initZobrist();
    // initPST();
    // if(initTT()){
    //     printf("WARNING FAILED TO ALLOCATED SPACE FOR TRANSPOSITION TABLE\n");
    //     return -1;
    // }

    // Create IO thread
    launch_threads();
    

    printf("All threads have finished.\n");
    return 0;
}


 

