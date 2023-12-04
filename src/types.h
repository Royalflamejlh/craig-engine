//
//  types.h
//  godengine
//
//  Created by John Howard on 11/21/23.
//

#ifndef types_h
#define types_h
#include <stdint.h>
#include <stddef.h>

#define WHITE_TURN 0x10
#define W_SHORT_CASTLE 0x08
#define W_LONG_CASTLE 0x04
#define B_SHORT_CASTLE 0x02 
#define B_LONG_CASTLE 0x01 

typedef struct {
    unsigned char from_x;
    unsigned char from_y;
    unsigned char to_x;
    unsigned char to_y;
    char promotion;
} Move;

typedef struct {
    uint64_t w_pawn;
    uint64_t w_bishop;
    uint64_t w_knight;
    uint64_t w_rook;
    uint64_t w_queen;
    uint64_t w_king;

    uint64_t b_pawn;
    uint64_t b_bishop;
    uint64_t b_knight;
    uint64_t b_rook;
    uint64_t b_queen;
    uint64_t b_king;

    uint64_t white;  //White piece positions
    uint64_t black;  //Black piece positions

    uint64_t en_passant;  //En Passant squares
    char flags;  //Castle aval as bit flags, in order : w_long_castle | w_short_castle | b_long_castle | b_short_castle | turn
    //1 means avaliable / white's turn

    int halfmove_clock;
    int fullmove_number;
} Position;

typedef struct {
    uint16_t move;
    uint64_t hash;
} Node;

#endif /* types_h */
