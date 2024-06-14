#include "evaluator.h"
#include "types.h"
#include "util.h"
#include "tables.h"
#include "movement.h"
#include "bitboard/bbutils.h"
#include "bitboard/bitboard.h"


//Piece values defined in header

#define BASE_ATTACK_BONUS   5  // Eval for each enemy piece under attack
#define BASE_DEFEND_BONUS  10  // Eval for each defended piece
#define BASE_HANGING_PEN   90  // Eval lost for hanging a piece

#define TO_MOVE_BONUS      0  // Eval bonus for being your turn

#define CHECK_PEN         100  // Eval lost under check

#define LOW_PAWN_PEN      500  // Eval lost for having 2 or less pawns
#define DOUBLE_PAWN_PEN   200  // Eval lost for double pawns
#define ISO_PAWN_PEN      100  // Eval lost for isolated pawns

#define PAWN_MOB      15  // Mobility bonus for Pawns in Eval
#define KNIGHT_MOB    35  // Mobility bonus for Knights in Eval
#define BISHOP_MOB    30  // Mobility bonus for Bishop in Eval
#define QUEEN_MOB     10  // Mobility bonus for Queen in Eval
#define ROOK_MOB      10  // Mobility bonus for Rook in Eval
#define KING_MOB      50  // Mobility bonus for King in Eval

#define CAN_CASTLE_BONUS 10 // Bonus for being able to Castle

#define KING_SAFTEY_FRIEND 4 // Eval for each friendly piece next to king

#define CONNECTED_ROOK_BONUS  100 // Eval for rooks being connected

#define DOUBLE_BISHOP_BONUS   700  // Bonus for having both bishops
#define DOUBLE_ROOK_PEN       100  // Penality for having both rooks

// Defines for Movement Eval
#define MOVE_CASTLE_BONUS  300  // Eval bonus for castling
#define MOVE_OPN_QUEEN_PEN   5  // Pen for moving queen in the opening

/* Material Values */

const int pawn_value   =   1000;
const int knight_value =   3500;
const int bishop_value =   3600;
const int rook_value   =   5000;
const int queen_value  =  10000;
const int king_value   = 100000;

static const i32 pieceValues[] = {
    [WHITE_PAWN  ] = pawn_value,
    [BLACK_PAWN  ] = pawn_value,
    [WHITE_KNIGHT] = knight_value,
    [BLACK_KNIGHT] = knight_value,
    [WHITE_BISHOP] = bishop_value,
    [BLACK_BISHOP] = bishop_value,
    [WHITE_ROOK  ] = rook_value,
    [BLACK_ROOK  ] = rook_value,
    [WHITE_QUEEN ] = queen_value,
    [BLACK_QUEEN ] = queen_value,
    [WHITE_KING  ] = king_value,
    [BLACK_KING  ] = king_value
};

/* Piece-Square Tables */

