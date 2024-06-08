#include <stdbool.h>
#include "search.h"
#include "threads.h"
#include "globals.h"
#include "tree.h"
#include "util.h"

volatile u8 is_searching; // Flag for if search loop is running

/*
* Starts the search threads
* Called from the IO Thread
*/
void startSearch(u32 time){
    is_searching = FALSE;
    startSearchThreads();
    if(time != 0){
        #ifdef DEBUG
        printf("info string Search time is: %d\n", time);
        #endif
        while(!is_searching); // Wait for search to begin
        startTimerThread(time);
    }
}

/*
 * Stops the search and prints the best move
 * Called from the IO Thread
 */
void stopSearch(){
    stopTimerThread();
    printBestMove(global_best_move);
    stopSearchThreads();
    is_searching = FALSE;
}

/*
 * Loop Function for Search Threads
 */
i32 searchLoop(){
    is_searching = TRUE;
    while(TRUE){
        if(global_position.hash && run_get_best_move){
            if(getBestMove(global_position, MAX_DEPTH)){
                return -1;
            }
        }
    }
    return 0;
}