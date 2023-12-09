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

#define MOVE_FROM_MASK        0x003F  // 0000 0000 0011 1111
#define MOVE_TO_MASK          0x0FC0  // 0000 1111 1100 0000
#define MOVE_FLAG_MASK        0xF000  // 1111 0000 0000 0000

/*  Flags
  prom cap  a   b  
    0	0	0	0	quiet moves
    0	0	0	1	double pawn push
    0	0	1	0	king castle
    0	0	1	1	queen castle
    0	1	0	0	captures
    0	1	0	1	ep-capture
    1	0	0	0	knight-promotion
    1	0	0	1	bishop-promotion
    1	0	1	0	rook-promotion
    1	0	1	1	queen-promotion
    1	1	0	0	knight-promo capture
    1	1	0	1	bishop-promo capture
    1	1	1	0	rook-promo capture
    1	1	1	1	queen-promo capture
*/

typedef uint16_t Move;

#define GET_FROM(move) ((move) & MOVE_FROM_MASK)
#define GET_TO(move) (((move) & MOVE_TO_MASK) >> 6)

#define SET_FROM(move, from) ((move) = ((move) & ~MOVE_FROM_MASK) | (from))
#define SET_TO(move, to) ((move) = ((move) & ~MOVE_TO_MASK) | ((to) << 6))

#define GET_FLAGS(move) (((move) & MOVE_FLAG_MASK) >> 12)
#define SET_FLAGS(move, flags) ((move) = ((move) & ~MOVE_FLAG_MASK) | ((flags) << 12))

// Move flag definitions
#define QUIET                   0x0
#define DOUBLE_PAWN_PUSH        0x1
#define KING_CASTLE             0x2
#define QUEEN_CASTLE            0x3
#define CAPTURE                 0x4
#define EP_CAPTURE              0x5
#define KNIGHT_PROMOTION        0x8
#define BISHOP_PROMOTION        0x9
#define ROOK_PROMOTION          0xA
#define QUEEN_PROMOTION         0xB
#define KNIGHT_PROMO_CAPTURE    0xC
#define BISHOP_PROMO_CAPTURE    0xD
#define ROOK_PROMO_CAPTURE      0xE
#define QUEEN_PROMO_CAPTURE     0xF

// Flag setters using defined constants
#define SET_QUIET(move)              SET_FLAGS(move, QUIET)
#define SET_DOUBLE_PAWN_PUSH(move)   SET_FLAGS(move, DOUBLE_PAWN_PUSH)
#define SET_KING_CASTLE(move)        SET_FLAGS(move, KING_CASTLE)
#define SET_QUEEN_CASTLE(move)       SET_FLAGS(move, QUEEN_CASTLE)
#define SET_CAPTURE(move)            SET_FLAGS(move, CAPTURE)
#define SET_EP_CAPTURE(move)         SET_FLAGS(move, EP_CAPTURE)
#define SET_KNIGHT_PROMOTION(move)   SET_FLAGS(move, KNIGHT_PROMOTION)
#define SET_BISHOP_PROMOTION(move)   SET_FLAGS(move, BISHOP_PROMOTION)
#define SET_ROOK_PROMOTION(move)     SET_FLAGS(move, ROOK_PROMOTION)
#define SET_QUEEN_PROMOTION(move)    SET_FLAGS(move, QUEEN_PROMOTION)
#define SET_KNIGHT_PROMO_CAPTURE(move) SET_FLAGS(move, KNIGHT_PROMO_CAPTURE)
#define SET_BISHOP_PROMO_CAPTURE(move) SET_FLAGS(move, BISHOP_PROMO_CAPTURE)
#define SET_ROOK_PROMO_CAPTURE(move) SET_FLAGS(move, ROOK_PROMO_CAPTURE)
#define SET_QUEEN_PROMO_CAPTURE(move) SET_FLAGS(move, QUEEN_PROMO_CAPTURE)

// Flag getters
inline int IS_QUIET(Move move) { return GET_FLAGS(move) == QUIET; }
inline int IS_DOUBLE_PAWN_PUSH(Move move) { return GET_FLAGS(move) == DOUBLE_PAWN_PUSH; }
inline int IS_KING_CASTLE(Move move) { return GET_FLAGS(move) == KING_CASTLE; }
inline int IS_QUEEN_CASTLE(Move move) { return GET_FLAGS(move) == QUEEN_CASTLE; }
inline int IS_CAPTURE(Move move) { return GET_FLAGS(move) == CAPTURE; }
inline int IS_EP_CAPTURE(Move move) { return GET_FLAGS(move) == EP_CAPTURE; }
inline int IS_KNIGHT_PROMOTION(Move move) { return GET_FLAGS(move) == KNIGHT_PROMOTION; }
inline int IS_BISHOP_PROMOTION(Move move) { return GET_FLAGS(move) == BISHOP_PROMOTION; }
inline int IS_ROOK_PROMOTION(Move move) { return GET_FLAGS(move) == ROOK_PROMOTION; }
inline int IS_QUEEN_PROMOTION(Move move) { return GET_FLAGS(move) == QUEEN_PROMOTION; }
inline int IS_KNIGHT_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == KNIGHT_PROMO_CAPTURE; }
inline int IS_BISHOP_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == BISHOP_PROMO_CAPTURE; }
inline int IS_ROOK_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == ROOK_PROMO_CAPTURE; }
inline int IS_QUEEN_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == QUEEN_PROMO_CAPTURE; }

static inline Move MAKE_MOVE(int from, int to, int flags) {
    Move move = 0;
    SET_FROM(move, from);
    SET_TO(move, to);
    SET_FLAGS(move, flags);
    return move;
}

#define IN_D_CHECK     0x40
#define IN_CHECK       0x20
#define WHITE_TURN     0x10
#define W_SHORT_CASTLE 0x08
#define W_LONG_CASTLE  0x04
#define B_SHORT_CASTLE 0x02 
#define B_LONG_CASTLE  0x01 

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
    char flags;  //Castle aval as bit flags, in order : w_long_castle | w_short_castle | b_long_castle | b_short_castle | turn | in_check | in_double_check
    //1 means avaliable / white's turn

    char charBoard[64];  //Character Board

    uint64_t pinned; //Absolutely pinned pieces

    uint64_t w_attack_mask; //Pieces white is attacking
    uint64_t b_attack_mask; //Pieces black is attacking

    int halfmove_clock;
    int fullmove_number;
} Position;


typedef struct {
    uint16_t move;
    uint64_t hash;
} Node;

#endif /* types_h */
