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

#ifdef RUN_TEST
#include "tests/bbtests.h"
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
    debug_print_search = 1;
    testBB();
    #endif

    #ifdef __PROFILE
    printf("info string In profile mode, playing forever.\r\n");
    fflush(stdout);
    play_self();
    return 0;
    #endif

    #ifdef DEBUG
    debug_print_search = 1;
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