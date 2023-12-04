//
//  bitboard.c
//  godengine
//
//  Created by John Howard on 11/22/23.
//   Bit 0 = A1, Bit 63 = H8
//
//   A B C D E F G H
// 1 0 0 0 0 0 0 0 0
// 2 0 0 0 0 0 0 0 0
// 3 0 0 0 0 0 0 0 0
// 4 0 0 0 0 0 0 0 0
// 5 0 0 0 0 0 0 0 0
// 6 0 0 0 0 0 0 0 0
// 7 0 0 0 0 0 0 0 0
// 8 0 0 0 0 0 0 0 0


#include "bitboard.h"
#include "../util.h"
static void generateKingMoveMasks(void);
static void generateKnightMoveMasks(void);

static uint64_t kingMoves[64];
static uint64_t knightMoves[64];


void generateMasks(void){
    generateKingMoveMasks();
    generateKnightMoveMasks();
}


static void generateKingMoveMasks(void) {
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;

        kingMoves[square] = 0ULL;
        if (rank > 0) {
            kingMoves[square] |= 1ULL << (square - 8);
            if (file > 0) kingMoves[square] |= 1ULL << (square - 9);
            if (file < 7) kingMoves[square] |= 1ULL << (square - 7);
        }
        if (rank < 7) {
            kingMoves[square] |= 1ULL << (square + 8);
            if (file > 0) kingMoves[square] |= 1ULL << (square + 7); // Down-Left
            if (file < 7) kingMoves[square] |= 1ULL << (square + 9); // Down-Right
        }
        if (file > 0) kingMoves[square] |= 1ULL << (square - 1); // Left
        if (file < 7) kingMoves[square] |= 1ULL << (square + 1); // Right
    }
}

static void setKnightMove(int square, int r, int f) {
    if (r >= 0 && r < 8 && f >= 0 && f < 8)
        knightMoves[square] |= 1ULL << (r * 8 + f);
}

static void generateKnightMoveMasks(void) {
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;

        knightMoves[square] = 0ULL;

        setKnightMove(square, rank - 2, file - 1);
        setKnightMove(square, rank - 2, file + 1);
        setKnightMove(square, rank - 1, file - 2);
        setKnightMove(square, rank - 1, file + 2);
        setKnightMove(square, rank + 1, file - 2);
        setKnightMove(square, rank + 1, file + 2);
        setKnightMove(square, rank + 2, file - 1);
        setKnightMove(square, rank + 2, file + 1);
    }
}

static void generatePawnMoveMasks(void){
    return;
}

//TODO IMPLEMENT
uint64_t getBishopMoves(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces) {
    int temp = bishops + ownPieces + oppPieces;
    return temp;
}

//TODO IMPLEMENT
uint64_t getRookMoves(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces) {
    int temp = rooks + ownPieces + oppPieces;
    return temp;
}

//TODO IMPLEMENT
uint64_t getPawnMoves(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces) {
    int temp = pawns + ownPieces + oppPieces;
    return temp;
}

uint64_t getQueenMoves(uint64_t queens, uint64_t ownPieces, uint64_t oppPieces) {
    uint64_t rookMoves = getRookMoves(queens, ownPieces, oppPieces);
    uint64_t bishopMoves = getBishopMoves(queens, ownPieces, oppPieces);
    return rookMoves | bishopMoves;
}

uint64_t getKnightMoves(uint64_t knights, uint64_t ownPieces) {
    uint64_t moves = 0ULL;

    while (knights) {
        int square = __builtin_ctzll(knights);
        moves |= knightMoves[square] & ~ownPieces;
        knights &= knights - 1;
    }

    return moves;
}

uint64_t getKingMoves(uint64_t kings, uint64_t ownPieces) {
    int square = __builtin_ctzll(kings);
    return kingMoves[square] & ~ownPieces;
}




