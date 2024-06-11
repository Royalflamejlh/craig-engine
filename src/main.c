#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include "bitboard/bbutils.h"
#include "transposition.h"
#include "bitboard/bitboard.h"
#include "bitboard/magic.h"
#include "hash.h"
#include "evaluator.h"
#include "globals.h"
#include "threads.h"
#include "types.h"
#include "util.h"

#ifdef __COMPILE_DEBUG
#define RUN_TEST
#include "tests/bbtests.h"
#elif defined(__PROFILE)
void playSelfInfinite(void);
#endif


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
    init_globals();

    #ifdef RUN_TEST
    testBB();
    #endif

    #ifdef __PROFILE
    printf("info string In profile mode, playing forever.\r\n");
    fflush(stdout);
    playSelfInfinite();
    #endif  

    launch_threads();

    printf("info string All threads have finished.\n");
    free_globals();
    freeTT();
    printf("info string All memory freed\n");
    printf("info string Goodbye! :)\n");
    return 0;
}