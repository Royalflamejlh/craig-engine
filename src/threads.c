
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

#if defined(__unix__) || defined(__APPLE__)

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

void *search_thread_entry(void *arg) {
    u32 thread_num = *(u32*)arg;
    free(arg);

    #ifdef DEBUG
    printf("info string Search Thread Starting\n");
    fflush(stdout);
    #endif

    search_loop(thread_num);
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

#elif defined(_WIN32) || defined(_WIN64)

#include <windows.h>
static HANDLE search_threads[NUM_THREADS];

// Global variable to control the timer thread
volatile i32 runTimerThread = 1;
static HANDLE timerThread;

// Quits thread
void quit_thread(){
    ExitThread(0);
}

// Timer thread function
DWORD WINAPI timerThreadFunction(LPVOID durationPtr) {
    i64 duration = *(i64*)durationPtr;
    free(durationPtr);
    Sleep((DWORD)(duration));  // Sleep for the specified duration, convert seconds to milliseconds

    if (runTimerThread) {
        while(best_move_found == FALSE);
        stopSearchThreads();
        print_best_move = TRUE;
    }

    return 0;
}

i32 startTimerThread(i64 duration_ms) {
    runTimerThread = 1;
    DWORD threadId;
    i64 *durationPtr = malloc(sizeof(i64));
    if(!durationPtr){
        printf("info string Warning failed to allocate space for duration pointer.\n");
        return -1;
    }
    *durationPtr = duration_ms;
    timerThread = CreateThread(NULL, 0, timerThreadFunction, durationPtr, 0, &threadId);
    if (timerThread == NULL) {
        printf("info string Failed to create timer thread\n");
        return -1;
    }
    return 0;
}

void stopTimerThread() {
    runTimerThread = FALSE;
    WaitForSingleObject(timerThread, INFINITE);
    CloseHandle(timerThread);
}

DWORD WINAPI input_thread_entry(LPVOID arg) {
    (void)arg;
    #ifdef DEBUG
    printf("info string Input Thread running...\n");
    #endif
    inputLoop();
    return 0;
}

DWORD WINAPI output_thread_entry(LPVOID arg) {
    (void)arg;
    #ifdef DEBUG
    printf("info string Output Thread running...\n");
    #endif
    outputLoop();
    return 0;
}

DWORD WINAPI search_thread_entry(LPVOID arg) {
    u32 thread_num = *(u32*)arg;
    free(arg);

    #ifdef DEBUG
    printf("info string Search Thread Starting\n");
    fflush(stdout);
    #endif

    search_loop(thread_num);
    return 0;
}

void start_search_threads(){
    run_get_best_move = true;
    DWORD threadId;
    for (i32 i = 0; i < NUM_THREADS && run_get_best_move; i++) {
        u32* thread_num = malloc(sizeof(u32));
        if(!thread_num){
            printf("info string Warning: failed to allocate memory in start search threads.\n");
            return;
        }
        *thread_num = i;
        search_threads[i] = CreateThread(NULL, 0, search_thread_entry, thread_num, 0, &threadId);
    }
}

void stopSearchThreads(){
    run_get_best_move = false;
}

i32 launch_threads(void){
    HANDLE input_thread, output_thread;
    DWORD inputThreadId, outputThreadId;
    input_thread = CreateThread(NULL, 0, input_thread_entry, NULL, 0, &inputThreadId);
    if (input_thread == NULL) {
        fprintf(stderr, "info string Error creating Input thread\n");
        return 1;
    }

    output_thread = CreateThread(NULL, 0, output_thread_entry, NULL, 0, &outputThreadId);
    if (output_thread == NULL) {
        fprintf(stderr, "info string Error creating Output thread\n");
        return 1;
    }

    WaitForSingleObject(input_thread, INFINITE);
    CloseHandle(input_thread);

    WaitForSingleObject(output_thread, INFINITE);
    CloseHandle(output_thread);

    return 0;
}

#endif // Windows
