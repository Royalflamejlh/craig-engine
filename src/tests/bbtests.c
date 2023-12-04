//
//  bbtests.c
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#include "bbtests.h"
#include "../bitboard/bitboard.h"
#include "../bitboard/magic.h"

void testBB(void) {
    generateMasks();
    generateMagics();
    /**
    printf("Testing Starting Board Position\n");
    char* FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Position pos = fenToPosition(FEN);
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

    printf("Knight moves for white after E4 C5 NF3\n");
    uint64_t moves = getKnightMoves(pos.w_knight, pos.white);
    printBB(moves);

    printf("Rook attacks from A1\n");
    printBB(rookAttacks(0, 1));
    
    printf("Bishop attacks from A1\n");
    printBB(bishopAttacks(0, 1));
    */
    return;
    
}
