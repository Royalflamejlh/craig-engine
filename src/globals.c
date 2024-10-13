#include "globals.h"
#include <stdlib.h>
#include "bitboard/bbutils.h"
#include "types.h"
#include "string.h"
#include "util.h"
#include <pthread.h>

// Flags
_Atomic volatile i32 run_program;
_Atomic volatile i32 run_get_best_move;
_Atomic volatile i32 best_move_found;

// Signals
_Atomic volatile i32 print_pv_info;
_Atomic volatile i32 print_best_move;

// Global Thread Data
static ThreadData global_td;

// PV Search Data
static SearchData global_sd;

static pthread_mutex_t mutex_global_td = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutex_global_PV = PTHREAD_MUTEX_INITIALIZER;

/*
 * Sets up Initial Global Data Values
 */
void init_globals(){
    run_program = TRUE;
    run_get_best_move = FALSE;
    best_move_found = FALSE;
    print_pv_info = FALSE;
    print_best_move = FALSE;

    pthread_mutex_lock(&mutex_global_td);
    memset(&global_td, 0, sizeof(ThreadData));
    global_td.thread_num = 1;
    global_td.pos = fen_to_position(START_FEN);
    pthread_mutex_lock(&mutex_global_PV);
    global_sd.pv_array = calloc(MAX_DEPTH, sizeof(Move));
    global_sd.depth = 0;
    global_sd.best_move = NO_MOVE;
    global_sd.eval = 0;
    pthread_mutex_unlock(&mutex_global_PV);
    pthread_mutex_unlock(&mutex_global_td);
}

/*
 * Frees Data Store for Global Data
 */
void free_globals(){
    pthread_mutex_lock(&mutex_global_td);

    pthread_mutex_lock(&mutex_global_PV);
    free(global_sd.pv_array);
    global_sd.pv_array = NULL;

    pthread_mutex_unlock(&mutex_global_PV);
    pthread_mutex_unlock(&mutex_global_td);
}

/*
 * Resets the global PV Data on a position change
 */
static void reset_global_pv_data(){
    best_move_found = FALSE;
    print_pv_info = FALSE;
    pthread_mutex_lock(&mutex_global_PV);
    global_sd.depth = 0;
    global_sd.best_move = NO_MOVE;
    global_sd.eval = 0;
    pthread_mutex_unlock(&mutex_global_PV);

}

/*
 * Checks and sees if the Global PV can be updated, and if it can it updates it
 * Returns true if an update happen, false if an update did not happen
 */
u8 update_global_pv(u32 depth, Move* pv_array, i32 eval, SearchStats stats){
    if(pv_array == NULL || pv_array[0] == NO_MOVE) return FALSE;

    pthread_mutex_lock(&mutex_global_PV); // Start Crit Section

    if(depth <= global_sd.depth){ // If new depth is less or same as current exit
        pthread_mutex_unlock(&mutex_global_PV);
        return FALSE;
    }

    global_sd.depth = depth;
    global_sd.eval = eval;
    global_sd.stats = stats;
    global_sd.best_move = pv_array[0];
    memcpy(global_sd.pv_array, pv_array, (MAX_DEPTH)*sizeof(Move));

    pthread_mutex_unlock(&mutex_global_PV);

    best_move_found = TRUE; // Set flag that best move has been found
    print_pv_info = TRUE;   // Set flag to print new PV
    return TRUE;
}

/*
 * Returns the Global Best Move
 */
Move get_global_best_move() {
    Move move;
    pthread_mutex_lock(&mutex_global_PV);
    move = global_sd.best_move;
    pthread_mutex_unlock(&mutex_global_PV);
    return move;
}

/*
 * Sets the global position to the supplied position
 * and clears the saved information
 */
void set_global_position(Position pos){
    pthread_mutex_lock(&mutex_global_td);
    memset(&global_td, 0, sizeof(ThreadData));
    global_td.thread_num = 1;
    global_td.pos = pos;
    reset_global_pv_data();
    pthread_mutex_unlock(&mutex_global_td);
}

/*
 * Returns a new copy of the global position
 */
Position copy_global_position(){
    pthread_mutex_lock(&mutex_global_td);
    Position pos = global_td.pos;
    pthread_mutex_unlock(&mutex_global_td);
    return pos;
}

/*
 * Returns a new copy of the global position
 */
void set_global_td(ThreadData td){
    pthread_mutex_lock(&mutex_global_td);
    global_td = td;
    pthread_mutex_unlock(&mutex_global_td);
}

/*
 * Returns a new copy of the global position
 */
ThreadData copy_global_td(){
    pthread_mutex_lock(&mutex_global_td);
    ThreadData pos = global_td;
    pthread_mutex_unlock(&mutex_global_td);
    return pos;
}


/*
 * Returns the Global PV Data (ALLOCATES MEMORY IN RETURNED PV ARRAY)
 */
SearchData get_global_pv_data(){
    SearchData data;

    pthread_mutex_lock(&mutex_global_PV); // Start Crit Section

    data.depth = global_sd.depth;
    data.eval = global_sd.eval;
    data.stats = global_sd.stats;
    data.best_move = global_sd.best_move;
    data.pv_array = malloc(MAX_DEPTH*sizeof(Move));
    memcpy(data.pv_array, global_sd.pv_array, (MAX_DEPTH)*sizeof(Move));

    pthread_mutex_unlock(&mutex_global_PV);

    return data;
}


