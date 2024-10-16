#pragma once
#include <stdint.h>
#include <stddef.h>
#include <inttypes.h>
#include <time.h>

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef int32_t bool32;
typedef bool32 b32;

typedef float real32;
typedef double real64;

typedef real32 f32;
typedef real64 d64;

#define TRUE 1
#define FALSE 0

#ifndef MAX_DEPTH
#define MAX_DEPTH 256
#endif

#define MAX_EVAL       9999999
#define MIN_EVAL      -9999999

#define CHECKMATE_VALUE (MAX_EVAL - 1000)

#define PLAYER_COUNT 2

#define BOARD_SIZE 64

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define MAX_MOVES    256 // Most moves possible in a single position
#define MAX_FEN_LEN  100 // Max length of a fen string
#define GAME_MOVES 12288 // Most moves possible in a game

#define NO_MOVE 0

#define MOVE_FROM_MASK        0x003F  // 0000 0000 0011 1111
#define MOVE_TO_MASK          0x0FC0  // 0000 1111 1100 0000
#define MOVE_FLAG_MASK        0xF000  // 1111 0000 0000 0000

#define HASHSTACK_SIZE 100
#define KMV_CNT 3

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

typedef u16 Move;

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

//For Compares
#define PROMOTION               0x8

// Flag setters using defined constants
#define SET_QUIET(move)                SET_FLAGS(move, QUIET)
#define SET_DOUBLE_PAWN_PUSH(move)     SET_FLAGS(move, DOUBLE_PAWN_PUSH)
#define SET_KING_CASTLE(move)          SET_FLAGS(move, KING_CASTLE)
#define SET_QUEEN_CASTLE(move)         SET_FLAGS(move, QUEEN_CASTLE)
#define SET_CAPTURE(move)              SET_FLAGS(move, CAPTURE)
#define SET_EP_CAPTURE(move)           SET_FLAGS(move, EP_CAPTURE)
#define SET_KNIGHT_PROMOTION(move)     SET_FLAGS(move, KNIGHT_PROMOTION)
#define SET_BISHOP_PROMOTION(move)     SET_FLAGS(move, BISHOP_PROMOTION)
#define SET_ROOK_PROMOTION(move)       SET_FLAGS(move, ROOK_PROMOTION)
#define SET_QUEEN_PROMOTION(move)      SET_FLAGS(move, QUEEN_PROMOTION)
#define SET_KNIGHT_PROMO_CAPTURE(move) SET_FLAGS(move, KNIGHT_PROMO_CAPTURE)
#define SET_BISHOP_PROMO_CAPTURE(move) SET_FLAGS(move, BISHOP_PROMO_CAPTURE)
#define SET_ROOK_PROMO_CAPTURE(move)   SET_FLAGS(move, ROOK_PROMO_CAPTURE)
#define SET_QUEEN_PROMO_CAPTURE(move)  SET_FLAGS(move, QUEEN_PROMO_CAPTURE)

// Flag getters
inline i32 IS_QUIET(Move move) { return GET_FLAGS(move) == QUIET; }
inline i32 IS_DOUBLE_PAWN_PUSH(Move move) { return GET_FLAGS(move) == DOUBLE_PAWN_PUSH; }
inline i32 IS_KING_CASTLE(Move move) { return GET_FLAGS(move) == KING_CASTLE; }
inline i32 IS_QUEEN_CASTLE(Move move) { return GET_FLAGS(move) == QUEEN_CASTLE; }
inline i32 IS_CAPTURE(Move move) { return GET_FLAGS(move) == CAPTURE; }
inline i32 IS_EP_CAPTURE(Move move) { return GET_FLAGS(move) == EP_CAPTURE; }
inline i32 IS_KNIGHT_PROMOTION(Move move) { return GET_FLAGS(move) == KNIGHT_PROMOTION; }
inline i32 IS_BISHOP_PROMOTION(Move move) { return GET_FLAGS(move) == BISHOP_PROMOTION; }
inline i32 IS_ROOK_PROMOTION(Move move) { return GET_FLAGS(move) == ROOK_PROMOTION; }
inline i32 IS_QUEEN_PROMOTION(Move move) { return GET_FLAGS(move) == QUEEN_PROMOTION; }
inline i32 IS_KNIGHT_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == KNIGHT_PROMO_CAPTURE; }
inline i32 IS_BISHOP_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == BISHOP_PROMO_CAPTURE; }
inline i32 IS_ROOK_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == ROOK_PROMO_CAPTURE; }
inline i32 IS_QUEEN_PROMO_CAPTURE(Move move) { return GET_FLAGS(move) == QUEEN_PROMO_CAPTURE; }

