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

#define PST_PAWN_MULT_OPN       7  // PST Mult Pawns in Eval
#define PST_PAWN_MULT_MID       5  // PST Mult Pawns in Eval
#define PST_PAWN_MULT_END       4  // PST Mult Pawns in Eval

#define PST_KNIGHT_MULT_OPN    70  // PST Mult Knights in Eval
#define PST_KNIGHT_MULT_MID    50  // PST Mult Knights in Eval
#define PST_KNIGHT_MULT_END    25  // PST Mult Knights in Eval

#define PST_BISHOP_MULT_OPN    70  // PST Mult Bishop in Eval
#define PST_BISHOP_MULT_MID    50  // PST Mult Bishop in Eval
#define PST_BISHOP_MULT_END    25  // PST Mult Bishop in Eval

#define PST_QUEEN_MULT_OPN     10  // PST Mult Queen in Eval
#define PST_QUEEN_MULT_MID     30  // PST Mult Queen in Eval
#define PST_QUEEN_MULT_END     20  // PST Mult Queen in Eval

#define PST_ROOK_MULT_OPN      10  // PST Mult Rook in Eval
#define PST_ROOK_MULT_MID      40  // PST Mult Rook in Eval
#define PST_ROOK_MULT_END      20  // PST Mult Rook in Eval

#define PST_KING_MULT_OPN      20  // PST Mult King in Eval
#define PST_KING_MULT_MID      40  // PST Mult King in Eval
#define PST_KING_MULT_END      30  // PST Mult King in Eval

// Defines for Movement Eval
#define MOVE_CASTLE_BONUS  300  // Eval bonus for castling
#define MOVE_OPN_QUEEN_PEN   5  // Pen for moving queen in the opening


static i32 PST[3][12][64];

