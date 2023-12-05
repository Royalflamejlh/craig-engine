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

#define MOVE_FROM_MASK        0x003F  // 000000 000000 11 1111
#define MOVE_TO_MASK          0x0FC0  // 000000 111111 00 0000
#define MOVE_PROMOTION_MASK   0x3000  // 0011 00 000000 000000
#define MOVE_ENPASSANT_MASK   0x4000  // 0100 00 000000 000000
#define MOVE_CASTLE_MASK      0x8000  // 1000 00 000000 000000

#define PROMOTE_QUEEN    0
#define PROMOTE_BISHOP   (1 << 12)
#define PROMOTE_KNIGHT   (2 << 12)
#define PROMOTE_ROOK     (3 << 12)

typedef uint16_t Move;
// 6-From | 6-To | 2-PromotionPiece | 1-Castle | 1-EnPassant
#define SET_MOVE(from, to) \
    (((from) & MOVE_FROM_MASK) | (((to) << 6) & MOVE_TO_MASK))

#define SET_MOVE_PROMOTION(move, promotion) \
    ((move) | ((promotion) & MOVE_PROMOTION_MASK))

#define SET_MOVE_FLAGS(move, isCastle, isEnPassant) \
    ((move) | ((isCastle) ? MOVE_CASTLE_MASK : 0) | ((isEnPassant) ? MOVE_ENPASSANT_MASK : 0))

#define GET_FROM(move) ((move) & MOVE_FROM_MASK)
#define GET_TO(move) (((move) & MOVE_TO_MASK) >> 6)

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

    char charBoard[64]; //Character Board

    int halfmove_clock;
    int fullmove_number;
} Position;


typedef struct {
    uint16_t move;
    uint64_t hash;
} Node;

#endif /* types_h */
