#include "globals.h"
#include <stdlib.h>
#include "bitboard/bbutils.h"
#include "types.h"
#include "string.h"
#include "util.h"

#if defined(__unix__) || defined(__APPLE__) // UNIX
#include <pthread.h>
#elif defined(_WIN32) || defined(_WIN64) // WINDOWS
#include <windows.h>
#endif

// Flags
_Atomic volatile i32 run_program;
_Atomic volatile i32 run_get_best_move;
_Atomic volatile i32 best_move_found;

// Signals
_Atomic volatile i32 print_pv_info;
_Atomic volatile i32 print_best_move;

// Position Data
static Position global_position;

// PV Search Data
static SearchData global_sd;

#if defined(__unix__) || defined(__APPLE__) // Unix
static pthread_mutex_t mutex_global_position = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_global_PV = PTHREAD_MUTEX_INITIALIZER;
#elif defined(_WIN32) || defined(_WIN64)
static CRITICAL_SECTION mutex_global_position;
static CRITICAL_SECTION mutex_global_PV;
#endif // Windows

/*
 * Sets up Initial Global Data Values
 */
void init_globals(){
    run_program = TRUE;
    run_get_best_move = FALSE;
    best_move_found = FALSE;
    print_pv_info = FALSE;
    print_best_move = FALSE;

#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_position);
    global_position = fenToPosition(START_FEN);
    pthread_mutex_lock(&mutex_global_PV);
    global_sd.PVArray = calloc(MAX_DEPTH, sizeof(Move));
    global_sd.depth = 0;
    global_sd.best_move = NO_MOVE;
    global_sd.eval = 0;
    pthread_mutex_unlock(&mutex_global_PV);
    pthread_mutex_unlock(&mutex_global_position);
#elif defined(_WIN32) || defined(_WIN64)
    InitializeCriticalSection(&mutex_global_position);
    InitializeCriticalSection(&mutex_global_PV);

    EnterCriticalSection(&mutex_global_position);
    global_position = fenToPosition(START_FEN);
    EnterCriticalSection(&mutex_global_PV);
    global_sd.PVArray = calloc(MAX_DEPTH, sizeof(Move));
    global_sd.depth = 0;
    global_sd.best_move = NO_MOVE;
    global_sd.eval = 0;
    LeaveCriticalSection(&mutex_global_PV);
    LeaveCriticalSection(&mutex_global_position);
#endif
}

/*
 * Frees Data Store for Global Data
 */
void free_globals(){
#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_position);

    removeHashStack(&global_position.hashStack);

    pthread_mutex_lock(&mutex_global_PV);
    free(global_sd.PVArray);
    global_sd.PVArray = NULL;

    pthread_mutex_unlock(&mutex_global_PV);
    pthread_mutex_unlock(&mutex_global_position);
#elif defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&mutex_global_position);

    removeHashStack(&global_position.hashStack);

    EnterCriticalSection(&mutex_global_PV);
    free(global_sd.PVArray);
    global_sd.PVArray = NULL;

    LeaveCriticalSection(&mutex_global_PV);
    LeaveCriticalSection(&mutex_global_position);

    DeleteCriticalSection(&mutex_global_PV);
    DeleteCriticalSection(&mutex_global_position);
#endif
}

/*
 * Resets the global PV Data on a position change
 */
static void reset_global_pv_data(){
    best_move_found = FALSE;
    print_pv_info = FALSE;
#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_PV);
    global_sd.depth = 0;
    global_sd.best_move = NO_MOVE;
    global_sd.eval = 0;
    pthread_mutex_unlock(&mutex_global_PV);
#elif defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&mutex_global_PV);
    global_sd.depth = 0;
    global_sd.best_move = NO_MOVE;
    global_sd.eval = 0;
    LeaveCriticalSection(&mutex_global_PV);
#endif
}

/*
 * Checks and sees if the Global PV can be updated, and if it can it updates it
 */
