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
#include "magic.h"
#include "../util.h"

#define W_SHORT_CASTLE_MASK 0x0000000000000060ULL
#define W_LONG_CASTLE_MASK  0x000000000000000EULL
#define B_SHORT_CASTLE_MASK 0x6000000000000000ULL 
#define B_LONG_CASTLE_MASK  0x0E00000000000000ULL

static void generateKingMoveMasks(void);
static void generateKnightMoveMasks(void);
static void generatePawnMoveMasks(void);

static uint64_t kingMoves[64];
static uint64_t knightMoves[64];
static uint64_t pawnMoves[64][4];

void generateMasks(void){
    generateKingMoveMasks();
    generateKnightMoveMasks();
    generatePawnMoveMasks();
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

void generatePawnMoveMasks(void){
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;

        uint64_t singleMove = 0ULL, doubleMove = 0ULL, leftCapture = 0ULL, rightCapture = 0ULL;
        uint64_t position = setBit(0ULL, square);

        // Single square forward
        if (rank < 7) {
            singleMove = northOne(position);
        }
        pawnMoves[square][0] = singleMove;

        // Double square forward
        if (rank == 1) {
            doubleMove = northTwo(position);
        }
        pawnMoves[square][1] = doubleMove;

        // Capture left
        if (file > 0 && rank < 7) {
            leftCapture = noWeOne(position);
        }
        pawnMoves[square][2] = leftCapture;

        // Capture right
        if (file < 7 && rank < 7) {
            rightCapture = noEaOne(position);
        }
        pawnMoves[square][3] = rightCapture;
    }
}

//Bishop
uint64_t getBishopMoves(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces) {
    uint64_t moves = 0ULL;
    while (bishops) {
        int square = __builtin_ctzll(bishops);
        moves |= bishopAttacks(ownPieces | oppPieces, square);
        bishops &= bishops - 1;
    }
    return moves & ~ownPieces;
}

uint64_t getBishopMovesAppend(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    while (bishops) {
        int square = __builtin_ctzll(bishops);
        uint64_t moves = bishopAttacks(ownPieces | oppPieces, square) & ~ownPieces;
        all_moves |= moves;
        while(moves){
            int move_sq = __builtin_ctzll(moves);
            moveList[*idx] = SET_MOVE(square, move_sq);
            (*idx)++;
            moves &= moves - 1;
        }
        bishops &= bishops - 1;
    }
    return all_moves;
}

//Rook
uint64_t getRookMoves(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces) {
    uint64_t moves = 0ULL;
    while (rooks) {
        int square = __builtin_ctzll(rooks);
        moves |= rookAttacks(ownPieces | oppPieces, square);
        rooks &= rooks - 1;
    }
    return moves & ~ownPieces;
}

uint64_t getRookMovesAppend(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    while (rooks) {
        int square = __builtin_ctzll(rooks);
        uint64_t moves = rookAttacks(ownPieces | oppPieces, square) & ~ownPieces;
        all_moves |= moves;
        while(moves){
            int move_sq = __builtin_ctzll(moves);
            moveList[*idx] = SET_MOVE(square, move_sq);
            (*idx)++;
            moves &= moves - 1;
        }
        rooks &= rooks - 1;
    }
    return all_moves;
}


//Pawn
uint64_t getWhitePawnMoves(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant) {
    uint64_t moves = 0ULL;
    uint64_t occ =  0ULL;

    while (pawns) {
        int square = __builtin_ctzll(pawns);
        int rank = square / 8;

        occ = (oppPieces | ownPieces) & pawnMoves[square][0];
        if(!occ) moves |= pawnMoves[square][0];

        if(occ == 0 && rank == 1){
            occ = (oppPieces | ownPieces) & pawnMoves[square][1];
            if(!occ) moves |= pawnMoves[square][1];
        }

        occ = (oppPieces | enPassant) & pawnMoves[square][2];
        if(occ) moves |= pawnMoves[square][2];

        occ = (oppPieces | enPassant) & pawnMoves[square][3];
        if(occ) moves |= pawnMoves[square][3];

        pawns &= pawns - 1;
    }

    return moves;
}

uint64_t getPawnMove(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, char flags){
    if(flags & WHITE_TURN) return getWhitePawnMoves(pawns, ownPieces, oppPieces, enPassant);
    return flipVertical(
        getWhitePawnMoves(
            flipVertical(pawns), 
            flipVertical(ownPieces), 
            flipVertical(oppPieces), 
            flipVertical(enPassant))
        );
}

