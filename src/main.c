#include "bitboard/bbutils.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include "transposition.h"
#include "bitboard/bitboard.h"
#include "bitboard/magic.h"
#include "hash.h"
#include "evaluator.h"
#include "globals.h"
#include "threads.h"
#include "types.h"

#ifdef __COMPILE_DEBUG
#define RUN_TEST
#elif defined(__PROFILE)
void playSelfInfinite(void);
#endif


/*
* Meet the Globals
*/
volatile Position global_position;
volatile i32 run_get_best_move;
volatile Move global_best_move;

#ifdef __PROFILE
void playSelfInfinite(void){
    global_position = fenToPosition(START_FEN);
    global_best_move = NO_MOVE;
    run_get_best_move = TRUE;
    Move moveList[MAX_MOVES];
    while(generateLegalMoves(global_position, moveList)){
        getBestMove(global_position, MAX_DEPTH);
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