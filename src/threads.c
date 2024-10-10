
#include "threads.h"
#include "types.h"
#include "util.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "globals.h"
#include "search.h"
#include "io.h"

#include <pthread.h>
#include <unistd.h>
#include <errno.h>

static pthread_t search_threads[NUM_THREADS];

// Global variable to control the timer thread
pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t timer_cond = PTHREAD_COND_INITIALIZER;
static pthread_t timerThread;

/*
 * Quits out of a thread
 */
void quit_thread(){
   pthread_exit(NULL);
}

/*
 * Starts a timer that prints out the best move and stops the search on completion
 */
void* timerThreadFunction(void* duration_ms_ptr) {
    #ifdef DEBUG
    printf("info string Running timer thread function\n");
    fflush(stdout);
    #endif

    i64 duration_ms = *(i64*)duration_ms_ptr;
    free(duration_ms_ptr);

    //Set up time to wait to
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec  += duration_ms / 1000;
    ts.tv_nsec += (duration_ms % 1000) * 1000000;

    u8 timed_out; // Whether or not to print after completion

    pthread_mutex_lock(&timer_mutex);
    int result = pthread_cond_timedwait(&timer_cond, &timer_mutex, &ts);
    
    if (result == ETIMEDOUT) {
        timed_out = TRUE;
        #ifdef DEBUG
        printf("info string Timer expired\n");
        fflush(stdout);
        #endif
    } else {
        timed_out = FALSE; // We dont want to print if manually stopped
        #ifdef DEBUG
        printf("info string Timer triggered early\n");
        fflush(stdout);
        #endif
    }
    pthread_mutex_unlock(&timer_mutex);
    
    
    if(timed_out){ // Call the print function after waking up
        search_timed_out();
    }

    #ifdef DEBUG
    printf("info string timer thread function finished\n");
    fflush(stdout);
    #endif

    return (void*)(i64)result;
}

i32 startTimerThread(i64 duration_ms) {
    i64 *durationPtr = malloc(sizeof(i64));
    #ifdef DEBUG
    printf("info string Starting timer thread\n");
    fflush(stdout);
    #endif

    if(!durationPtr){
        printf("info string Warning failed to allocate space for duration pointer.\n");
        return -1;
    }
    *durationPtr = duration_ms;
    
    if (pthread_create(&timerThread, NULL, timerThreadFunction, durationPtr)) {
        perror("info string Failed to create timer thread\n");
        return 1;
    }
    #ifdef DEBUG
    printf("info string Timer thread started\n");
    fflush(stdout);
    #endif
    return 0;
}

/*
 * Cancels the timer thread
 */
void stopTimerThread() {
    #ifdef DEBUG
    printf("info string Stopping timer thread\n");
    fflush(stdout);
    #endif

    pthread_mutex_lock(&timer_mutex);
    pthread_cond_signal(&timer_cond);
    pthread_mutex_unlock(&timer_mutex);

    #ifdef DEBUG
    printf("info string Timer thread stopped\n");
    fflush(stdout);
    #endif
}


void *input_thread_entry(void *arg) {
    (void)arg;

    #ifdef DEBUG
    printf("info string Input Thread running...\n");
    #endif

    inputLoop();
    return NULL;
}

void *output_thread_entry(void *arg) {
    (void)arg;

    #ifdef DEBUG
    printf("info string Output Thread running...\n");
    #endif

    outputLoop();
    return NULL;
}

/**
 * Creates the search thread and initializes
 * the search thread data structure
 */
void *search_thread_entry(void *arg) {
    u32 thread_num = *(u32*)arg;
    free(arg);
    ThreadData td = {0};
    td.thread_num = thread_num;

    #ifdef DEBUG
    printf("info string Search Thread Starting\n");
    fflush(stdout);
    #endif
    
    enter_loop(&td);
    return NULL;
}

void start_search_threads(){
    #ifdef DEBUG
    printf("info string starting search threads\n");
    #endif
    run_get_best_move = TRUE;
    for (i32 i = 0; i < NUM_THREADS; i++) {
        u32* thread_num = malloc(sizeof(u32));
        if(!thread_num){
            printf("info string Warning: failed to allocate memory in start search threads.\n");
            return;
        }
        *thread_num = i;
        pthread_create(&search_threads[i], NULL, search_thread_entry, thread_num);
    }
}

void stopSearchThreads(){
    #ifdef DEBUG
    printf("info string stopping search threads\n");
    #endif
    run_get_best_move = false;
}

i32 launch_threads(void){
    pthread_t input_thread, output_thread;
    if (pthread_create(&input_thread, NULL, input_thread_entry, NULL)) {
        fprintf(stderr, "info string Error creating Input thread\n");
        return 1;
    }
    if (pthread_create(&output_thread, NULL, output_thread_entry, NULL)) {
        fprintf(stderr, "info string Error creating Output thread\n");
        return 1;
    }
    pthread_join(input_thread, NULL);
    pthread_join(output_thread, NULL);
    return 0;
}
