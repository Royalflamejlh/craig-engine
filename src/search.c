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
#include "params.h"

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
_Atomic volatile i32 helper_eval;

_Atomic volatile u32 search_depth;
_Atomic volatile u32 search_time;
_Atomic volatile u64 start_time;


pthread_mutex_t helper_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t helper_cond = PTHREAD_COND_INITIALIZER;
int do_helper_search = FALSE;

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
    
    start_search_threads(); // Launch Threads

    if(params.max_time){ // If a time has been set setup the timer (Under a second the timer will not have time to launch)
        #ifdef DEBUG
        printf("info string Starting timer with max time: %d\n", params.max_time);
        #endif
        startTimerThread(params.max_time);
    } else if (params.depth != MAX_DEPTH){ // If we are in a depth based search we want to setup to print move 
        print_on_depth = TRUE;
    }
}

/*
 * Called from the timer thread when the search times out
 */
void search_timed_out(void){
    if(best_move_found == FALSE){
        #ifdef DEBUG_PRINT
        printf("info string Max time hit setting searchtime to 0\n");
        #endif
        search_time = 0;
    }
    else if(run_get_best_move){
        #ifdef DEBUG_PRINT
        printf("info string Max time hit stopping search\n");
        #endif
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
    #ifdef DEBUG_PRINT
    printf("info string Stop search called, stopping search and timer threads\n");
    #endif
    stopSearchThreads();
    stopTimerThread();
    #ifdef DEBUG_PRINT
    printf("info string Stop search completed, search and timer threads closed\n");
    #endif
}

/**
 * Call back function from search loop to cancel the thread
 */
void exit_search(){
    is_searching = FALSE;
    quit_thread();
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

/*
 * Function to update the search time for the search loop
 */
static void update_search_time(ThreadData *td, u8 updated){
    if(!can_shorten) return;

    // If we are have found a checkmate then we want to stop searching
    if(td->time_pref == HALT_TIME       ||  
        td->found_eval[td->depth] >= (CHECKMATE_VALUE-MAX_MOVES)){
        search_time = 0;
    }

    // If we have a good evaluation and we are in the endgame, extend to look for mate
    if(!updated && td->pos.stage == END_GAME && td->found_eval[td->depth] >= 5){
        td->time_pref = EXTEND_TIME;
    }

    // In the case of normal time we look at the past moves/evals to see if we need to continue or we can stop
    if(td->time_pref == NORMAL_TIME){        
        if(td->depth >= 7){
            Move prev_move = NO_MOVE;
            i32 move_changes = 0;
            i32 min_eval = MAX_EVAL;
            i32 max_eval = 0;
            for(u32 i = td->depth; i >= td->depth - 6; i--){
                if(td->found_eval[i] == 0 && td->found_move[i] == NO_MOVE) continue;
                if(td->found_eval[i] > max_eval) max_eval = td->found_eval[i];
                if(td->found_eval[i] < min_eval) min_eval = td->found_eval[i];
                if(prev_move != td->found_move[i]) move_changes++;
                prev_move = td->found_move[i];
            }  
            if( (move_changes == 0) || (abs(min_eval - max_eval) <= ( 100)))    td->time_pref = REDUCE_TIME;
            if( (move_changes >  3) || (abs(min_eval - max_eval) >= (1000)))    td->time_pref = EXTEND_TIME;
        }
    }

    // Update the search time.
    if(td->time_pref == REDUCE_TIME){
        search_time = (u32)((real64)search_time * SEARCH_REDUCTION_LEVEL);
    }
    if(td->time_pref == EXTEND_TIME){
        search_time = (u32)((real64)search_time * SEARCH_EXTENSION_LEVEL);
    }
}

i32 helper_loop(ThreadData *td){
    #ifdef DEBUG_PRINT
    printf("info string entered helper thread, number is %d\n", td->thread_num);
    #endif
    helpers_run = TRUE;
    helper_wait();
    u32 helper_depth = helpers_search_depth + (td->thread_num % 3);
    while(helpers_run && helper_depth <= search_depth){
        //printf("Helper thread searching at depth %d\n", helpers_search_depth + (thread_num % 3));
        helper_search_tree(td, helper_depth, helper_eval);
        helper_wait();
    }
    return 0;
}

/*
 * Loop Function for Search Threads
 */
i32 search_loop(ThreadData *td){
    #ifdef DEBUG_PRINT
    printf("info string thread number of search loop is %d\n", td->thread_num);
    #endif
    if(search_depth == 0){
        printf("info string Warning search depth was 0\n");
        return -1;
    }

    // Begin Search
    is_searching = TRUE;

    while(run_get_best_move && td->depth <= search_depth){
        if(td->depth >= 2) td->avg_eval = (td->found_eval[td->depth-1] + td->found_eval[td->depth-2]) / 2;
        if(td->depth > MIN_HELPER_DEPTH) resume_helpers(td->depth, td->avg_eval); // Run Search
        td->found_eval[td->depth] = search_tree(td);
        td->found_move[td->depth] = td->pv_array[0];
        u8 updated = update_global_pv(td->depth, td->pv_array, td->found_eval[td->depth], td->stats);

        update_search_time(td, updated);

        if(can_shorten && updated && ((u32)(millis() - start_time) >= (search_time) / 2) ){ // If over 50% of the time has elapsed we stop the search
            stopTimerThread();
            run_get_best_move = FALSE;
            print_best_move = TRUE;
            break;
        }
        td->depth++;
    }
    stop_helpers();

    if(run_get_best_move && td->depth > search_depth && print_on_depth){ // Print best move in the case we reached max depth
        print_on_depth = FALSE;
        print_best_move = TRUE;
    }
    #ifdef DEBUG_PRINT
    printf("info string Completed search thread, freeing and exiting.\n");
    #endif
    return 0;
}