static inline Move create_move(i32 from, i32 to, i32 flags) {
    Move move = 0;
    SET_FROM(move, from);
    SET_TO(move, to);
    SET_FLAGS(move, flags);
    return move;
}

typedef enum {
    IN_D_CHECK     = 0x40,
    IN_CHECK       = 0x20,
    W_SHORT_CASTLE = 0x10,
    W_LONG_CASTLE  = 0x08,
    B_SHORT_CASTLE = 0x04,
    B_LONG_CASTLE  = 0x02,
    TURN_MASK      = 0x01
} PositionFlag;

typedef enum{
    BLACK_TURN = 0,
    WHITE_TURN = 1,
    BLACK = 0,
    WHITE = 1,
} Turn;

typedef enum {
    WHITE_PAWN, WHITE_KNIGHT, WHITE_BISHOP, WHITE_ROOK, WHITE_QUEEN, WHITE_KING,
    BLACK_PAWN, BLACK_KNIGHT, BLACK_BISHOP, BLACK_ROOK, BLACK_QUEEN, BLACK_KING
} PieceIndex;

static const i32 pieceToIndex[128] = {
    ['P'] = WHITE_PAWN,
    ['N'] = WHITE_KNIGHT,
    ['B'] = WHITE_BISHOP,
    ['R'] = WHITE_ROOK,
    ['Q'] = WHITE_QUEEN,
    ['K'] = WHITE_KING,
    ['p'] = BLACK_PAWN,
    ['n'] = BLACK_KNIGHT,
    ['b'] = BLACK_BISHOP,
    ['r'] = BLACK_ROOK,
    ['q'] = BLACK_QUEEN,
    ['k'] = BLACK_KING
};

typedef struct {
    u64 hash[HASHSTACK_SIZE]; //A stack of hashes of player positions
    i32 cur_idx;
    i32 reset_idx; //An index to the last move which reset halfmove clock
} HashStack;

typedef enum {
    OPN_GAME,
    MID_GAME,
    END_GAME
} Stage;

typedef struct {            //Each size of 2 array contains {Black, White}
    u64 pawn[2];     
    u64 bishop[2];
    u64 knight[2];
    u64 rook[2];
    u64 queen[2];
    u64 king[2];

    u64 attack_mask[2]; // {Attacked by Black, Attacked by White}

    u64 color[2];  // {White Pieces, Black Pieces}

    u64 en_passant;  //En Passant squares
    u8 flags;  //Castle aval as bit flags, in order : w_long_castle | w_short_castle | b_long_castle | b_short_castle | turn | in_check | in_double_check
    //1 means avaliable / white's turn

    char charBoard[64];  //Character Board

    u64 pinned; //Absolutely pinned pieces

    u64 hash; //Hash of the position

    i32 material_eval;

    Stage stage; //The stage of the game

    i32 halfmove_clock;
    i32 fullmove_number;
} Position;

typedef struct {
    u64 attack_mask[2];

    u64 en_passant;
    u8 flags;  
    u8 captured;

    u64 pinned;

    i32 material_eval;

    i32 halfmove_clock;

    i32 hash_reset_idx;

    u64 hash;
} Undo;

typedef struct {
    Undo undo[MAX_DEPTH];
    i32 idx;
} UndoStack;

typedef struct {
    u16 move;
    u64 hash;
} Node;

typedef enum {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
} Square;


typedef struct{
    struct timespec start_time;
    struct timespec end_time;
    double elap_time;
    u64 node_count;
} SearchStats;

typedef struct{
    Move* pv_array;
    Move best_move;
    u32 depth;
    i32 eval;
    SearchStats stats;
} SearchData;

typedef struct{
    Move table[MAX_DEPTH][KMV_CNT];
    u32 kmvIdx;
} KillerMoves;

typedef struct{
    u32 max_time;
    u32 rec_time;
    u8  can_shorten;
    u32 depth;
} SearchParameters;

typedef enum {
    NORMAL_TIME,
    EXTEND_TIME,
    REDUCE_TIME,
    HALT_TIME
} TimePreference;

typedef struct{
    i32 thread_num;
    u8 is_helper_thread;
    Move pv_array[MAX_DEPTH];
    KillerMoves km;
    u32 depth;
    Position pos; 
    HashStack hash_stack;
    UndoStack undo_stack;
    Move found_move[MAX_DEPTH];
    i32  found_eval[MAX_DEPTH];
    i32 avg_eval;
    TimePreference time_pref;
    SearchStats stats;
} ThreadData;

// Forward definitions
typedef struct EvalData EvalData;
