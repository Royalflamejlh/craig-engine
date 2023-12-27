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

//Fake Headers
int readInput();
int searchLoop();
static void printBestMove();
/*
* Meet the Globals
*/
Position global_position;
int run_get_best_move;
Move global_best_move;

typedef struct {
    long wtime; 
    long btime; 
} SearchParameters;

#if defined(__unix__) || defined(__APPLE__)

#include <pthread.h>
#include <unistd.h>

static pthread_t search_threads[NUM_SEARCH_THREADS];

// Global variable to control the timer thread
volatile int runTimerThread = 1;
static pthread_t timerThread;

// Timer thread function
void* timerThreadFunction(void* durationPtr) {
    long duration = *(long*)durationPtr;
    free(durationPtr);
    sleep(duration / 1000);  // Sleep for the specified duration

    if (runTimerThread) {
        printBestMove();  // Call the function after waking up
    }

    return NULL;
}

int startTimerThread(long durationInSeconds) {
    long *durationPtr = malloc(sizeof(long));
    if(!durationPtr){
        printf("info Warning failed to allocate space for duration pointer.");
        return -1;
    }
    *durationPtr = durationInSeconds;
    runTimerThread = 1;
    if (pthread_create(&timerThread, NULL, timerThreadFunction, durationPtr)) {
        perror("Failed to create timer thread");
        return 1;
    }
    return 0;
}

void stopTimerThread() {
    runTimerThread = 0;
    pthread_join(timerThread, NULL); 
}


void *io_thread_entry(void *arg) {
    (void)arg;
    printf("IO Thread running...\n");
    readInput();
    return NULL;
}

void *search_thread_entry(void *arg) {
    (void)arg;
    searchLoop();
    return NULL;
}

void startSearchThreads(){
    run_get_best_move = true;
    for (int i = 0; i < NUM_SEARCH_THREADS; i++) {
        pthread_create(&search_threads[i], NULL, search_thread_entry, NULL);
    }
}

void stopSearchThreads(){
    run_get_best_move = false;
    for (int i = 0; i < NUM_SEARCH_THREADS; i++) {
        if(search_threads[i]){
            pthread_join(search_threads[i], NULL);
        }
    }
}


static int launch_threads(void){
    pthread_t io_thread;
    if (pthread_create(&io_thread, NULL, io_thread_entry, NULL)) {
        fprintf(stderr, "Error creating IO thread\n");
        return 1;
    }

    pthread_join(io_thread, NULL);
    
    return 0;
}

#elif defined(_WIN32) || defined(_WIN64)

#include <windows.h>
static HANDLE search_threads[NUM_SEARCH_THREADS];

// Global variable to control the timer thread
volatile int runTimerThread = 1;
static HANDLE timerThread;

// Timer thread function
DWORD WINAPI timerThreadFunction(LPVOID durationPtr) {
    long duration = *(long*)durationPtr;
    free(durationPtr);
    Sleep((DWORD)(duration));  // Sleep for the specified duration, convert seconds to milliseconds

    if (runTimerThread) {
        printBestMove();  // Call the function after waking up
    }

    fflush(stdout);
    return 0;
}

int startTimerThread(long durationInSeconds) {
    runTimerThread = 1;
    DWORD threadId;
    long *durationPtr = malloc(sizeof(long));
    if(!durationPtr){
        printf("info Warning failed to allocate space for duration pointer.");
        return -1;
    }
    *durationPtr = durationInSeconds;
    timerThread = CreateThread(NULL, 0, timerThreadFunction, durationPtr, 0, &threadId);
    if (timerThread == NULL) {
        printf("Failed to create timer thread\n");
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
    printf("IO Thread running...\n");
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
    for (int i = 0; i < NUM_SEARCH_THREADS; i++) {
        search_threads[i] = CreateThread(NULL, 0, search_thread_entry, NULL, 0, &threadId);
    }
}

void stopSearchThreads(){
    run_get_best_move = false;
    for (int i = 0; i < NUM_SEARCH_THREADS; i++) {
        if(search_threads[i]){
            WaitForSingleObject(search_threads[i], INFINITE);
            CloseHandle(search_threads[i]);
        }
    }
}

static int launch_threads(void){
    HANDLE io_thread;
    DWORD ioThreadId;
    io_thread = CreateThread(NULL, 0, io_thread_entry, NULL, 0, &ioThreadId);
    if (io_thread == NULL) {
        fprintf(stderr, "Error creating IO thread\n");
        return 1;
    }

    WaitForSingleObject(io_thread, INFINITE);
    CloseHandle(io_thread);

    return 0;
}

#endif

int main(void) {
    generateMasks();
    generateMagics();
    initZobrist();
    initPST();
    if(initTT()){
        printf("WARNING FAILED TO ALLOCATED SPACE FOR TRANSPOSITION TABLE\n");
        return -1;
    }
    #ifdef RUN_TEST
    testBB();
    #endif

    #ifdef __PROFILE
    printf("In profile mode, running forever.\r\n");
    fflush(stdout);
    playSelfInfinite();
    #endif  

    // Create IO thread
    global_position = fenToPosition(START_FEN);
    global_best_move = NO_MOVE;
    launch_threads();

      

    printf("All threads have finished.\n");
    return 0;
}


 

/*
* Search function stuff
*/
static void processUCI(void) {
    printf("id name CraigEngine\r\n");
    printf("id author John\r\n");
    printf("uciok\r\n");
    fflush(stdout);
}

static void processIsReady(void) {
    printf("readyok\r\n");
    fflush(stdout);
}

static char isNullMove(char* move){
    if(move == NULL || strlen(move) < 4) return 0;
    for(int i = 0; i < 4; i++){
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

static void printBestMove(){
    char str[6];
    Move bestMove = global_best_move;
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

    printf("bestmove %s\r\n", str);
    return;

}

void processGoCommand(char* input) {
    char* token;
    char* saveptr;
    SearchParameters searchParams;
    searchParams.wtime = -1; // Initialize with default values
    searchParams.btime = -1;

    token = strtok_r(input, " ", &saveptr);
    while (token != NULL) {
        if (strcmp(token, "infinite") == 0) {
            return;
        } else if (strcmp(token, "wtime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                searchParams.wtime = atol(token);
            }
        } else if (strcmp(token, "btime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                searchParams.btime = atol(token);
            }
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    if(global_position.flags & WHITE_TURN){
        startTimerThread(searchParams.wtime / 20);
    }
    else{
        startTimerThread(searchParams.btime / 20);
    }
}

static int processInput(char* input){
    if (strncmp(input, "uci", 3) == 0) {
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
        startSearchThreads();
        processGoCommand(input + 3);
        fflush(stdout);
    }
    else if (strncmp(input, "stop", 4) == 0){
        stopSearchThreads();
        stopTimerThread();
        printBestMove();
        fflush(stdout);
    }
    else if (strncmp(input, "debug", 5) == 0){
        printPosition(global_position, TRUE);
        printf("Best move is: ");
        printMove(global_best_move);
        printf("\r\n");
        fflush(stdout);
    }
    else if (strncmp(input, "quit", 4) == 0) {
        return -1; 
    }
    return 0;
}


int readInput(){
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

int searchLoop(){
    while(TRUE){
        if(global_position.hash && run_get_best_move){
            if(getBestMove(global_position)){
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