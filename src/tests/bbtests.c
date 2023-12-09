//
//  bbtests.c
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#include "bbtests.h"
#include "../bitboard/bitboard.h"
#include "../bitboard/magic.h"
#include "../movement.h"
#include "../util.h"

#define MOVE_GEN_TEST


#define POSITION1 "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define P1_MOVES_D1 20

#define POSITION2 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - "
#define P2_MOVES_D1 48

#define POSITION3 "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - "
#define P3_MOVES_D1 14

#define POSITION4 "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1"
#define P4_MOVES_D1 6

#define POSITION41 "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1 "
#define P41_MOVES_D1 6

#define POSITION5 "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  "
#define P5_MOVES_D1 44

#define POSITION6 "r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2"
#define P6_MOVES_D1 8

#define POSITION7 "8/8/8/2k5/2pP4/8/B7/4K3 b - d3 0 3"
#define P7_MOVES_D1 8

#define POSITION8 "r1bqkbnr/pppppppp/n7/8/8/P7/1PPPPPPP/RNBQKBNR w KQkq - 2 2"
#define P8_MOVES_D1 19

#define POSITION9 "r3k2r/p1pp1pb1/bn2Qnp1/2qPN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQkq - 3 2"
#define P9_MOVES_D1 5

#define POSITION10 "2kr3r/p1ppqpb1/bn2Qnp1/3PN3/1p2P3/2N5/PPPBBPPP/R3K2R b KQ - 3 2"
#define P10_MOVES_D1 44

#define POSITION11 "rnb2k1r/pp1Pbppp/2p5/q7/2B5/8/PPPQNnPP/RNB1K2R w KQ - 3 9"
#define P11_MOVES_D1 39

#define POSITION12 "2r5/3pk3/8/2P5/8/2K5/8/8 w - - 5 4"
#define P12_MOVES_D1 9

void testBB(void) {
    generateMasks();
    generateMagics();
    char* FEN;
    Position pos;
    



    #ifdef FENTEST
    printf("\n----------------------------------\n");

    printf("Testing Starting Board Position\n");
    FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos = fenToPosition(FEN);
    printPosition(pos);

    printf("After E4\n");
    FEN = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    pos = fenToPosition(FEN);
    printPosition(pos);

    printf("After E4 C5\n");
    FEN = "rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2";
    pos = fenToPosition(FEN);
    printPosition(pos);

    printf("After E4 C5 NF3\n");
    FEN = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
    pos = fenToPosition(FEN);
    printPosition(pos);
    #endif




    #ifdef MOVE_MASK_TEST
    printf("\n----------------------------------\n");

    uint64_t moves;
    FEN = "rn1qkbnr/pp1bppp1/2p5/3p2Pp/3P4/3BPN2/PPP2P1P/RNBQK2R w KQkq h6 0 7";
    pos = fenToPosition(FEN);
    //TODO: Add checks for moves
    #endif




    #ifdef MOVE_GEN_TEST
    Move moveList[256];
    int size;
    
    printf("\n----------------------------------\n");
    
    pos = fenToPosition(POSITION1);
    size = generateLegalMoves(pos, moveList);
    if(size != P1_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 1, correct: %d, found: %d\n", P1_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }
    
    pos = fenToPosition(POSITION2);
    size = generateLegalMoves(pos, moveList);
    if(size != P2_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 2, correct: %d, found: %d\n", P2_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION3);
    size = generateLegalMoves(pos, moveList);
    if(size != P3_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 3, correct: %d, found: %d\n", P3_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION4);
    size = generateLegalMoves(pos, moveList);
    if(size != P4_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 4, correct: %d, found: %d\n", P4_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }
    

    pos = fenToPosition(POSITION41);
    size = generateLegalMoves(pos, moveList);
    if(size != P41_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 4.1, correct: %d, found: %d\n", P41_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }
    
    pos = fenToPosition(POSITION5);
    size = generateLegalMoves(pos, moveList);
    if(size != P5_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 5, correct: %d, found: %d\n", P5_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }
    /*
    pos = fenToPosition(POSITION6);
    size = generateLegalMoves(pos, moveList);
    if(size != P6_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 6, correct: %d, found: %d\n", P6_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION7);
    size = generateLegalMoves(pos, moveList);
    if(size != P7_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 7, correct: %d, found: %d\n", P7_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION8);
    size = generateLegalMoves(pos, moveList);
    if(size != P8_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 8, correct: %d, found: %d\n", P8_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION9);
    size = generateLegalMoves(pos, moveList);
    if(size != P9_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 9, correct: %d, found: %d\n", P9_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION10);
    size = generateLegalMoves(pos, moveList);
    if(size != P10_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 9, correct: %d, found: %d\n", P10_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION11);
    size = generateLegalMoves(pos, moveList);
    if(size != P11_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 9, correct: %d, found: %d\n", P11_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }

    pos = fenToPosition(POSITION12);
    size = generateLegalMoves(pos, moveList);
    if(size != P12_MOVES_D1){
        printf("Failed to get correct amount of moves for Position 9, correct: %d, found: %d\n", P12_MOVES_D1, size);
        printPosition(pos);
        for(int i = 0; i < size; i++){
            printMove(moveList[i]);
        }
    }
    */

    printf("Finished Depth 1 Position Check \n");
    #endif




    #ifdef PERF_TEST
    printf("\n----------------------------------\n");

    FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos = fenToPosition(FEN);
    for(int depth = 1; depth < 5; depth++){
        uint64_t num_moves = perft(depth, pos);
        printf("Perft output is %lld for depth %d\n", (long long unsigned)num_moves, depth);
    }
    #endif
    


    

    
    
    return;
    
}
