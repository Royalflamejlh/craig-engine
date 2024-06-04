#include "bitboard/bbutils.h"
#include "movement.h"
#include "tests/bbtests.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "transposition.h"
#include "bitboard/bitboard.h"
#include "bitboard/magic.h"
#include "hash.h"
#include "evaluator.h"
#include "globals.h"
#include "tree.h"
#include "types.h"
#include "util.h"

#ifdef __COMPILE_DEBUG
#define RUN_TEST
#elif defined(__PROFILE)
void playSelfInfinite(void);
#endif

#define NUM_SEARCH_THREADS 1

//Function Headers
i32 readInput();
i32 searchLoop();

static void printBestMove();

/*
* Meet the Globals
*/
volatile Position global_position;
volatile i32 run_get_best_move;
volatile Move global_best_move;
volatile u8 is_searching;

typedef struct {
    i64 wtime; 
    i64 btime; 
} SearchParameters;

#if defined(__unix__) || defined(__APPLE__)

#include <pthread.h>
#include <unistd.h>
#include <errno.h>

static pthread_t search_threads[NUM_SEARCH_THREADS];

// Global variable to control the timer thread
pthread_mutex_t timer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t timer_cond = PTHREAD_COND_INITIALIZER;
static pthread_t timerThread;

// Timer thread function
void* timerThreadFunction(void* durationPtr) {
    #ifdef DEBUG
    printf("info string Running timer thread function\n");
    fflush(stdout);
    #endif

    i64 duration = *(i64*)durationPtr;
    free(durationPtr);

    //Set up time to wait to
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += duration;

    u8 print; // Whether or not to print after completion

    pthread_mutex_lock(&timer_mutex);
    int result = pthread_cond_timedwait(&timer_cond, &timer_mutex, &ts);
    
    if (result == ETIMEDOUT) {
        print = TRUE;
        #ifdef DEBUG
        printf("info string Timer expired\n");
        fflush(stdout);
        #endif
    } else {
        print = FALSE; // We dont want to print if manually stopped
        #ifdef DEBUG
        printf("info string Timer triggered early\n");
        fflush(stdout);
        #endif
    }
    pthread_mutex_unlock(&timer_mutex);
    
    if(print) printBestMove();  // Call the function after waking up

    #ifdef DEBUG
    printf("info string timer thread function finished\n");
    fflush(stdout);
    #endif

    return (void*)(i64)result;
}

