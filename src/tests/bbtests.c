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

void testBB(void) {
    generateMasks();
    generateMagics();
    char* FEN;
    Position pos;
    
    #ifdef FENTEST
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
    uint64_t moves;
    FEN = "rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2";
    pos = fenToPosition(FEN);

    printf("Knight moves for white after E4 C5 NF3\n");
    moves = getKnightMoves(pos.w_knight, pos.white);
    printBB(moves);

    printf("Pawn moves for white after E4 C5 NF3 \n");
    moves = getPawnMoves(pos.w_pawn, pos.white, pos.black,  pos.en_passant, WHITE_TURN);
    printBB(moves);

    printf("Pawn moves for black after E4 C5 NF3 \n");
    moves = getPawnMoves(pos.b_pawn, pos.black, pos.white, pos.en_passant, 0);
    printBB(moves);
    #endif

    #ifdef MOVE_GEN_TEST
    FEN = "rn1qkbnr/pp1bppp1/2p5/3p2Pp/3P4/3BPN2/PPP2P1P/RNBQK2R w KQkq h6 0 7";
    pos = fenToPosition(FEN);
    printf("Testing a position with en-passant and castling\n");
    printPosition(pos);
    printf("Visual bishop moves \n");
    printBB(getBishopMoves(pos.w_bishop, pos.white, pos.black));
    Move moveList[256];
    int size;
    generateLegalMoves(pos, moveList, &size);
    for(int i = 0; i < size; i++){
        printMove(moveList[i]);
    }
    #endif

    FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    pos = fenToPosition(FEN);
    for(int depth = 1; depth < 5; depth++){
        uint64_t num_moves = perft(depth, pos);
        printf("Perft output is %lld for depth %d\n", (long long unsigned)num_moves, depth);
    }
    


    

    
    
    return;
    
}
