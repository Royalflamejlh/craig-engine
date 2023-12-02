//
//  bbtests.c
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#include "bbtests.h"

void testBB(void) {
    position initialBB = getInitialBB();
    
    // Test with an empty board
    uint64_t emptyBoard = 0;
    printf("Empty Board:\n");
    printBB(emptyBoard);
    printf("\n");

    // Test with a single piece at A1 (bit 0)
    uint64_t a1Board = 1ULL; // Bit 0 set
    printf("Board with piece at A1:\n");
    printBB(a1Board);
    printf("\n");

    // Test with a single piece at H8 (bit 63)
    uint64_t h8Board = 1ULL << 63; // Bit 63 set
    printf("Board with piece at H8:\n");
    printBB(h8Board);
    printf("\n");

    // Test with pieces on A1, B2, C3... (diagonal)
    uint64_t diagonalBoard = 0;
    for (int i = 0; i < 8; i++) {
        diagonalBoard |= 1ULL << (i * 8 + i);
    }
    printf("Board with pieces on diagonal A1, B2, C3, ... H8:\n");
    printBB(diagonalBoard);
    printf("\n");
    
    //Test getting knight moves
    printf("Board with knight moves for white\r\n");
    uint64_t knightBoard = getKnightMoves(initialBB.knight, initialBB.white);
    printBB(knightBoard);
    
    //Test getting pawn moves
    printf("Gettings pawn moves for white\r\n");
    
}
