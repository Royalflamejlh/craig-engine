#include "hash.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include "util.h"
#include <time.h>
#include <stdlib.h>

static uint64_t zobristTable[64][12];
static uint64_t zobristEnPassant[8];
static uint64_t zobristCastle[4];
static uint64_t zobristTurn;

static int convertPieceToIndex(char piece);

void initZobrist(void) {
    time_t t;
    srand((unsigned) time(&t));

    for (int square = 0; square < 64; square++) {
        for (int piece = 0; piece < 12; piece++) {
            zobristTable[square][piece] = random_uint64();
        }
    }

    for(int i = 0; i < 8; i++){
        zobristEnPassant[i] = random_uint64();
    }

    for(int i = 0; i < 4; i++){
        zobristCastle[i] = random_uint64();
    }

    zobristTurn = random_uint64();
}

uint64_t hashPosition(Position pos){
    uint64_t hash = 0;

    //Hash the board
    for (int i = 0; i < 64; i++) { 
        if (pos.charBoard[i] != 0) { 
            int piece = convertPieceToIndex(pos.charBoard[i]);
            hash ^= zobristTable[i][piece];
        }
    }

    //Hash the enpassant file
    if(pos.en_passant) hash ^= zobristEnPassant[__builtin_ctzll(pos.en_passant) % 8];
    
    //Hash the turn
    if(pos.flags & WHITE_TURN) hash ^= zobristTurn;

    //Hash the castle flags
    if(pos.flags & W_SHORT_CASTLE) hash ^= zobristCastle[0];
    if(pos.flags & W_LONG_CASTLE)  hash ^= zobristCastle[1];
    if(pos.flags & B_SHORT_CASTLE) hash ^= zobristCastle[2];
    if(pos.flags & B_LONG_CASTLE)  hash ^= zobristCastle[3];

    return hash;
}

static int convertPieceToIndex(char piece) {
    switch (piece) {
        case 'P': return 0;  // White Pawn
        case 'N': return 1;  // White Knight
        case 'B': return 2;  // White Bishop
        case 'R': return 3;  // White Rook
        case 'Q': return 4;  // White Queen
        case 'K': return 5;  // White King
        case 'p': return 6;  // Black Pawn
        case 'n': return 7;  // Black Knight
        case 'b': return 8;  // Black Bishop
        case 'r': return 9;  // Black Rook
        case 'q': return 10; // Black Queen
        case 'k': return 11; // Black King
        default:  return -1; // Invalid piece
    }
}
