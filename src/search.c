#include <stdbool.h>
#include "search.h"
#include "threads.h"
#include "globals.h"
#include "tree.h"
#include "tables.h"
#include "types.h"
#include "util.h"

#ifdef DEBUG
#include <stdio.h>
#endif

volatile u8 is_searching; // Flag for if search loop is running
/*
* Starts the search threads
* Called from the IO Thread
*/
void startSearch(u32 time){
    is_searching = FALSE; // Set up new search
    clearKillerMoves();
    
    startSearchThreads(); // Launch Threads
    if(time){
        #ifdef DEBUG
        printf("info string Search time is: %d\n", time);
        #endif
        while(!is_searching); // Wait for search to begin in at least one thread
        startTimerThread(time);
    }
}

/*
 * Stops the search and prints the best move
 * Called from the IO Thread
 */
void stopSearch(){
    stopTimerThread();
    stopSearchThreads();
    is_searching = FALSE;
}

/*
 * Loop Function for Search Threads
 */
i32 searchLoop(){
    // Set up local thread info
    Move *pvArray = calloc((MAX_DEPTH*MAX_DEPTH + MAX_DEPTH)/2, sizeof(Move));

    Position searchPosition = get_global_position();

    // Begin Search
    is_searching = TRUE;

    u32 cur_depth = 1;
    i32 eval = 0, eval_prev = 0;

    while(run_get_best_move && cur_depth <= MAX_DEPTH){
        SearchStats stats;
        eval = searchTree(searchPosition, cur_depth, pvArray, eval_prev, &stats);
        update_global_pv(cur_depth, pvArray, eval, stats);

        // Update for next iteration
        eval_prev = eval;
        cur_depth++;
    }

    // Free Thread Data
    free(pvArray);
    return 0;
}