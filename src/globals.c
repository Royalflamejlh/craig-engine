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

// Position Data
static Position global_position;

// PV Search Data
static SearchData global_sd;

static pthread_mutex_t mutex_global_position = PTHREAD_MUTEX_INITIALIZER;
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

    pthread_mutex_lock(&mutex_global_position);
    global_position = fen_to_position(START_FEN);
    pthread_mutex_lock(&mutex_global_PV);
    global_sd.pv_array = calloc(MAX_DEPTH, sizeof(Move));
    global_sd.depth = 0;
    global_sd.best_move = NO_MOVE;
    global_sd.eval = 0;
    pthread_mutex_unlock(&mutex_global_PV);
    pthread_mutex_unlock(&mutex_global_position);
}

/*
 * Frees Data Store for Global Data
 */
void free_globals(){
    pthread_mutex_lock(&mutex_global_position);

    remove_hash_stack(&global_position.hashStack);

    pthread_mutex_lock(&mutex_global_PV);
    free(global_sd.pv_array);
    global_sd.pv_array = NULL;

    pthread_mutex_unlock(&mutex_global_PV);
    pthread_mutex_unlock(&mutex_global_position);
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
 * Sets the global position to the supplied position
 */
void set_global_position(Position pos){
    pthread_mutex_lock(&mutex_global_position);
    if(pos.hashStack.ptr != global_position.hashStack.ptr) remove_hash_stack(&global_position.hashStack);
    global_position = pos;
    reset_global_pv_data();
    pthread_mutex_unlock(&mutex_global_position);
}

/*
 * Gets the global position
 */
Position get_global_position(){
    pthread_mutex_lock(&mutex_global_position);
    Position pos = global_position;
    pthread_mutex_unlock(&mutex_global_position);
    return pos;
}

/*
 * Returns a new copy of the global position
 */
Position copy_global_position(){
    pthread_mutex_lock(&mutex_global_position);
    Position pos = global_position;
    pos.hashStack = createHashStack();
    memcpy(pos.hashStack.ptr, global_position.hashStack.ptr, sizeof(u64)*HASHSTACK_SIZE);
    pos.hashStack.current_idx    = global_position.hashStack.current_idx;
    pos.hashStack.last_reset_idx = global_position.hashStack.last_reset_idx;
    pthread_mutex_unlock(&mutex_global_position);
    return pos;
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


