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
#include "../bitboard/bitboard.h"
#include "../bitboard/magic.h"
#include "../tree.h"
#include "../movement.h"
#include "../util.h"
#include "../hash.h"
#include "../transposition.h"
#include "../evaluator.h"
#include "../globals.h"

#define MOVE_GEN_TEST
#define MOVE_MAKE_TEST
#define PERF_TEST
//#define SEE_TEST
//#define PUZZLE_TEST

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
    

    file = fopen("perftsuite.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
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
                    return -1;
                }
                break;
            }
        }

        remove_hash_stack(&pos.hashStack);
    }

    fclose(file);

    printf("Finished Depth 1 Position Check \n");
    #endif

    #ifdef MOVE_GEN_TEST
    printf("\n---------------------------------- MOVE MAKING TESTING ----------------------------------\n\n");
    time_t t;
    srand((unsigned) time(&t));

    
    printf("Starting Quick Check\n");
    Move threatMoveList[MAX_MOVES];
    i32 threatSize;
    for(i32 j = 0; j < 100; j++){
        char* FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        pos = fen_to_position(FEN);
        size = generateLegalMoves(&pos, moveList);
        while(size != 0 && pos.halfmove_clock < 50){
            i32 randMove = rand() % size;
            makeMove(&pos, moveList[randMove]);
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
        remove_hash_stack(&pos.hashStack);
    }
    printf("Check Complete.\n");

    #endif


    #ifdef PERF_TEST
    printf("\n---------------------------------- PERFT TESTING ----------------------------------\n\n");

    printf("Perft from default position:\n");
    pos = fen_to_position(START_FEN);
    for(i32 depth = 1; depth < 4; depth++){
        u64 num_moves = perft(depth, pos);
        printf("Perft output is %ld for depth %d\n", (long)num_moves, depth);
    }
    remove_hash_stack(&pos.hashStack);

    printf("\nComplete, running perft suite.\n");

    file = fopen("perftsuite.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    while (fgets(line, sizeof(line), file)) {
        char *fen = line;
        pos = fen_to_position(fen);
        //printf("Testing: %s", fen);
        for(i32 depth = 1; depth < 2; depth++){
            perft(depth, pos);
            //i64 num_moves = perft(depth, pos);
            //printf("D%d: %lld |", depth, (long long i32)num_moves);
        }
        remove_hash_stack(&pos.hashStack);
        //printf("\n\n");
    }
    printf("\nPerft Suite Complete\n");

    fclose(file);
    #endif


    #ifdef NODE_TEST
    printf("\n---------------------------------- NODE TESTING ----------------------------------\n\n");

    pos = fen_to_position("START_FEN");
    printPosition(pos, FALSE);

    Move moveListNode[MAX_MOVES];
    i32 sizeNode = 0;
    i32 moveVals[MAX_MOVES] = {0};
    sizeNode = generateLegalMoves(pos, moveListNode);
    evalMoves(moveListNode, moveVals, sizeNode, NO_MOVE, NULL, 0, pos);
    for(i32 i = 0; i < sizeNode; i++){
        printf("Move found with move value of %d:\n", moveVals[i]);
        printMove(moveListNode[i]);
    }
    printf("\n----------------------------- SELECT SORT TESTING ------------------------------\n\n");
    for (i32 i = 0; i < sizeNode; i++)  {
        select_sort(i, moveListNode, moveVals, sizeNode);
        printf("Move with value %d selected at pos %d\n", moveVals[i], i);
        printMove(moveListNode[i]);
    }
    printf("\n--------------------------------------------------------------------------\n");

    remove_hash_stack(&pos.hashStack);
    
    pos = fen_to_position(START_FEN);
    Move best_move = getBestMove(pos);
    while(best_move != NO_MOVE){
        printf("Best move found to be: \n");
        printMove(best_move);
        printf("Press Enter to Continue\n");
        while( getchar() != '\n' && getchar() != '\r');
        makeMove(&pos, best_move);
        printf("Pos after move: \n");
        printPosition(pos, FALSE);
        best_move = getBestMove(pos);
    }

    remove_hash_stack(&pos.hashStack);
    #endif

    #ifdef HASH_TEST
    printf("\n---------------------------------- HASH TESTING ----------------------------------\n\n");

    pos = fen_to_position(START_FEN);
    printf("Hash %d is: %" PRIu64 "\n", 0, pos.hash);
    Move moveList_hash[MAX_MOVES];
    i32 size_hash = 0;
    i32 moveVals[MAX_MOVES] = {0};
    

    for (i32 i = 0; i < 100; i++)  {
        size_hash = generateLegalMoves(pos, moveList_hash);
        evalMoves(moveList_hash, moveVals, size_hash, NO_MOVE, NULL, 0, pos);
        select_sort(0, moveList_hash, moveVals, size_hash);
        makeMove(&pos, moveList_hash[0]);
        printf("Hash %d is: %" PRIu64 "\n", i+1, pos.hash);
    }

    printf("Now going through Hash Table: (Size : %d) \n", pos.hashStack.current_idx);
    for(i32 i = 0; i < pos.hashStack.current_idx; i++){
        printf("HashTable[%d] : %" PRIu64 "\n", i, pos.hashStack.ptr[i]);
    }

    printf("Printing Hash Table Since Last Unique Move (at: %d): \n", pos.hashStack.last_reset_idx);
    for(i32 i = pos.hashStack.last_reset_idx; i != pos.hashStack.current_idx; i = (i+1) % HASHSTACK_SIZE){
        printf("HashTable[%d] : %" PRIu64 "\n", i, pos.hashStack.ptr[i]);
    }

    remove_hash_stack(&pos.hashStack);
    #endif

    #ifdef PUZZLE_TEST
    #ifdef DEBUG
    printf("\n--------------------------------- PUZZLE TESTING ----------------------------------\n\n");

    printf("\nRunning ERET puzzles.\n");

    file = fopen("puzzles/ERET.epd", "r");
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    run_get_best_move = TRUE;

    while (fgets(line, sizeof(line), file)) {
        char *fen = line;
        pos = fen_to_position(fen);
        getBestMove(pos, 5);
        Move best_move = global_best_move;
        remove_hash_stack(&pos.hashStack);

        printPosition(pos, FALSE);
        printf(fen);
        printf("\nBest move is: ");
        printMove(best_move);
        printf("\n");
        printf("Press Enter to Continue\n");
        while( getchar() != '\n' && getchar() != '\r');
    }

    run_get_best_move = FALSE;
    global_best_move = NO_MOVE;
    printf("\nPuzzle Tests Complete\n");

    fclose(file);

    #endif //Debug
    #endif //Puzzle Test

    #ifdef SEE_TEST
    printf("\n---------------------------------- SEE TESTING ------------------------------------\n\n");
    Position testpos = fen_to_position("1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - -");
    i32 val = see(testpos, E5, BLACK_PAWN, E1, WHITE_ROOK);
    printf("See val: %d\n", val);

    testpos = fen_to_position("1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - -");
    val = see(testpos, E5, BLACK_PAWN, D3, WHITE_KNIGHT);
    printf("See val: %d\n", val);

    testpos = fen_to_position("1k1r3q/1ppn3p/p4b2/4P3/8/P2N2P1/1PP1R1BP/2K1Q3 b - -");
    val = see(testpos, E5, WHITE_PAWN, D7, BLACK_KNIGHT);
    printf("See val: %d\n", val);

    testpos = fen_to_position("rnkr3b/6pp/2p1b3/p3p1N1/2q1N3/1R3Q2/P1PPPP1P/2BKR3 w - -");
    val = see(testpos, E6, BLACK_BISHOP, G5, WHITE_KNIGHT);
    printf("See val: %d\n", val);

    

    printf("Press Enter to Continue\n");
    while( getchar() != '\n' && getchar() != '\r');
    #endif //SEE_TEST

    #ifdef PYTHON
    python_close();
    #endif

    return 0;
}