void update_global_pv(u32 depth, Move* pvArray, i32 eval, SearchStats stats){
    if(pvArray == NULL || pvArray[0] == NO_MOVE) return;

#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_PV); // Start Crit Section

    if(depth <= global_sd.depth){ // If new depth is less or same as current exit
        pthread_mutex_unlock(&mutex_global_PV);
        return;
    }

    global_sd.depth = depth;
    global_sd.eval = eval;
    global_sd.stats = stats;
    global_sd.best_move = pvArray[0];
    memcpy(global_sd.PVArray, pvArray, (MAX_DEPTH)*sizeof(Move));

    pthread_mutex_unlock(&mutex_global_PV);
#elif defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&mutex_global_PV); // Start Crit Section

    if(depth <= global_sd.depth){ // If new depth is less or same as current exit
        LeaveCriticalSection(&mutex_global_PV);
        return;
    }

    global_sd.depth = depth;
    global_sd.eval = eval;
    global_sd.stats = stats;
    global_sd.best_move = pvArray[0];
    memcpy(global_sd.PVArray, pvArray, (MAX_DEPTH)*sizeof(Move));

    LeaveCriticalSection(&mutex_global_PV);
#endif
    best_move_found = TRUE; // Set flag that best move has been found
    print_pv_info = TRUE;   // Set flag to print new PV
}

/*
 * Sets the global position to the supplied position
 */
void set_global_position(Position pos){
#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_position);
    if(pos.hashStack.ptr != global_position.hashStack.ptr) removeHashStack(&global_position.hashStack);
    global_position = pos;
    reset_global_pv_data();
    pthread_mutex_unlock(&mutex_global_position);
#elif defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&mutex_global_position);
    if(pos.hashStack.ptr != global_position.hashStack.ptr) removeHashStack(&global_position.hashStack);
    global_position = pos;
    reset_global_pv_data();
    LeaveCriticalSection(&mutex_global_position);
#endif
}

/*
 * Gets the global position
 */
Position get_global_position(){
#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_position);
    Position pos = global_position;
    pthread_mutex_unlock(&mutex_global_position);
#elif defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&mutex_global_position);
    Position pos = global_position;
    LeaveCriticalSection(&mutex_global_position);
#endif
    return pos;
}

/*
 * Returns a new copy of the global position
 */
Position copy_global_position(){
#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_position);
    Position pos = global_position;
    pos.hashStack = createHashStack();
    memcpy(pos.hashStack.ptr, global_position.hashStack.ptr, sizeof(u64)*HASHSTACK_SIZE);
    pos.hashStack.current_idx    = global_position.hashStack.current_idx;
    pos.hashStack.last_reset_idx = global_position.hashStack.last_reset_idx;
    pthread_mutex_unlock(&mutex_global_position);
#elif defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&mutex_global_position);
    Position pos = global_position;
    pos.hashStack = createHashStack();
    memcpy(pos.hashStack.ptr, global_position.hashStack.ptr, sizeof(u64)*HASHSTACK_SIZE);
    pos.hashStack.current_idx    = global_position.hashStack.current_idx;
    pos.hashStack.last_reset_idx = global_position.hashStack.last_reset_idx;
    LeaveCriticalSection(&mutex_global_position);
#endif
    return pos;
}

/*
 * Returns the Global Best Move
 */
Move get_global_best_move() {
    Move move;
#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_PV);
    move = global_sd.best_move;
    pthread_mutex_unlock(&mutex_global_PV);
#elif defined(_WIN32) || defined(_WIN64)  
    EnterCriticalSection(&mutex_global_PV);
    move = global_sd.best_move;
    LeaveCriticalSection(&mutex_global_PV);
#endif
    return move;
}

/*
 * Returns the Global PV Data (ALLOCATES MEMORY IN RETURNED PV ARRAY)
 */
SearchData get_global_pv_data(){
    SearchData data;

#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_lock(&mutex_global_PV); // Start Crit Section
#elif defined(_WIN32) || defined(_WIN64)    
    EnterCriticalSection(&mutex_global_PV);
#endif

    data.depth = global_sd.depth;
    data.eval = global_sd.eval;
    data.stats = global_sd.stats;
    data.best_move = global_sd.best_move;
    data.PVArray = malloc(MAX_DEPTH*sizeof(Move));
    memcpy(data.PVArray, global_sd.PVArray, (MAX_DEPTH)*sizeof(Move));

#if defined(__unix__) || defined(__APPLE__)
    pthread_mutex_unlock(&mutex_global_PV);
#elif defined(_WIN32) || defined(_WIN64)   
    LeaveCriticalSection(&mutex_global_PV);
#endif
    return data;
}