uint64_t getWhitePawnMovesAppend(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    uint64_t occ =  0ULL;

    while (pawns) {
        uint64_t moves = 0ULL;
        int square = __builtin_ctzll(pawns);
        int rank = square / 8;
        char promotion = (rank == 6) ? 1 : 0;

        //Single Step
        occ = (oppPieces | ownPieces) & pawnMoves[square][0];
        if(!occ) moves |= pawnMoves[square][0];
        
        //Double Step
        if(occ == 0 && rank == 1){
            occ = (oppPieces | ownPieces) & pawnMoves[square][1];
            if(!occ) moves |= pawnMoves[square][1];
        }

        //Capture Left
        occ = (oppPieces | enPassant) & pawnMoves[square][2];
        if(occ) moves |= pawnMoves[square][2];

        //Capture Right
        occ = (oppPieces | enPassant) & pawnMoves[square][3];
        if(occ) moves |= pawnMoves[square][3];

        all_moves |= moves;
        while(moves){
            int move_sq = __builtin_ctzll(moves);
            if (promotion) {
                Move baseMove = SET_MOVE(square, move_sq);
                moveList[*idx] = SET_MOVE_PROMOTION(baseMove, PROMOTE_QUEEN);
                (*idx)++;
                moveList[*idx] = SET_MOVE_PROMOTION(baseMove, PROMOTE_ROOK);
                (*idx)++;
                moveList[*idx] = SET_MOVE_PROMOTION(baseMove, PROMOTE_BISHOP);
                (*idx)++;
                moveList[*idx] = SET_MOVE_PROMOTION(baseMove, PROMOTE_KNIGHT);
                (*idx)++;
            } else {
                moveList[*idx] = SET_MOVE(square, move_sq);
                (*idx)++;
            }
            moves &= moves - 1;
        }

        pawns &= pawns - 1;
    }

    return all_moves;
}

uint64_t getPawnMovesAppend(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, char flags, Move* moveList, int* idx){
    if(flags & WHITE_TURN) return getWhitePawnMovesAppend(pawns, ownPieces, oppPieces, enPassant, moveList, idx);
    return flipVertical(
        getWhitePawnMovesAppend(
            flipVertical(pawns), 
            flipVertical(ownPieces), 
            flipVertical(oppPieces), 
            flipVertical(enPassant), 
            moveList, 
            idx)
        );
}


//Knight
uint64_t getKnightMoves(uint64_t knights, uint64_t ownPieces) {
    uint64_t moves = 0ULL;

    while (knights) {
        int square = __builtin_ctzll(knights);
        moves |= knightMoves[square];
        knights &= knights - 1;
    }

    return moves & ~ownPieces;
}

uint64_t getKnightMovesAppend(uint64_t knights, uint64_t ownPieces, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;

    while (knights) {
        int square = __builtin_ctzll(knights);
        uint64_t moves = knightMoves[square] & ~ownPieces;
        all_moves |= moves;
        while(moves){
            int move_sq = __builtin_ctzll(moves);
            moveList[*idx] = SET_MOVE(square, move_sq);
            (*idx)++;
            moves &= moves - 1;
        }
        knights &= knights - 1;
    }

    return all_moves;
}

//King
uint64_t getKingMoves(uint64_t kings, uint64_t ownPieces) {
    int square = __builtin_ctzll(kings);
    return kingMoves[square] & ~ownPieces;
}

uint64_t getKingMovesAppend(uint64_t kings, uint64_t ownPieces, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;

    int square = __builtin_ctzll(kings);
    uint64_t moves = kingMoves[square] & ~ownPieces;
    all_moves |= moves;
    while(moves){
        int move_sq = __builtin_ctzll(moves);
        moveList[*idx] = SET_MOVE(square, move_sq);
        (*idx)++;
        moves &= moves - 1;
    }
    return all_moves;
}

void getCastleMovesWhiteAppend(uint64_t white, char flags, Move* moveList, int* idx){
    if ((flags & W_SHORT_CASTLE) && !(white & W_SHORT_CASTLE_MASK)) {
        Move move = SET_MOVE(4, 6);
        move = SET_MOVE_FLAGS(move, 1, 0);
        moveList[*idx] = move;
        (*idx)++;
    }

    if ((flags & W_LONG_CASTLE) && !(white & W_LONG_CASTLE_MASK)) {
        Move move = SET_MOVE(4, 2);
        move = SET_MOVE_FLAGS(move, 1, 0);
        moveList[*idx] = move;
        (*idx)++;
    }
}

void getCastleMovesBlackAppend(uint64_t black, char flags, Move* moveList, int* idx){
    if ((flags & B_SHORT_CASTLE) && !(black & B_SHORT_CASTLE_MASK)) {
        Move move = SET_MOVE(60, 62);
        move = SET_MOVE_FLAGS(move, 1, 0);
        moveList[*idx] = move;
        (*idx)++;
    }

    if ((flags & B_LONG_CASTLE) && !(black & B_LONG_CASTLE_MASK)) {
        Move move = SET_MOVE(60, 58);
        move = SET_MOVE_FLAGS(move, 1, 0);
        moveList[*idx] = move;
        (*idx)++;
    }
}