const i32 pst_pawn[3][64] = {
    [OPN_GAME] =
    {
          0,   0,   0,   0,   0,   0,   0,   0,
         10,   0,   0, -50, -50,   0,   0,  10,
          0,  30,  30,  10,  10,  30,  30,   0,
          0,   0,   0,  40,  40,   0,   0,   0,
          0,   0,   0,  20,  20,   0,   0,   0,
         10,  10,  10,  20,  20,  10,  10,  10,
         30,  30,  30,  30,  30,  30,  30,  30,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    [MID_GAME] =
    {
          0,   0,   0,   0,   0,   0,   0,   0,
         10,   0,   0, -10, -10,   0,   0,  10,
         10,  20,  20,  10,  10,  20,  20,  10,
          0,  10,  10,  30,  30,  10,  10,   0,
          0,   0,   0,  30,  30,   0,   0,   0,
         30,  30,  30,  40,  40,  30,  30,  30,
         90,  90,  90,  90,  90,  90,  90,  90,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    [END_GAME] =
    {
          0,   0,   0,   0,   0,   0,   0,   0,
        -10, -10, -10, -10, -10, -10, -10, -10,
         -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,
          0,   0,   0,   0,   0,   0,   0,   0,
         30,  30,  30,  30,  30,  30,  30,  30,
         70,  70,  70,  70,  70,  70,  70,  70,
         90,  90,  90,  90,  90,  90,  90,  90,
          0,   0,   0,   0,   0,   0,   0,   0
    }
};

const i32 pst_knight[3][64] = {
    [OPN_GAME] =
    {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,  20,  20,   0, -20, -40,
        -30,   0,  30,  30,  30,  30,   0, -30,
        -30,   0,  10,  20,  20,  10,   0, -30,
        -30,   0,  10,  20,  20,  10,   0, -30,
        -30,   0,  10,  10,  10,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    },
    [MID_GAME] = 
    {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -30,   0,  10,  10,  10,  10,   0, -30,
        -30,   0,  10,  20,  20,  10,   0, -30,
        -30,   0,  10,  20,  20,  10,   0, -30,
        -30,   0,  10,  10,  10,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    },
    [END_GAME] =
    {
        -50, -40, -30, -30, -30, -30, -40, -50,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -30,   0,  10,  10,  10,  10,   0, -30,
        -30,   0,  10,  20,  20,  10,   0, -30,
        -30,   0,  10,  20,  20,  10,   0, -30,
        -30,   0,  10,  10,  10,  10,   0, -30,
        -40, -20,   0,   0,   0,   0, -20, -40,
        -50, -40, -30, -30, -30, -30, -40, -50
    }
};

const i32 pst_bishop[3][64] = {
    [OPN_GAME] =
    {
         10, -10, -10, -10, -10, -10, -10,  10,
        -10,  40,   0,   0,   0,   0,  40, -10,
        -10,   0,  20,  40,  40,  20,   0, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,  10,   0,   0,   0,   0,  10, -10,
         10, -10, -10, -10, -10, -10, -10,  10
    },
    [MID_GAME] = 
    {
         10, -10, -10, -10, -10, -10, -10,  10,
        -10,  10,   0,   0,   0,   0,  10, -10,
        -10,   0,  10,  20,  20,  10,   0, -10,
        -10,   0,  10,  20,  20,  10,   0, -10,
        -10,   0,  20,  20,  20,  20,   0, -10,
        -10,  20,  20,  20,  20,  20,  20, -10,
        -10,  10,   0,   0,   0,   0,  10, -10,
         10, -10, -10, -10, -10, -10, -10,  10
    },
    [END_GAME] =
    {
         20, -10, -10, -10, -10, -10, -10,  20,
        -10,  20,   0,   0,   0,   0,  20, -10,
        -10,   0,  20,  10,  10,  20,   0, -10,
        -10,   0,  10,  20,  20,  10,   0, -10,
        -10,   0,  10,  20,  20,  10,   0, -10,
        -10,   0,  20,  10,  10,  20,   0, -10,
        -10,  20,   0,   0,   0,   0,  20, -10,
         20, -10, -10, -10, -10, -10, -10,  20
    }
};

const i32 pst_rook[3][64] = {
    [OPN_GAME] =
    {
          0,   0,   0,  20,  20,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
         10,  20,  20,  20,  20,  20,  20,  10,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    [MID_GAME] =
    {
          0,   0,   0,  10,  10,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
         10,  20,  20,  20,  20,  20,  20,  10,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    [END_GAME] =
    {
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0
    },
};

const i32 pst_queen[3][64] = {
    [OPN_GAME] =
    {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   0,   0,  10,  10,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    },
    [MID_GAME] =
    {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,  10,  10,  10,  10,   0, -10,
        -10,   0,  10,  20,  20,  10,   0, -10,
        -10,   0,  10,  20,  20,  10,   0, -10,
        -10,  10,  10,  10,  10,  10,  10, -10,
        -10,   0,  10,   0,   0,  10,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    },
    [END_GAME] =
    {
        -20, -10, -10, -10, -10, -10, -10, -20,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -10,   0,   0,   0,   0,   0,   0, -10,
        -20, -10, -10, -10, -10, -10, -10, -20
    }
};

i32 pst_king[3][64] = {
    [OPN_GAME] =
    {
         40,  40, -10,   0,   0, -10,  40,  40,
         20,  20, -10, -10, -10, -10,  20,  20,
        -10, -20, -20, -20, -20, -20, -20, -10,
        -20, -30, -30, -40, -40, -30, -30, -20,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -50, -50, -50, -50, -40, -30
    },
    [MID_GAME] =
    {
         30,  40,  10,   0,   0,  10,  40,  30,
         10,  10, -10, -10, -10, -10,  10,  10,
        -10, -20, -20, -50, -50, -20, -20, -10,
        -20, -50, -50, -90, -90, -50, -50, -20,
        -30, -50, -50, -90, -90, -50, -50, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30,
        -30, -40, -40, -50, -50, -40, -40, -30
    },
    [END_GAME] =
    {
        -50, -40, -30, -20, -20, -30, -40, -50,
        -30, -20, -10,   0,   0, -10, -20, -30,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -30, -10,  30,  40,  40,  30, -10, -30,
        -30, -10,  30,  40,  40,  30, -10, -30,
        -30, -10,  20,  30,  30,  20, -10, -30,
        -30, -30,   0,   0,   0,   0, -30, -30,
        -50, -30, -30, -30, -30, -30, -30, -50
    }
};

i32 quickEval(Position pos){
    i32 eval_val = 0;
    Turn turn = pos.flags & TURN;
    eval_val += king_value   * (count_bits(pos.king[turn])   - count_bits(pos.king[!turn]));
    eval_val += queen_value  * (count_bits(pos.queen[turn])  - count_bits(pos.queen[!turn]));
    eval_val += rook_value   * (count_bits(pos.rook[turn])   - count_bits(pos.rook[!turn]));
    eval_val += bishop_value * (count_bits(pos.bishop[turn]) - count_bits(pos.bishop[!turn]));
    eval_val += knight_value * (count_bits(pos.knight[turn]) - count_bits(pos.knight[!turn]));
    eval_val += pawn_value   * (count_bits(pos.pawn[turn])   - count_bits(pos.pawn[!turn]));
    return eval_val;
};

u64 getLeastValuablePiece(Position pos, u64 attadef, u8 turn, PieceIndex* piece){
    u64 subset = attadef & pos.pawn[turn]; // Pawn
    if (subset){
        *piece = WHITE_PAWN + 6*turn;
        return subset & -subset;
    }

    subset = attadef & pos.knight[turn]; // Knight
    if (subset){
        *piece = WHITE_KNIGHT + 6*turn;
        return subset & -subset;
    }

    subset = attadef & pos.bishop[turn]; // Bishops
    if (subset){
        *piece = WHITE_BISHOP + 6*turn;
        return subset & -subset;
    }

    subset = attadef & pos.rook[turn]; // Rooks
    if (subset){
        *piece = WHITE_ROOK + 6*turn;
        return subset & -subset;
    }

    subset = attadef & pos.queen[turn]; // Queens
    if (subset){
        *piece = WHITE_QUEEN + 6*turn;
        return subset & -subset;
    }

    subset = attadef & pos.king[turn]; // Kings
    if (subset){
        *piece = WHITE_KING + 6*turn;
        return subset & -subset;
    }

   return 0; // empty set
}

static inline i32 quickEval(Position pos){
    i32 eval_val = 0;
    Turn turn = pos.flags & WHITE_TURN;
    eval_val += KING_VALUE   * (count_bits(pos.king[turn])   - count_bits(pos.king[!turn]));
    eval_val += QUEEN_VALUE  * (count_bits(pos.queen[turn])  - count_bits(pos.queen[!turn]));
    eval_val += ROOK_VALUE   * (count_bits(pos.rook[turn])   - count_bits(pos.rook[!turn]));
    eval_val += BISHOP_VALUE * (count_bits(pos.bishop[turn]) - count_bits(pos.bishop[!turn]));
    eval_val += KNIGHT_VALUE * (count_bits(pos.knight[turn]) - count_bits(pos.knight[!turn]));
    eval_val += PAWN_VALUE   * (count_bits(pos.pawn[turn])   - count_bits(pos.pawn[!turn]));
    return eval_val;
};

/* Static Exchange Evaluator */

i32 see ( Position pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece){
    i32 gain[32], d = 0;
    i32 turn = pos.flags & WHITE_TURN;
    u64 mayXray = pos.pawn[0] | pos.pawn[1] | pos.bishop[0] | pos.bishop[1] | pos.rook[0] | pos.rook[1] | pos.queen[0] | pos.queen[1];
    u64 removed = 0;
    u64 fromSet = 1ULL << frSq;
    u64 attadef = getAttackers( pos, toSq, 0) | getAttackers( pos, toSq, 1);
    gain[d]     = pieceValues[target];
    while (fromSet) {
        d++; // next depth and side
        gain[d]  = pieceValues[aPiece] - gain[d-1]; // Speculative store
        attadef ^= fromSet; // reset bit in set to traverse
        removed |= fromSet; // Update bitboard storing removed pieces
        if ( fromSet & mayXray ){ // If the piece was possibly xrayed through
            attadef |= getXRayAttackers( pos, toSq, 0, removed) | getXRayAttackers( pos, toSq, 1, removed);
        }
        fromSet  = getLeastValuablePiece (pos, attadef, (turn + d) & 1, &aPiece);
    }
    while (--d){
        if(gain[d] >= -gain[d-1]) gain[d-1] = -gain[d];
    }
    return gain[0];
}   

i32 eval_mobility(Position position){
    i32 score = 0;
    i32 size[] = {0};
    i32 turn = position.flags & WHITE_TURN; // True for white false for black
    u64 ownPos = position.color[turn];
    u64 oppPos = position.color[!turn];
    u64 oppAttackMask = position.attack_mask[!turn];
    Move moveList[MAX_MOVES];


    if(!(position.flags & IN_CHECK) && !(position.pinned & ownPos)){
        getBishopMovesAppend(position.queen[turn],  ownPos, oppPos, moveList, size);
        getRookMovesAppend(  position.queen[turn],  ownPos, oppPos, moveList, size);
        score += QUEEN_MOB * *size;
        *size = 0;

        getRookMovesAppend(  position.rook[turn],   ownPos, oppPos, moveList, size);
        score += ROOK_MOB * *size;
        *size = 0;

        getBishopMovesAppend(position.bishop[turn], ownPos, oppPos, moveList, size);
        score += BISHOP_MOB * *size;
        *size = 0;

        getKnightMovesAppend(position.knight[turn], ownPos, oppPos, moveList, size);
        score += KNIGHT_MOB * *size;
        *size = 0;

        getKingMovesAppend(  position.king[turn],   ownPos, oppPos, oppAttackMask, moveList, size);
        score += KING_MOB * *size;
        *size = 0;

        getPawnMovesAppend(  position.pawn[turn],   ownPos, oppPos, position.en_passant, position.flags, moveList, size);
        score += PAWN_MOB * *size;
        *size = 0;
    }

    return score;
}

//
// Evaluation Function
//
#ifdef DEBUG
i32 evaluate(Position pos, u8 verbose){
#else
i32 evaluate(Position pos){
#endif // Debug
    i32 eval_val = pos.quick_eval;
    i32 turn = pos.flags & WHITE_TURN;
    i32 stage = pos.stage;
    
    // Insufficient Material Stuff
    if(isInsufficient(pos)) return 0;

    #ifdef DEBUG
    if(verbose){
        printf("\nEval for stage: %d, Starting at: %d\n", stage, eval_val);
    }
    #endif

    //
    // TURN
    //
    if(turn) eval_val += TO_MOVE_BONUS;
    else     eval_val -= TO_MOVE_BONUS;

    #ifdef DEBUG
    if(verbose) printf("After To Move Bonus: %d\n", eval_val);
    #endif
 
    //
    // Mobility
    //
    Position tempPos = pos;
    eval_val += eval_mobility(pos);
    makeNullMove(&pos);
    eval_val -= eval_mobility(pos);
    pos = tempPos;
    #ifdef DEBUG
    if(verbose) printf("After Mobility Bonus: %d\n", eval_val);
    #endif

    //
    // Can Castle
    //
    if (pos.flags & W_SHORT_CASTLE) eval_val += CAN_CASTLE_BONUS;
    if (pos.flags & W_LONG_CASTLE ) eval_val += CAN_CASTLE_BONUS;
    if (pos.flags & B_SHORT_CASTLE) eval_val -= CAN_CASTLE_BONUS;
    if (pos.flags & B_LONG_CASTLE ) eval_val -= CAN_CASTLE_BONUS;
    #ifdef DEBUG
    if(verbose) printf("After Can Castle Bonus: %d\n", eval_val);
    #endif

    //
    // Penalities
    //
    if(pos.flags & IN_CHECK) eval_val -= CHECK_PEN; // Penalty for being in check

    #ifdef DEBUG
    if(verbose) printf("After Penalties: %d\n", eval_val);
    #endif

    //
    // Pairing
    //
    if (count_bits(pos.bishop[turn]) == 2)  eval_val += DOUBLE_BISHOP_BONUS;
    if (count_bits(pos.bishop[!turn]) == 2) eval_val -= DOUBLE_BISHOP_BONUS;

    if (count_bits(pos.rook[turn])  == 2) eval_val -= DOUBLE_ROOK_PEN;
    if (count_bits(pos.rook[!turn]) == 2) eval_val += DOUBLE_ROOK_PEN;

    if(pos.stage != END_GAME){
        u64 rook1 = pos.rook[turn] & -pos.rook[turn];
        if(getRookAttacks(rook1, pos.color[turn], pos.color[!turn]) & (pos.rook[turn] & ~rook1))  eval_val += CONNECTED_ROOK_BONUS;
        rook1 = pos.rook[!turn] & -pos.rook[!turn];
        if(getRookAttacks(rook1, pos.color[!turn], pos.color[turn]) & (pos.rook[!turn] & ~rook1)) eval_val -= CONNECTED_ROOK_BONUS;
    }

    //
    // Defense
    //
    eval_val += BASE_DEFEND_BONUS * count_bits(pos.color[turn] & pos.attack_mask[turn]);
    eval_val -= BASE_DEFEND_BONUS * count_bits(pos.color[!turn] & pos.attack_mask[!turn]);
    #ifdef DEBUG
    if(verbose) printf("After Defend Bonus: %d\n", eval_val);
    #endif

    //
    // Attack
    //
    eval_val += BASE_ATTACK_BONUS * count_bits(pos.color[!turn] &  pos.attack_mask[turn]);
    eval_val -= BASE_ATTACK_BONUS * count_bits(pos.color[turn] &  pos.attack_mask[!turn]);
    #ifdef DEBUG
    if(verbose) printf("After Attack Bonus: %d\n", eval_val);
    #endif
    
    //
    // Hanging
    //
    eval_val -= BASE_HANGING_PEN * count_bits((pos.color[turn]  & ~pos.attack_mask[turn])  & pos.attack_mask[!turn]);
    eval_val += BASE_HANGING_PEN * count_bits((pos.color[!turn] & ~pos.attack_mask[!turn]) & pos.attack_mask[turn]);
    #ifdef DEBUG
    if(verbose) printf("After Hanging Penalty: %d\n", eval_val);
    #endif


    //
    // Pawn Evals
    //
    if(count_bits(pos.pawn[turn])  <= 2) eval_val -= LOW_PAWN_PEN; // Penalty for two or less pawns
    if(count_bits(pos.pawn[!turn]) <= 2) eval_val += LOW_PAWN_PEN;
    for(i32 i = 0; i < 8; i++){ //Double Pawn Check
        i32 double_pawns = count_bits(pos.pawn[turn] & fileMask[i]) - 1;
        if(double_pawns >= 1){
            eval_val -= DOUBLE_PAWN_PEN * double_pawns;
        }
        double_pawns = count_bits(pos.pawn[!turn] & fileMask[i]) - 1;
        if(double_pawns >= 1){
            eval_val += DOUBLE_PAWN_PEN * double_pawns;
        }
    }
    #ifdef DEBUG
    if(verbose) printf("After Pawn Structure: %d\n", eval_val);
    #endif

    // 
    // King Safety
    //
    if(pos.stage != END_GAME){
        eval_val += (i32)count_bits(getKingAttacks(pos.king[turn])  | pos.color[turn])  * KING_SAFTEY_FRIEND;
        eval_val -= (i32)count_bits(getKingAttacks(pos.king[!turn]) | pos.color[!turn]) * KING_SAFTEY_FRIEND;
    }

    //
    // Piece Square Tables (PST)
    //
    //Pawns
    i32 index = turn ? WHITE_PAWN : BLACK_PAWN;
    i32 index_op = turn ? BLACK_PAWN : WHITE_PAWN;
    u64 pieces = pos.pawn[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST[stage][index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.pawn[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST[stage][index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    if(verbose){
        printf("After Pawn PST: %d\n", eval_val);
    }
    #endif

    //Knights
    index = turn ? WHITE_KNIGHT : BLACK_KNIGHT;
    index_op = turn ? BLACK_KNIGHT : WHITE_KNIGHT;
    pieces = pos.knight[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST[stage][index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.knight[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST[stage][index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    if(verbose){
        printf("After Knight PST: %d\n", eval_val);
    }
    #endif

    //Bishops
    index = turn ? WHITE_BISHOP : BLACK_BISHOP;
    index_op = turn ? BLACK_BISHOP : WHITE_BISHOP;
    pieces = pos.bishop[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST[stage][index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.bishop[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST[stage][index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    if(verbose){
        printf("After Bishop PST: %d\n", eval_val);
    }
    #endif

    //Rooks
    index = turn ? WHITE_ROOK : BLACK_ROOK;
    index_op = turn ? BLACK_ROOK : WHITE_ROOK;
    pieces = pos.rook[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST[stage][index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.rook[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST[stage][index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    if(verbose){
        printf("After Rook PST: %d\n", eval_val);
    }
    #endif

    //Queens
    index = turn ? WHITE_QUEEN : BLACK_QUEEN;
    index_op = turn ? BLACK_QUEEN : WHITE_QUEEN;
    pieces = pos.queen[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST[stage][index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.queen[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST[stage][index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    if(verbose){
        printf("After Queen PST: %d\n", eval_val);
    }
    #endif

    //King
    index = turn ? WHITE_KING : BLACK_KING; // 5 : 11
    index_op = turn ? BLACK_KING : WHITE_KING;
    pieces = pos.king[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST[stage][index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.king[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST[stage][index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    if(verbose){
        printf("After King PST: %d\n", eval_val);
    }
    #endif

    return eval_val;
}

//Move iterating logic
//First find TTMove, test, and remove from movelist
//Then find killerMoves, test, and remove from movelist
//Then generate values for the remaining moves
//Selection sort remaining moves

#define HIST_SCORE_SHIFT 16
#define HIST_MAX_SCORE  PAWN_VALUE - 100


i32 evalMove(Move move, Position* pos){

    i32 eval = 0;

    // Get the piece information from the move and the position.
    Square fr_sq = GET_FROM(move);
    Square to_sq = GET_TO(move);
    char fr_piece = pos->charBoard[fr_sq];
    char to_piece = pos->charBoard[to_sq];
    i32 fr_piece_i = pieceToIndex[(int)fr_piece];
    i32 to_piece_i = pieceToIndex[(int)to_piece];

    #ifdef DEBUG
    if(fr_piece_i >= 12 || to_piece_i >= 12){
        printf("Warning illegal piece found at:");
        printPosition(*pos, TRUE);
        printf("from piece: %d", pos->charBoard[fr_sq]);
        printf(" to piece: %d", pos->charBoard[to_sq]);
    }
    #endif

    //Add on the PST values
    eval += PST[pos->stage][fr_piece_i][to_sq] - PST[pos->stage][fr_piece_i][fr_sq];
    
    //Add on the flag values
    //i32 histScore;
    switch(GET_FLAGS(move)){
        case QUEEN_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + QUEEN_VALUE;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case ROOK_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + ROOK_VALUE;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case BISHOP_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + BISHOP_VALUE;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case KNIGHT_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + KNIGHT_VALUE;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
            
        case EP_CAPTURE:
            eval += see(*pos, to_sq, ( WHITE_PAWN + ((pos->flags & WHITE_TURN) * 6) ), fr_sq, fr_piece_i);
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;

        case CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i);
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;

        case QUEEN_PROMOTION:
            eval += (queen_value - pawn_value);    
            break;
        case ROOK_PROMOTION:
            eval += (ROOK_VALUE - pawn_value);    
            break;
        case BISHOP_PROMOTION:
            eval += (BISHOP_VALUE - pawn_value);    
            break;
        case KNIGHT_PROMOTION:
            eval += (KNIGHT_VALUE - pawn_value);    
            break;
            
        case QUEEN_CASTLE:
        case KING_CASTLE:
            eval += MOVE_CASTLE_BONUS;
            break;

        default:
            break;
    }

    return eval;
}

/*
 * Verison of move evaluation that is less accurate and faster using only see
 */
void q_evalMoves(Move* moveList, i32* moveVals, i32 size, Position pos){
    for(i32 i = 0; i < size; i++){
        moveVals[i] = 0;
        Move move = moveList[i];
        Square fr_sq = GET_FROM(move);
        Square to_sq = GET_TO(move);

        i32 fr_piece = (i32)pos.charBoard[fr_sq];
        i32 to_piece = (i32)pos.charBoard[to_sq];

        i32 fr_piece_i = pieceToIndex[fr_piece];
        i32 to_piece_i = pieceToIndex[to_piece];

        switch(GET_FLAGS(move)){
            case QUEEN_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + QUEEN_VALUE;
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case ROOK_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + ROOK_VALUE;
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case BISHOP_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + BISHOP_VALUE;
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case KNIGHT_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + KNIGHT_VALUE;
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case EP_CAPTURE:
                moveVals[i] += see(pos, to_sq, ( WHITE_PAWN + ((pos.flags & WHITE_TURN) * 6) ), fr_sq, fr_piece_i);
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i);
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            default:
                break;
        }
    }
    return;
}
