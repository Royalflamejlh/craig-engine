#include <stdbool.h>
#include "search.h"
#include "threads.h"
#include "globals.h"
#include "tree.h"
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

// Search Parameters
_Atomic volatile u8  helpers_run;
_Atomic volatile u32 helpers_search_depth;
_Atomic volatile u32 helper_eval;

_Atomic volatile u32 search_depth;
_Atomic volatile u8  print_on_depth;


pthread_mutex_t helper_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t helper_cond = PTHREAD_COND_INITIALIZER;
int do_helper_search = FALSE;


/*
* Starts the search threads
* Passed the max time and the max depth for the search
* Called from the IO Thread
*/
void start_search(SearchParameters params){
    is_searching = FALSE; // Set up new search
    print_on_depth = FALSE;
    search_depth = params.depth;
    
    start_search_threads(); // Launch Threads

    if(params.max_time){ // If a time has been set setup the timer
        #ifdef DEBUG
        printf("info string Search max time is: %d\n", params.max_time);
        #endif
        while(!is_searching); // Wait for search to begin in at least one thread
        startTimerThread(params.max_time);
    }
    else if (params.depth != MAX_DEPTH){ // If we are in a depth based search we want to setup to print move 
        print_on_depth = TRUE;
    }
}

/*
 * Called from the timer thread when the search times out
 */
void search_timed_out(void){
    while(best_move_found == FALSE);
    stopSearchThreads();
    print_best_move = TRUE;
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
    i32 eval, prev_eval;
    eval = prev_eval = 0;

    u32 cur_depth = (1 + (thread_num % NUM_MAIN_THREADS)) % search_depth;
    u8 is_helper_thread = (thread_num >= NUM_MAIN_THREADS);

    Position search_pos = copy_global_position(); // TODO: Copy global position hashtable to new hashtable

    if(is_helper_thread){ // If the thread is a helper thread enter the helper loop
        helper_loop(&search_pos, pv_array, &km, thread_num);
        goto exit_search_loop;
    }

    // Begin Search
    is_searching = TRUE;

    while(run_get_best_move && cur_depth <= search_depth){
        SearchStats stats;
        
        i32 avg_eval = (eval + prev_eval) / 2;
        prev_eval = eval;

        if(cur_depth > MIN_HELPER_DEPTH) resume_helpers(cur_depth, avg_eval);
        eval = search_tree(search_pos, cur_depth, pv_array, &km, avg_eval, &stats);
        update_global_pv(cur_depth, pv_array, eval, stats);
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

/*
 * Loop Function for Helper Threads
 */
i32 startHelpers(Position pos, i32 eval, u32 depth);