i32 startTimerThread(i64 durationInSeconds) {
    i64 *durationPtr = malloc(sizeof(i64));
    #ifdef DEBUG
    printf("info string Starting timer thread\n");
    fflush(stdout);
    #endif

    if(!durationPtr){
        printf("info string Warning failed to allocate space for duration pointer.\n");
        return -1;
    }
    *durationPtr = (durationInSeconds / 1000);
    
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


void *io_thread_entry(void *arg) {
    (void)arg;

    #ifdef DEBUG
    printf("info string IO Thread running...\n");
    #endif

    readInput();
    return NULL;
}

void *search_thread_entry(void *arg) {
    (void)arg;

    #ifdef DEBUG
    printf("info string Search Thread Starting\n");
    fflush(stdout);
    #endif

    searchLoop();
    return NULL;
}

void startSearchThreads(){
    #ifdef DEBUG
    printf("info string starting search threads\n");
    #endif

    run_get_best_move = true;
    for (i32 i = 0; i < NUM_SEARCH_THREADS; i++) {
        pthread_create(&search_threads[i], NULL, search_thread_entry, NULL);
    }
}

void stopSearchThreads(){
    #ifdef DEBUG
    printf("info string stopping search threads\n");
    #endif

    run_get_best_move = false;
    for (i32 i = 0; i < NUM_SEARCH_THREADS; i++) {
        if(search_threads[i]){
            pthread_join(search_threads[i], NULL);
        }
    }
}


static i32 launch_threads(void){
    pthread_t io_thread;
    if (pthread_create(&io_thread, NULL, io_thread_entry, NULL)) {
        fprintf(stderr, "info string Error creating IO thread\n");
        return 1;
    }

    pthread_join(io_thread, NULL);
    
    return 0;
}

#elif defined(_WIN32) || defined(_WIN64)

#include <windows.h>
static HANDLE search_threads[NUM_SEARCH_THREADS];

// Global variable to control the timer thread
volatile i32 runTimerThread = 1;
static HANDLE timerThread;

// Timer thread function
DWORD WINAPI timerThreadFunction(LPVOID durationPtr) {
    i64 duration = *(i64*)durationPtr;
    free(durationPtr);
    Sleep((DWORD)(duration));  // Sleep for the specified duration, convert seconds to milliseconds

    if (runTimerThread) {
        printBestMove();  // Call the function after waking up
    }

    fflush(stdout);
    return 0;
}

i32 startTimerThread(i64 durationInSeconds) {
    runTimerThread = 1;
    DWORD threadId;
    i64 *durationPtr = malloc(sizeof(i64));
    if(!durationPtr){
        printf("info string Warning failed to allocate space for duration pointer.\n");
        return -1;
    }
    *durationPtr = durationInSeconds;
    timerThread = CreateThread(NULL, 0, timerThreadFunction, durationPtr, 0, &threadId);
    if (timerThread == NULL) {
        printf("info string Failed to create timer thread\n");
        return -1;
    }
    return 0;
}

void stopTimerThread() {
    runTimerThread = 0;
    WaitForSingleObject(timerThread, INFINITE);
    CloseHandle(timerThread);
}

DWORD WINAPI io_thread_entry(LPVOID arg) {
    (void)arg;
    #ifdef DEBUG
    printf("info string IO Thread running...\n");
    #endif
    readInput();
    return 0;
}

DWORD WINAPI search_thread_entry(LPVOID arg) {
    (void)arg;
    searchLoop();
    return 0;
}

void startSearchThreads(){
    run_get_best_move = true;
    DWORD threadId;
    for (i32 i = 0; i < NUM_SEARCH_THREADS; i++) {
        search_threads[i] = CreateThread(NULL, 0, search_thread_entry, NULL, 0, &threadId);
    }
}

void stopSearchThreads(){
    run_get_best_move = false;
    for (i32 i = 0; i < NUM_SEARCH_THREADS; i++) {
        if(search_threads[i]){
            WaitForSingleObject(search_threads[i], INFINITE);
            CloseHandle(search_threads[i]);
        }
    }
}

static i32 launch_threads(void){
    HANDLE io_thread;
    DWORD ioThreadId;
    io_thread = CreateThread(NULL, 0, io_thread_entry, NULL, 0, &ioThreadId);
    if (io_thread == NULL) {
        fprintf(stderr, "info string Error creating IO thread\n");
        return 1;
    }

    WaitForSingleObject(io_thread, INFINITE);
    CloseHandle(io_thread);

    return 0;
}

#endif // Windows

/*
* Search function stuff
*/
static void processUCI(void) {
    printf("id name CraigEngine\r\n");
    printf("id author John\r\n");
    printf("uciok\r\n");
}

static void processIsReady(void) {
    printf("readyok\r\n");
    fflush(stdout);
}

static char isNullMove(char* move){
    if(move == NULL || strlen(move) < 4) return 0;
    for(i32 i = 0; i < 4; i++){
        if(move[i] != '0') return 0;
    }
    return 1;
}

char* trimWhitespace(char* str) {
    char* end;

    while(isspace((unsigned char)*str)) str++;

    if(*str == 0) {
        return str;
    }

    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    end[1] = '\0';
    return str;
}

void processMoves(char* str) {
    char* pch;
    char* rest = str; 
    pch = strtok_r(str, " ", &rest);
    Position pos = fenToPosition(START_FEN);
    
    while (pch != NULL) {
        char* moveStr = trimWhitespace(pch);
        if(isNullMove(moveStr)){
            goto get_next_token;
        }
        //printf("Move String found: %s", moveStr);
        makeMove(&pos, moveStrToType(pos, moveStr));
get_next_token:
        pch = strtok_r(NULL, " ", &rest);
    }
    global_position = pos;
}


static void search(u32 time){
    startSearchThreads();
    if(time != 0){
        #ifdef DEBUG
        printf("info string Search time is: %d\n", time);
        #endif
        while(!is_searching); // Wait for search to begin
        startTimerThread(time);
    }
}

static void stopSearch(){
    stopTimerThread();
    printBestMove();
    stopSearchThreads();
    is_searching = FALSE;
}

static void printBestMove(){
    char str[6];
    volatile Move bestMove = global_best_move;
    while(bestMove == NO_MOVE){
        bestMove = global_best_move;
    }
    str[0] = (GET_FROM(bestMove) % 8) + 'a';
    str[1] = (GET_FROM(bestMove) / 8) + '1';
    str[2] = (GET_TO(bestMove) % 8) + 'a';
    str[3] = (GET_TO(bestMove) / 8) + '1';
    str[4] = '\0';

    switch(GET_FLAGS(bestMove)){
        case QUEEN_PROMO_CAPTURE:
        case QUEEN_PROMOTION:
            str[4] = 'q';
            str[5] = '\0';
            break;
        case ROOK_PROMO_CAPTURE:
        case ROOK_PROMOTION:
            str[4] = 'r';
            str[5] = '\0';
            break;
        case BISHOP_PROMO_CAPTURE:
        case BISHOP_PROMOTION:
            str[4] = 'b';
            str[5] = '\0';
            break;
        case KNIGHT_PROMO_CAPTURE:
        case KNIGHT_PROMOTION:
            str[4] = 'n';
            str[5] = '\0';
            break;
        default:
            break;
    }

    #if defined(_WIN32) || defined(_WIN64)
    printf("bestmove %s\r\n", str);
    #else
    printf("bestmove %s\n", str);
    #endif

    fflush(stdout);
    return;

}

void processGoCommand(char* input) {
    char* token;
    char* saveptr;
    u32 wtime = 0; // Initialize with default values
    u32 btime = 0;
    u32 time = 0;

    token = strtok_r(input, " ", &saveptr);
    while (token != NULL) {
        if (strcmp(token, "infinite") == 0) {
            time = 0;
        } else if (strcmp(token, "wtime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                wtime = atol(token);
            }
        } else if (strcmp(token, "btime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                btime = atol(token);
            }
        } else if (strcmp(token, "movetime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                time = atol(token);
            }
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    if(global_position.flags & WHITE_TURN){
        time += (wtime / 20);
    }
    else{
        time += (btime / 20);
    }

    search(time);
}

static i32 processInput(char* input){
    if (strncmp(input, "uci", 3) == 0) {
        input += 3;
        if(strncmp(input, "newgame", 7) == 0) return 0;
        processUCI();
        fflush(stdout);
        return 0;
    } 
    else if (strncmp(input, "isready", 7) == 0) {
        processIsReady();
        fflush(stdout);
        return 0;
    }
    else if (strncmp(input, "position", 8) == 0) {
        input += 9;
        global_best_move = NO_MOVE; // Reset best move for current position
        stopSearchThreads();
        if (strncmp(input, "startpos", 8) == 0) {
            input += 9;
            if (strncmp(input, "moves", 5) == 0) {
                input += 6;
                processMoves(input);
            }
            else{
                global_position = fenToPosition(START_FEN); 
            }
        }
        else if (strncmp(input, "fen", 3) == 0) {
            input += 4;
            global_position = fenToPosition(input);
        }
        fflush(stdout);
    }
    else if (strncmp(input, "go", 2) == 0) {
        processGoCommand(input + 3);
    }
    else if (strncmp(input, "stop", 4) == 0){
        #ifdef DEBUG
        printf("info string Stopping\n");
        #endif
        //printBestMove();
        stopSearch();
    }
    #ifdef DEBUG
    else if (strncmp(input, "debug", 5) == 0){
        input += 6;
        if (strncmp(input, "position", 10) == 0) {
            printPosition(global_position, TRUE);
        }
        else if (strncmp(input, "bestmove", 8) == 0) {
            printf("Current bestmove is: ");
            printMove(global_best_move);
            printf("\n");
        }
        else if (strncmp(input, "list moves", 10) == 0) {
            Move debug_moves[MAX_MOVES];
            u32 size = generateLegalMoves(global_position, debug_moves);
            printf("Moves: \n");
            for(u32 i = 0; i < size; i++){
                printMove(debug_moves[i]);
                printf("\n");
            }
        }

    }
    #endif
    else if (strncmp(input, "quit", 4) == 0) {
        return -1; 
    }
    return 0;
}


i32 readInput(){
    char input[1024];
    input[1023] = '\0';

    while (true) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }
        if(processInput(input)){
            break;
        }
    }
    return 0;
}

/*
* Search thread stuff
*/

i32 searchLoop(){
    is_searching = TRUE;
    while(TRUE){
        if(global_position.hash && run_get_best_move){
            if(getBestMove(global_position 
            #ifdef MAX_DEPTH
            , MAX_DEPTH
            #endif
            )){
                return -1;
            }
        }
    }
    return 0;
}

#ifdef __PROFILE
void playSelfInfinite(void){
    global_position = fenToPosition(START_FEN);
    global_best_move = NO_MOVE;
    run_get_best_move = TRUE;
    Move moveList[MAX_MOVES];
    while(generateLegalMoves(global_position, moveList)){
        getBestMove(global_position);
        if(global_best_move != NO_MOVE){
            makeMove(&global_position, global_best_move);
            printPosition(global_position, FALSE);
        }
    }
}
#endif

/*
* Behold the main function
*/
i32 main(void) {
    generateMasks();
    generateMagics();
    initZobrist();
    initPST();
    if(initTT()){
        printf("info string WARNING FAILED TO ALLOCATED SPACE FOR TRANSPOSITION TABLE\n");
        return -1;
    }
    is_searching = FALSE;
    #ifdef RUN_TEST
    testBB();
    #endif

    #ifdef __PROFILE
    printf("info string In profile mode, playing forever.\r\n");
    fflush(stdout);
    playSelfInfinite();
    #endif  

    // Create IO thread
    global_position = fenToPosition(START_FEN);
    global_best_move = NO_MOVE;
    launch_threads();

    
    printf("info string All threads have finished.\n");
    return 0;
}