#ifndef THREADS_H
#define THREADS_H
#include "types.h"
i32 startTimerThread(i64 durationInSeconds);
void stopTimerThread();
void startSearchThreads();
void stopSearchThreads();
i32 launch_threads(void);
#endif