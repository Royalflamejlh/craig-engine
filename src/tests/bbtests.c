//
//  bbtests.c
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#include "bbtests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../tree.h"
#include "../bitboard/bitboard.h"
#include "../bitboard/magic.h"
#include "../tree.h"
#include "../movement.h"
#include "../util.h"
#include "../hash.h"
#include "../transposition.h"
#include "../evaluator.h"
#include "../globals.h"
#include "../search.h"
#include "../moveorder.h"

#define MOVE_GEN_TEST
#define MOVE_MAKE_TEST
#define PERF_TEST
#define SEE_TEST
//#define MOVE_SORT_TEST
#define PUZZLE_TEST

i32 testBB(void) {
    #ifdef PYTHON
    python_init();
    #endif

    #ifdef MOVE_GEN_TEST
    FILE *file;
    char line[1024];
    Position pos;
    Move moveList[MAX_MOVES];
    i32 size, expectedMoves;
    
    printf("\n---------------------------------- MOVE GEN TESTING ----------------------------------\n\n");
    

    file = fopen("../perftsuite.epd", "r");
    if (file == NULL) {
        perror("Error opening file ../perftsuite.epd");
        return -1;
    }
    while (fgets(line, sizeof(line), file)) {
        char *fenEnd = strchr(line, ';');
        if (fenEnd) *fenEnd = '\0';
        char *fen = line;
        pos = fen_to_position(fen);
        if (fenEnd) *fenEnd = ';'; 

        char *token;
        while ((token = strtok(fenEnd ? fenEnd + 1 : NULL, " ;")) != NULL) {
            if (token[0] == 'D' && token[1] == '1') {
                expectedMoves = atoi(token + 3);
                size = generateLegalMoves(&pos, moveList);
                if (size != expectedMoves) {
                    printf("Failed to get correct amount of moves for Position %s, correct: %d, found: %d\n", fen, expectedMoves, size);
                    printPosition(pos, TRUE);
                    for (i32 i = 0; i < size; i++) {
                        printMove(moveList[i]);
                    }
                    while(1);
                    return -1;
                }
                break;
            }
        }
    }
    fclose(file);
    printf("Finished Quick Movegen Check \n");
    #endif

    #ifdef MOVE_GEN_TEST
    printf("\n---------------------------------- MOVE MAKING TESTING ----------------------------------\n\n");
    time_t t;
    srand((unsigned) time(&t));

    
    printf("Starting Move Making Check\n");
    Move threatMoveList[MAX_MOVES];
    i32 threatSize;
    for(i32 j = 0; j < 100; j++){
        char* FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        pos = fen_to_position(FEN);
        size = generateLegalMoves(&pos, moveList);
        while(size != 0 && pos.halfmove_clock < 50){
            i32 randMove = rand() % size;
            make_move(&pos, NULL, moveList[randMove]);
            size = generateLegalMoves(&pos, moveList);
            threatSize = generateThreatMoves(&pos, threatMoveList);
            for(i32 k = 0; k < threatSize; k++){
                char found = 0;
                for(i32 l = 0; l < size; l++){
                    if(threatMoveList[k] == moveList[l]) found = 1;
                }
                if(!found){
                    printf("Move in threat moves, that is not in move list!\n");
                    return -1;
                }
            }
        }
    }
    printf("Move Making Check Complete.\n");

    #endif


    #ifdef PERF_TEST
    printf("\n---------------------------------- PERFT TESTING ----------------------------------\n\n");
    int perft_depth = 5;
    printf("Running perft suite to depth: %d\n", perft_depth);
    file = fopen("../perftsuite.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    ThreadData temp_td = {0};
    while (fgets(line, sizeof(line), file)) {
        char *fen = line;
        temp_td.pos = fen_to_position(fen);
        for(int depth = 1; depth < perft_depth; depth++){
            i64 num_moves = perft(&temp_td, depth, FALSE);
            i64 real_num_moves = get_perft_fen_depth(fen, depth);
            if(real_num_moves == -1) continue;
            if(num_moves != real_num_moves){
                printf("Incorrect number of moves found for fen: \n %s \n", fen);
                printf("Found %" PRIi64 " moves, expected %" PRIi64 " moves at depth %d \n", num_moves, real_num_moves, depth);
                fflush(stdout);
                while(1);
            }
        }
        printf(".");
        fflush(stdout);
    }

    printf("\nPerft Suite Completed Successfully!\n");

    fclose(file);
    #endif


    #ifdef MOVE_SORT_TEST
    printf("\n----------------------------- MOVE EVAL TESTING ------------------------------\n\n");

    ThreadData me_td = {0};
    me_td.pos = get_random_position();
    Move me_move_list[MAX_MOVES] = {0};
    i32 me_move_vals[MAX_MOVES] = {0};
    u16 me_moves = generateLegalMoves(&me_td.pos, me_move_list);
    eval_movelist(&me_td.pos, me_move_list, me_move_vals, me_moves);

    remove_hash_stack(&pos.hashStack);
    
    pos = fen_to_position(START_FEN);
    Move best_move = getBestMove(pos);
    while(best_move != NO_MOVE){
        printf("Best move found to be: \n");
        printMove(best_move);
        printf("Press Enter to Continue\n");
        while( getchar() != '\n' && getchar() != '\r');
        make_move(&pos, best_move);
        printf("Pos after move: \n");
        printPosition(pos, FALSE);
        best_move = getBestMove(pos);
    }

    remove_hash_stack(&pos.hashStack);
    #endif


    #ifdef PUZZLE_TEST
    printf("\n--------------------------------- PUZZLE TESTING ----------------------------------\n\n");

    printf("\nRunning ERET puzzles.\n");

    file = fopen("../puzzles/ERET.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    run_get_best_move = TRUE;

    while (fgets(line, sizeof(line), file)) {
        char *fen = line;
        Position puzzle_pos = fen_to_position(fen);
        char* move_str = get_move_from_epd_line(fen);
        if(!move_str){
            printf("Couldn't get move string from fen: \n %s \n", fen);
            while(1);
            continue;
        }
        Move correct_move = move_from_str_alg(move_str, &puzzle_pos);
        if(correct_move == NO_MOVE){
            printf("Move was not found from move string %s\n and fen %s \n", move_str, fen);
            free(move_str);
            while(1);
            continue;
        }
        free(move_str);

        set_global_position(puzzle_pos);
        SearchParameters sp = {0};
        sp.depth = MAX_DEPTH - 1;
        sp.can_shorten = FALSE;

        start_search(sp);
        sleep(5);
        stopSearch();

        //printPosition(fen_to_position(fen), FALSE);
        printf(fen);
        printf("Best move found to be: ");
        printMove(get_global_best_move());
        printf("\nBest move is actually: ");
        printMove(correct_move);
        printf("\n");
    }

    printf("\nPuzzle Tests Complete\n");

    fclose(file);

    #endif

    #ifdef SEE_TEST
    printf("\n---------------------------------- SEE TESTING ------------------------------------\n\n");
    Position testpos = fen_to_position("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -");
    i32 val = see(&testpos, E5, BLACK_PAWN, E1, WHITE_ROOK);
    if(val < 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    }

    testpos = fen_to_position("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -");
    val = see(&testpos, E5, BLACK_PAWN, D3, WHITE_KNIGHT);
    if(val > 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    }

    testpos = fen_to_position("1k1r3q/1ppn3p/p4b2/4P3/8/P2N2P1/1PP1R1BP/2K1Q3 b - -");
    val = see(&testpos, E5, WHITE_PAWN, D7, BLACK_KNIGHT);
        if(val > 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    }

    testpos = fen_to_position("rnkr3b/6pp/2p1b3/p3p1N1/2q1N3/1R3Q2/P1PPPP1P/2BKR3 w - -");
    val = see(&testpos, E6, BLACK_BISHOP, G5, WHITE_KNIGHT);
    if(val < 0){
        printf("SEE returned incorrect exchange winner (see val: %d) at position: \n", val);
        printPosition(testpos, TRUE);
        while(1);
    } 

    printf("Static exchange tests passed!\n");
    #endif //SEE_TEST

    #ifdef PYTHON
    python_close();
    #endif

    printf("\n--------------------------------------------------------------------------\n");

    return 0;
}
