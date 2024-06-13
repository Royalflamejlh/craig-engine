#include <stdbool.h>
#include "search.h"
#include "bitboard/bbutils.h"
#include "threads.h"
#include "globals.h"
#include "tree.h"
#include "evaluator.h"
#include "tables.h"
#include <stdio.h>
#include "pthread.h"
#include "types.h"
#include "util.h"

#ifdef DEBUG
#include <stdio.h>
#endif

_Atomic volatile u8 is_searching;          // Flag for if search loop is running
_Atomic volatile u8 is_helpers_searching;  // Flag for if helper loop is running
_Atomic volatile u8 can_shorten;           // Flag for if can leave before timer finishes
_Atomic volatile u8 print_on_depth;        // Flag for whether or not to print when expected depth is reached

// Search Parameters
_Atomic volatile u8  helpers_run;
_Atomic volatile u32 helpers_search_depth;
_Atomic volatile u32 helper_eval;

_Atomic volatile u32 search_depth;
_Atomic volatile u32 search_time;
_Atomic volatile u64 start_time;


pthread_mutex_t helper_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t helper_cond = PTHREAD_COND_INITIALIZER;
int do_helper_search = FALSE;

#define SEARCH_REDUCTION_LEVEL 0.75    // How much time is reduced when search finds move to reduce time on
#define SEARCH_EXTENSION_LEVEL 1.50    // How much time is expanded when search finds move to extend time on


/*
* Starts the search threads
* Passed the max time and the max depth for the search
* Called from the IO Thread
*/
void start_search(SearchParameters params){
    is_searching   = FALSE; // Set up new search
    print_on_depth = FALSE;

    search_depth = params.depth;
    search_time  = params.rec_time;
    start_time   = millis();
    can_shorten  = params.can_shorten;

    if(params.max_time){ // If a time has been set setup the timer (Under a second the timer will not have time to launch)
        #ifdef DEBUG
        printf("info string Search max time is: %d\n", params.max_time);
        #endif
        startTimerThread(params.max_time);
    } else if (params.depth != MAX_DEPTH){ // If we are in a depth based search we want to setup to print move 
        print_on_depth = TRUE;
    }

    start_search_threads(); // Launch Threads
}

/*
 * Called from the timer thread when the search times out
 */
void search_timed_out(void){
    if(best_move_found == FALSE){
        search_time = 0;
    }
    else if(run_get_best_move){
        stopSearchThreads();
        print_best_move = TRUE;
    }
    else{
        printf("info string Warning: Timer finished but no search is running or move is found!\n");
    }
}

/*
 * Stops the search and prints the best move
 * Called from the IO Thread
 */
void stopSearch(){
    stopTimerThread();
    stopSearchThreads();
}

/*
 * Resumes the helpers at the current depth and ecal
 */
static inline void resume_helpers(i32 depth, i32 eval){
    pthread_mutex_lock(&helper_lock);
    helpers_search_depth = depth;
    helper_eval = eval;
    do_helper_search = TRUE;
    pthread_cond_broadcast(&helper_cond);  // Change this line
    pthread_mutex_unlock(&helper_lock);
}

static inline void stop_helpers(){
    if(!helpers_run) return;
    helpers_run = FALSE;
    resume_helpers(0, MAX_DEPTH+1);
}

static inline void helper_wait(){ // TODO: abstract away in threads.h or something for windows
    pthread_mutex_lock(&helper_lock);
    do_helper_search = FALSE;
    while (!do_helper_search) pthread_cond_wait(&helper_cond, &helper_lock); // Block until the helpers are released
    pthread_mutex_unlock(&helper_lock);
}

static void helper_loop(Position* pos, Move* pv_array, KillerMoves* km, u32 thread_num){
    SearchStats stats;
    helpers_run = TRUE;
    helper_wait();
    while(helpers_run && helpers_search_depth + (thread_num % 3) <= search_depth){
        //printf("Helper thread searching at depth %d\n", helpers_search_depth + (thread_num % 3));
        helper_search_tree(*pos, helpers_search_depth + (thread_num % 3), pv_array, km, helper_eval, &stats, thread_num);
        helper_wait();
    }
    return;
}

/*
 * Loop Function for Search Threads
 */