static const i32 pieceValues[] = {
    [WHITE_PAWN] = PAWN_VALUE,
    [WHITE_KNIGHT] = KNIGHT_VALUE,
    [WHITE_BISHOP] = BISHOP_VALUE,
    [WHITE_ROOK] = ROOK_VALUE,
    [WHITE_QUEEN] = QUEEN_VALUE,
    [WHITE_KING] = KING_VALUE,
    [BLACK_PAWN] = PAWN_VALUE,
    [BLACK_KNIGHT] = KNIGHT_VALUE,
    [BLACK_BISHOP] = BISHOP_VALUE,
    [BLACK_ROOK] = ROOK_VALUE,
    [BLACK_QUEEN] = QUEEN_VALUE,
    [BLACK_KING] = KING_VALUE
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


// Here be the static exchange evaluator
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
        //if (-gain[d-1] < 0 && gain[d] < 0) break; // Pruning
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


/*
*  Below here is the move evaluating code
*/

void initPST(){


    // Pawns
    i32 PST_PAWN_OPN[64] = {
         0,   0,   0,   0,   0,   0,   0,   0,
        10,   0,   0, -50, -50,   0,   0,  10,
         0,  30,  30,  10,  10,  30,  30,   0,
         0,   0,   0,  40,  40,   0,   0,   0,
         0,   0,   0,   2,   2,   0,   0,   0,
         1,   1,   2,   3,   3,   2,   1,   1,
         5,   5,   5,   5,   5,   5,   5,   5,
         0,   0,   0,   0,   0,   0,   0,   0
    };

    i32 PST_PAWN_MID[64] = {
         0,   0,   0,   0,   0,   0,   0,   0,
         0,   0,   0, -10, -10,   0,   0,   0,
        10,  20,  20,  10,  10,  20,  20,  10,
         0,  10,  10,  30,  30,  10,  10,   0,
         0,   0,   0,  30,  30,   0,   0,   0,
        10,  10,  20,  40,  40,  20,  10,  10,
        50,  50,  50,  50,  50,  50,  50,  50,
         0,   0,   0,   0,   0,   0,   0,   0
    };

    i32 PST_PAWN_END[64] = {
         0,   0,   0,   0,   0,   0,   0,   0,
       -10, -10, -10, -10, -10, -10, -10, -10,
        -5,  -5,  -5,  -5,  -5,  -5,  -5,  -5,
         0,   0,   0,   0,   0,   0,   0,   0,
        30,  30,  30,  30,  30,  30,  30,  30,
        70,  70,  70,  70,  70,  70,  70,  70,
        90,  90,  90,  90,  90,  90,  90,  90,
         0,   0,   0,   0,   0,   0,   0,   0
    };


    // Knights
    i32 PST_KNIGHT_OPN[64] = {
        -5, -4, -3, -3, -3, -3, -4, -5,
        -4, -2,  0,  2,  2,  0, -2, -4,
        -3,  0,  3,  3,  3,  3,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -5, -4, -3, -3, -3, -3, -4, -5
    };

    i32 PST_KNIGHT_MID[64] = {
        -5, -4, -3, -3, -3, -3, -4, -5,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -5, -4, -3, -3, -3, -3, -4, -5
    };

    i32 PST_KNIGHT_END[64] = {
        -5, -4, -3, -3, -3, -3, -4, -5,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -5, -4, -3, -3, -3, -3, -4, -5
    };

    // Bishops
    i32 PST_BISHOP_OPN[64] = {
         1, -1, -1, -1, -1, -1, -1,  1,
        -1,  4,  0,  0,  0,  0,  4, -1,
        -1,  0,  2,  4,  4,  2,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  1,  0,  0,  0,  0,  1, -1,
         1, -1, -1, -1, -1, -1, -1,  1
    };

    i32 PST_BISHOP_MID[64] = {
         1, -1, -1, -1, -1, -1, -1,  1,
        -1,  1,  0,  0,  0,  0,  1, -1,
        -1,  0,  1,  2,  2,  1,  0, -1,
        -1,  0,  1,  2,  2,  1,  0, -1,
        -1,  0,  2,  2,  2,  2,  0, -1,
        -1,  2,  2,  2,  2,  2,  2, -1,
        -1,  1,  0,  0,  0,  0,  1, -1,
         1, -1, -1, -1, -1, -1, -1,  1
    };

    i32 PST_BISHOP_END[64] = {
         2, -1, -1, -1, -1, -1, -1,  2,
        -1,  2,  0,  0,  0,  0,  2, -1,
        -1,  0,  2,  1,  1,  2,  0, -1,
        -1,  0,  1,  2,  2,  1,  0, -1,
        -1,  0,  1,  2,  2,  1,  0, -1,
        -1,  0,  2,  1,  1,  2,  0, -1,
        -1,  2,  0,  0,  0,  0,  2, -1,
         2, -1, -1, -1, -1, -1, -1,  2
    };


    // Rooks
    i32 PST_ROOK_OPN[64] = {
        0,  0,  0,  2,  2,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        1,  2,  2,  2,  2,  2,  2,  1,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    i32 PST_ROOK_MID[64] = {
        0,  0,  0,  1,  1,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        1,  2,  2,  2,  2,  2,  2,  1,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    i32 PST_ROOK_END[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0
    };


    //Queens
    i32 PST_QUEEN_OPN[64] = {
        -2, -1, -1, -1, -1, -1, -1, -2,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  0,  1,  1,  0,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -2, -1, -1, -1, -1, -1, -1, -2
    };

    i32 PST_QUEEN_MID[64] = {
        -2, -1, -1, -1, -1, -1, -1, -2,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  2,  2,  1,  0, -1,
        -1,  0,  1,  2,  2,  1,  0, -1,
        -1,  1,  1,  1,  1,  1,  1, -1,
        -1,  0,  1,  0,  0,  1,  0, -1,
        -2, -1, -1, -1, -1, -1, -1, -2
    };

    i32 PST_QUEEN_END[64] = {
        -2, -1, -1, -1, -1, -1, -1, -2,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -2, -1, -1, -1, -1, -1, -1, -2
    };


    //King
    i32 PST_KING_OPN[64] = {
         4,  4, -1,  0,  0, -1,  4,  4,
         2,  2,  0,  0,  0,  0,  2,  2,
        -1, -2, -2, -2, -2, -2, -2, -1,
        -2, -3, -3, -4, -4, -3, -3, -2,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -5, -5, -5, -5, -4, -3
    };

    i32 PST_KING_MID[64] = {
        3,   4,  2,  0,  0,  2,  4,  3,
        1,   1,  0,  0,  0,  0,  1,  1,
        -1, -2, -2, -2, -2, -2, -2, -1,
        -2, -3, -3, -4, -4, -3, -3, -2,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3
    };

    i32 PST_KING_END[64] = {
        -5, -4, -3, -2, -2, -3, -4, -5,
        -3, -2, -1,  0,  0, -1, -2, -3,
        -3, -1,  2,  3,  3,  2, -1, -3,
        -3, -1,  3,  4,  4,  3, -1, -3,
        -3, -1,  3,  4,  4,  3, -1, -3,
        -3, -1,  2,  3,  3,  2, -1, -3,
        -3, -3,  0,  0,  0,  0, -3, -3,
        -5, -3, -3, -3, -3, -3, -3, -5
    };


    // Initialize PST for white pieces
    for (i32 i = 0; i < 64; i++) {
        // Pawns
        PST[OPN_GAME][WHITE_PAWN][i] = PST_PAWN_OPN[i] * PST_PAWN_MULT_OPN;
        PST[MID_GAME][WHITE_PAWN][i] = PST_PAWN_MID[i] * PST_PAWN_MULT_MID;
        PST[END_GAME][WHITE_PAWN][i] = PST_PAWN_END[i] * PST_PAWN_MULT_END;

        PST[OPN_GAME][BLACK_PAWN][63 - i] = PST_PAWN_OPN[i] * PST_PAWN_MULT_OPN;
        PST[MID_GAME][BLACK_PAWN][63 - i] = PST_PAWN_MID[i] * PST_PAWN_MULT_MID;
        PST[END_GAME][BLACK_PAWN][63 - i] = PST_PAWN_END[i] * PST_PAWN_MULT_END;

        // Knights
        PST[OPN_GAME][WHITE_KNIGHT][i] = PST_KNIGHT_OPN[i] * PST_KNIGHT_MULT_OPN;
        PST[MID_GAME][WHITE_KNIGHT][i] = PST_KNIGHT_MID[i] * PST_KNIGHT_MULT_MID;
        PST[END_GAME][WHITE_KNIGHT][i] = PST_KNIGHT_END[i] * PST_KNIGHT_MULT_END;

        PST[OPN_GAME][BLACK_KNIGHT][63 - i] = PST_KNIGHT_OPN[i] * PST_KNIGHT_MULT_OPN;
        PST[MID_GAME][BLACK_KNIGHT][63 - i] = PST_KNIGHT_MID[i] * PST_KNIGHT_MULT_MID;
        PST[END_GAME][BLACK_KNIGHT][63 - i] = PST_KNIGHT_END[i] * PST_KNIGHT_MULT_END;

        // Bishops
        PST[OPN_GAME][WHITE_BISHOP][i] = PST_BISHOP_OPN[i] * PST_BISHOP_MULT_OPN;
        PST[MID_GAME][WHITE_BISHOP][i] = PST_BISHOP_MID[i] * PST_BISHOP_MULT_MID;
        PST[END_GAME][WHITE_BISHOP][i] = PST_BISHOP_END[i] * PST_BISHOP_MULT_END;

        PST[OPN_GAME][BLACK_BISHOP][63 - i] = PST_BISHOP_OPN[i] * PST_BISHOP_MULT_OPN;
        PST[MID_GAME][BLACK_BISHOP][63 - i] = PST_BISHOP_MID[i] * PST_BISHOP_MULT_MID;
        PST[END_GAME][BLACK_BISHOP][63 - i] = PST_BISHOP_END[i] * PST_BISHOP_MULT_END;

        // Rooks
        PST[OPN_GAME][WHITE_ROOK][i] = PST_ROOK_OPN[i] * PST_ROOK_MULT_OPN;
        PST[MID_GAME][WHITE_ROOK][i] = PST_ROOK_MID[i] * PST_ROOK_MULT_MID;
        PST[END_GAME][WHITE_ROOK][i] = PST_ROOK_END[i] * PST_ROOK_MULT_END;

        PST[OPN_GAME][BLACK_ROOK][63 - i] = PST_ROOK_OPN[i] * PST_ROOK_MULT_OPN;
        PST[MID_GAME][BLACK_ROOK][63 - i] = PST_ROOK_MID[i] * PST_ROOK_MULT_MID;
        PST[END_GAME][BLACK_ROOK][63 - i] = PST_ROOK_END[i] * PST_ROOK_MULT_END;

        // Queens
        PST[OPN_GAME][WHITE_QUEEN][i] = PST_QUEEN_OPN[i] * PST_QUEEN_MULT_OPN;
        PST[MID_GAME][WHITE_QUEEN][i] = PST_QUEEN_MID[i] * PST_QUEEN_MULT_MID;
        PST[END_GAME][WHITE_QUEEN][i] = PST_QUEEN_END[i] * PST_QUEEN_MULT_END;

        PST[OPN_GAME][BLACK_QUEEN][63 - i] = PST_QUEEN_OPN[i] * PST_QUEEN_MULT_OPN;
        PST[MID_GAME][BLACK_QUEEN][63 - i] = PST_QUEEN_MID[i] * PST_QUEEN_MULT_MID;
        PST[END_GAME][BLACK_QUEEN][63 - i] = PST_QUEEN_END[i] * PST_QUEEN_MULT_END;

        // King
        PST[OPN_GAME][WHITE_KING][i] = PST_KING_OPN[i] * PST_KING_MULT_OPN;
        PST[MID_GAME][WHITE_KING][i] = PST_KING_MID[i] * PST_KING_MULT_MID;
        PST[END_GAME][WHITE_KING][i] = PST_KING_END[i] * PST_KING_MULT_END;

        PST[OPN_GAME][BLACK_KING][63 - i] = PST_KING_OPN[i] * PST_KING_MULT_OPN;
        PST[MID_GAME][BLACK_KING][63 - i] = PST_KING_MID[i] * PST_KING_MULT_MID;
        PST[END_GAME][BLACK_KING][63 - i] = PST_KING_END[i] * PST_KING_MULT_END;

    }

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

    Square fr_sq = GET_FROM(move);
    Square to_sq = GET_TO(move);
    i32 fr_piece = (i32)pos->charBoard[fr_sq];
    i32 to_piece = (i32)pos->charBoard[to_sq];
    i32 fr_piece_i = pieceToIndex[fr_piece];
    i32 to_piece_i = pieceToIndex[to_piece];

    if(pos->stage == OPN_GAME && (fr_piece == WHITE_QUEEN || fr_piece == BLACK_QUEEN)) eval -= MOVE_OPN_QUEEN_PEN;

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
            eval += (QUEEN_VALUE - PAWN_VALUE);    
            break;
        case ROOK_PROMOTION:
            eval += (ROOK_VALUE - PAWN_VALUE);    
            break;
        case BISHOP_PROMOTION:
            eval += (BISHOP_VALUE - PAWN_VALUE);    
            break;
        case KNIGHT_PROMOTION:
            eval += (KNIGHT_VALUE - PAWN_VALUE);    
            break;
            
        case QUEEN_CASTLE:
        case KING_CASTLE:
            eval += MOVE_CASTLE_BONUS;
            break;

        case DOUBLE_PAWN_PUSH:
        case QUIET:
            //histScore = getHistoryScore(pos.flags, moveList[i]) >> HIST_SCORE_SHIFT;
            //moveVals[i] += MIN(histScore, HIST_MAX_SCORE);
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
