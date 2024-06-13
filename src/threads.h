#ifndef THREADS_H
#define THREADS_H
#include "types.h"


#define MIN_HELPER_DEPTH 8 // At what depth to launch helper threads


#define NUM_THREADS      5 // Total number of threads
#define NUM_MAIN_THREADS 1 // How main of these are main threads (remaining will be helpers)

i32 startTimerThread(i64 durationInSeconds);
void stopTimerThread();
void start_search_threads();
void stopSearchThreads();
void quit_thread();
i32 launch_threads(void);
#endif