i32 search_loop(u32 thread_num){
    #ifdef DEBUG
    printf("thread number is %d\n", thread_num);
    #endif
    // Set up local thread info
    Move pv_array[MAX_DEPTH] = {0};
    KillerMoves km = {0};

    if(search_depth == 0){
        printf("info string Warning search depth was 0\n");
        return -1;
    }

    u32 cur_depth = (1 + (thread_num % NUM_MAIN_THREADS)) % search_depth;
    u8 is_helper_thread = (thread_num >= NUM_MAIN_THREADS);

    Position search_pos = copy_global_position(); 

    if(is_helper_thread){ // If the thread is a helper thread enter the helper loop
        helper_loop(&search_pos, pv_array, &km, thread_num);
        goto exit_search_loop;
    }

    Move found_move[MAX_DEPTH] = {0};   // Arrays of the previous evals and moves found in search
    i32  found_eval[MAX_DEPTH] = {0};

    // Begin Search
    is_searching = TRUE;

    while(run_get_best_move && cur_depth <= search_depth){

        SearchStats stats; // Set up for iteration
        i32 avg_eval = 0;
        if(cur_depth >= 2) avg_eval = (found_eval[cur_depth-1] + found_eval[cur_depth-2]) / 2;
        TimePreference time_preference = NORMAL_TIME;

        if(cur_depth > MIN_HELPER_DEPTH) resume_helpers(cur_depth, avg_eval); // Run Search
        found_eval[cur_depth] = search_tree(search_pos, cur_depth, pv_array, &km, avg_eval, &stats, &time_preference);
        found_move[cur_depth] = pv_array[0];
        u8 updated = update_global_pv(cur_depth, pv_array, found_eval[cur_depth], stats);

        if(can_shorten && updated){ // Continue searching if we can't shorten or we didn't find a better move
            if( time_preference == HALT_TIME       ||   // If we should stop the search early
                found_eval[cur_depth] >= (CHECKMATE_VALUE-MAX_MOVES)){
                search_time = 0;
            }
            if(time_preference == NORMAL_TIME){        // In the case of normal time we look at the past moves/evals to see if we need to continue or we can stop
                if(cur_depth >= 7){
                    Move prev_move = NO_MOVE;
                    i32 move_changes = 0;
                    i32 min_eval = MAX_EVAL;
                    i32 max_eval = 0;
                    for(u32 i = cur_depth; i >= cur_depth - 6; i--){
                        if(found_eval[i] == 0 && found_move[i] == NO_MOVE) continue;
                        if(found_eval[i] > max_eval) max_eval = found_eval[i];
                        if(found_eval[i] < min_eval) min_eval = found_eval[i];
                        if(prev_move != found_move[i]) move_changes++;
                        prev_move = found_move[i];
                    }  
                    if( (move_changes == 0) || (abs(min_eval - max_eval) <= (PAWN_VALUE/8)))  time_preference = REDUCE_TIME;
                    if( (move_changes >  3) || (abs(min_eval - max_eval) >= (PAWN_VALUE)))    time_preference = EXTEND_TIME;
                }
            }
            if(time_preference == REDUCE_TIME){
                search_time = (u32)((double)search_time * SEARCH_REDUCTION_LEVEL);
            }
            if(time_preference == EXTEND_TIME){
                search_time = (u32)((double)search_time * SEARCH_EXTENSION_LEVEL);
            }
        }
        if(can_shorten && search_pos.stage == OPN_GAME){
            if(search_pos.fullmove_number <= 1)      search_time = 0;  // No time in the start TODO: make sure it matches the starting hash?
            else if(search_pos.fullmove_number == 2) search_time = MIN(10, search_time);
            else if(search_pos.fullmove_number == 3) search_time = MIN(100, search_time);
            else search_time -= (search_time/4);
        }

        printf("info string checking time %d >= %d\n", (u32)(millis() - start_time), (search_time) / 2);
        if(can_shorten && updated && ((u32)(millis() - start_time) >= (search_time) / 2) ){ // If over 50% of the time has elapsed we stop the search
            run_get_best_move = FALSE;
            print_best_move = TRUE;
            stopTimerThread();
            break;
        }

        cur_depth++;
    }
    stop_helpers();

    if(run_get_best_move && cur_depth > search_depth && print_on_depth){ // Print best move in the case we reached max depth
        print_on_depth = FALSE;
        print_best_move = TRUE;
    }
    #ifdef DEBUG
    printf("info string Completed search thread, freeing and exiting.\n");
    #endif

exit_search_loop:
    remove_hash_stack(&search_pos.hashStack);
    is_searching = FALSE;
    quit_thread();
    return 0;
}

//exit thread function
void exit_search(Position* pos){
   remove_hash_stack(&pos->hashStack);
   is_searching = FALSE;
   quit_thread();
}