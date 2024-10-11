#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include "bitboard/bbutils.h"
#include "search.h"
#include "transposition.h"
#include "bitboard/bitboard.h"
#include "bitboard/magic.h"
#include "hash.h"
#include "evaluator.h"
#include "globals.h"
#include "threads.h"
#include "types.h"
#include "tree.h"
#include "util.h"
#include "masks.h"

#ifdef DEBUG
#define RUN_TEST
#include "tests/bbtests.h"
#elif defined(__PROFILE)
void playSelfInfinite(void);
#endif


#ifdef __PROFILE
void playSelfInfinite(void){

    Position tempPos = get_global_position();
    
    while(generateLegalMoves(tempPos, moveList)){
        make_move(&tempPos, NULL, get_global_best_move());
        set_global_position(tempPos);
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
    init_pst();
    if(init_tt(2)){
        printf("info string Warning failed to create transposition table, exiting.\n");
        return -1;
    }
    init_masks();
    init_globals();

    printf("info string Finished start up!\n");

    #ifdef RUN_TEST
    testBB();
    #endif

    #ifdef __PROFILE
    printf("info string In profile mode, playing forever.\r\n");
    fflush(stdout);
    playSelfInfinite();
    #endif  

    launch_threads();
    stopSearch();
    printf("info string All threads have finished.\n");
    free_globals();
    tt_free();
    printf("info string All memory freed\n");
    printf("info string Goodbye! :)\n");
    return 0;
}