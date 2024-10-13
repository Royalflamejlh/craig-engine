#include "hash.h"
#include "util.h"
#include <time.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

static u64 zobristTable[64][12];
static u64 zobristEnPassant[8];
static u64 zobristCastle[4];
static u64 zobristTurn;

void initZobrist(void) {
    #ifdef __RAND_SEED
    srand(__RAND_SEED);
    #else
    time_t t;
    srand((unsigned) time(&t));
    #endif

    for (i32 square = 0; square < 64; square++) {
        for (i32 piece = 0; piece < 12; piece++) {
            zobristTable[square][piece] = random_uint64();
        }
    }

    for(i32 i = 0; i < 8; i++){
        zobristEnPassant[i] = random_uint64();
    }

    for(i32 i = 0; i < 4; i++){
        zobristCastle[i] = random_uint64();
    }

    zobristTurn = random_uint64();
}

#ifdef DEBUG
void debug_hash_difference(u64 hash1, u64 hash2){
    u64 diff = hash1 ^ hash2;
    for (i32 square = 0; square < 64; square++) {
        for (i32 piece = 0; piece < 12; piece++) {
            if(zobristTable[square][piece] == diff){
                printf("Hash difference found to be from hash for piece %d at square %d\n", piece, square);
            }
        }
    }
    for(i32 i = 0; i < 8; i++){
        if(zobristEnPassant[i] == diff){
            printf("Hash difference found to be from en passant %d\n", i);
        }
    }
    for(i32 i = 0; i < 4; i++){
        if(zobristCastle[i] == diff){
            printf("Hash difference found to be from castle %d\n", i);
        }
    }
    if(zobristTurn  == diff){
        printf("Hash difference found to be from turn hash\n");
    }
}
#endif

u64 hashPosition(Position *pos){
    u64 hash = 0;

    //Hash the board
    for (i32 i = 0; i < 64; i++) { 
        if (pos->charBoard[i] != 0) { 
            i32 piece = pieceToIndex[(int)pos->charBoard[i]];
            hash ^= zobristTable[i][piece];
        }
    }

    //Hash the enpassant file
    if(pos->en_passant) {
        i32 epFile = getlsb(pos->en_passant) % 8;
        hash ^= zobristEnPassant[epFile];
    }
    //Hash the turn
    if(pos->flags & WHITE_TURN) hash ^= zobristTurn;

    //Hash the castle flags
    if(pos->flags & W_SHORT_CASTLE) hash ^= zobristCastle[0];
    if(pos->flags & W_LONG_CASTLE)  hash ^= zobristCastle[1];
    if(pos->flags & B_SHORT_CASTLE) hash ^= zobristCastle[2];
    if(pos->flags & B_LONG_CASTLE)  hash ^= zobristCastle[3];

    return hash;
}

/**
 * Removes or adds a piece to/from the hash.
 */
u64 hash_update_piece(u64 hash, i32 sq, i32 piece){
    return (hash ^ zobristTable[sq][piece]);
}

/**
 * Changes the turn in the hash.
 */
u64 hash_update_turn(u64 hash){
    return (hash ^ zobristTurn);
}

/**
 * Removes or adds a enpassant square to/from the hash.
 */
u64 hash_update_enpassant(u64 hash, i32 sq){
    return (hash ^ zobristEnPassant[sq % 8]);
}

/**
 * Hash update castling based on the change in flags
 */
u64 hash_update_castle(u64 hash, PositionFlag castle){
    if(castle == W_SHORT_CASTLE) hash ^= zobristCastle[0];
    if(castle == W_LONG_CASTLE)  hash ^= zobristCastle[1];
    if(castle == B_SHORT_CASTLE) hash ^= zobristCastle[2];
    if(castle == B_LONG_CASTLE)  hash ^= zobristCastle[3];
    return hash;
}