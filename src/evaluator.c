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

i32 PST[3][12][64];

/* Material Values */

const int PawnValue   =   1000;
const int KnightValue =   3500;
const int BishopValue =   3600;
const int RookValue   =   5000;
const int QueenValue  =  10000;
const int KingValue   = 100000;

static const i32 pieceValues[] = {
    [WHITE_PAWN  ] = PawnValue,
    [BLACK_PAWN  ] = PawnValue,
    [WHITE_KNIGHT] = KnightValue,
    [BLACK_KNIGHT] = KnightValue,
    [WHITE_BISHOP] = BishopValue,
    [BLACK_BISHOP] = BishopValue,
    [WHITE_ROOK  ] = RookValue,
    [BLACK_ROOK  ] = RookValue,
    [WHITE_QUEEN ] = QueenValue,
    [BLACK_QUEEN ] = QueenValue,
    [WHITE_KING  ] = KingValue,
    [BLACK_KING  ] = KingValue
};

/* Piece-Square Tables */

const i32 PSTPawn[3][64] = {
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

const i32 PSTKnight[3][64] = {
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

const i32 PSTBishop[3][64] = {
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

const i32 PSTRook[3][64] = {
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

const i32 PSTQueen[3][64] = {
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

i32 PSTKing[3][64] = {
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

void init_pst(){
    for (int i = 0; i < 64; i++) {
        for(int stage = 0; stage < 3; stage++){
            // Pawn
            PST[stage][WHITE_PAWN][i]        = PSTPawn[stage][i];
            PST[stage][BLACK_PAWN][63 - i]   = PSTPawn[stage][i];
            // Knight
            PST[stage][WHITE_KNIGHT][i]      = PSTKnight[stage][i];
            PST[stage][BLACK_KNIGHT][63 - i] = PSTKnight[stage][i];
            // Bishop
            PST[stage][WHITE_BISHOP][i]      = PSTBishop[stage][i];
            PST[stage][BLACK_BISHOP][63 - i] = PSTBishop[stage][i];
            // Rook
            PST[stage][WHITE_ROOK][i]        = PSTRook[stage][i];
            PST[stage][BLACK_ROOK][63 - i]   = PSTRook[stage][i];
            // Queen
            PST[stage][WHITE_QUEEN][i]       = PSTQueen[stage][i];
            PST[stage][BLACK_QUEEN][63 - i]  = PSTQueen[stage][i];
            // King
            PST[stage][WHITE_KING][i]        = PSTKing[stage][i];
            PST[stage][BLACK_KING][63 - i]   = PSTKing[stage][i];
        }
    }
}

i32 quick_eval(Position pos){
    i32 eval_val = 0;
    Turn turn = pos.flags & TURN;
    eval_val += KingValue   * (count_bits(pos.king[turn])   - count_bits(pos.king[!turn]));
    eval_val += QueenValue  * (count_bits(pos.queen[turn])  - count_bits(pos.queen[!turn]));
    eval_val += RookValue   * (count_bits(pos.rook[turn])   - count_bits(pos.rook[!turn]));
    eval_val += BishopValue * (count_bits(pos.bishop[turn]) - count_bits(pos.bishop[!turn]));
    eval_val += KnightValue * (count_bits(pos.knight[turn]) - count_bits(pos.knight[!turn]));
    eval_val += PawnValue   * (count_bits(pos.pawn[turn])   - count_bits(pos.pawn[!turn]));
    return eval_val;
};

u64 getLeastValuablePiece(Position pos, u64 attadef, Turn turn, PieceIndex* piece){
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

   return 0; // None were found
}

/* Static Exchange Evaluator */

i32 see ( Position pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece){
    i32 gain[32], d = 0;
    Turn turn = pos.flags & TURN;
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
    Turn turn = position.flags & TURN;
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

i32 eval(Position pos){
    i32 eval_val = pos.quick_eval;
    Turn turn = pos.flags & TURN;
    i32 stage = pos.stage;
    
    // Insufficient Material Stuff
    if(isInsufficient(pos)) return 0;
    if(TRACE) printf("\nEval for stage: %d, Starting at: %d\n", stage, eval_val);

    //
    // TURN
    //
    if(turn) eval_val += TO_MOVE_BONUS;
    else     eval_val -= TO_MOVE_BONUS;
    if(TRACE) printf("After To Move Bonus: %d\n", eval_val);
 
    //
    // Mobility
    //
    Position tempPos = pos;
    eval_val += eval_mobility(pos);
    makeNullMove(&pos);
    eval_val -= eval_mobility(pos);
    pos = tempPos;
    if(TRACE) printf("After Mobility Bonus: %d\n", eval_val);


    //
    // Can Castle
    //
    if (pos.flags & W_SHORT_CASTLE) eval_val += CAN_CASTLE_BONUS;
    if (pos.flags & W_LONG_CASTLE ) eval_val += CAN_CASTLE_BONUS;
    if (pos.flags & B_SHORT_CASTLE) eval_val -= CAN_CASTLE_BONUS;
    if (pos.flags & B_LONG_CASTLE ) eval_val -= CAN_CASTLE_BONUS;
    if(TRACE) printf("After Can Castle Bonus: %d\n", eval_val);


    //
    // Penalities
    //
    if(pos.flags & IN_CHECK) eval_val -= CHECK_PEN; // Penalty for being in check
    if(TRACE) printf("After Penalties: %d\n", eval_val);

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
    if(TRACE) printf("After Defend Bonus: %d\n", eval_val);


    //
    // Attack
    //
    eval_val += BASE_ATTACK_BONUS * count_bits(pos.color[!turn] &  pos.attack_mask[turn]);
    eval_val -= BASE_ATTACK_BONUS * count_bits(pos.color[turn] &  pos.attack_mask[!turn]);
    if(TRACE) printf("After Attack Bonus: %d\n", eval_val);
 
    //
    // Hanging
    //
    eval_val -= BASE_HANGING_PEN * count_bits((pos.color[turn]  & ~pos.attack_mask[turn])  & pos.attack_mask[!turn]);
    eval_val += BASE_HANGING_PEN * count_bits((pos.color[!turn] & ~pos.attack_mask[!turn]) & pos.attack_mask[turn]);
    if(TRACE) printf("After Hanging Penalty: %d\n", eval_val);

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
    if(TRACE) printf("After Pawn Structure: %d\n", eval_val);

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

    if(TRACE) printf("After Pawn PST: %d\n", eval_val);

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

    if(TRACE) printf("After Knight PST: %d\n", eval_val);

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
    if(TRACE) printf("After Bishop PST: %d\n", eval_val);


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
    if(TRACE) printf("After Rook PST: %d\n", eval_val);

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
    if(TRACE) printf("After Queen PST: %d\n", eval_val);


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
    if(TRACE) printf("After King PST: %d\n", eval_val);

    return eval_val;
}

//Move iterating logic
//First find TTMove, test, and remove from movelist
//Then find killerMoves, test, and remove from movelist
//Then generate values for the remaining moves
//Selection sort remaining moves

i32 move_eval(Move move, Position* pos){

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

    // Add on the PST values
    eval += PST[pos->stage][fr_piece_i][to_sq] - PST[pos->stage][fr_piece_i][fr_sq];
    
    // Add on calculated values depending on the flag
    switch(GET_FLAGS(move)){
        // Promotion Capture Moves
        case QUEEN_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + QueenValue - PawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case ROOK_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + RookValue - PawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case BISHOP_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + BishopValue - PawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case KNIGHT_PROMO_CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + KnightValue - PawnValue;
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        
        // Capture Moves
        case EP_CAPTURE:
            eval += see(*pos, to_sq, ( WHITE_PAWN + ((pos->flags & TURN) * 6) ), fr_sq, fr_piece_i);
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        case CAPTURE:
            eval += see(*pos, to_sq, to_piece_i, fr_sq, fr_piece_i);
            eval += PST[pos->stage][to_piece_i][to_sq];
            break;
        
        // Promotion Moves
        case QUEEN_PROMOTION:
            eval += (QueenValue - PawnValue);    
            break;
        case ROOK_PROMOTION:
            eval += (RookValue - PawnValue);    
            break;
        case BISHOP_PROMOTION:
            eval += (BishopValue - PawnValue);    
            break;
        case KNIGHT_PROMOTION:
            eval += (KnightValue - PawnValue);    
            break;
        
        // Castle Moves
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
void movelist_eval(Move* moveList, i32* moveVals, i32 size, Position pos){
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
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + QueenValue;
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case ROOK_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + RookValue;
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case BISHOP_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + BishopValue;
                moveVals[i] += PST[pos.stage][to_piece_i][to_sq];
                break;
            case KNIGHT_PROMO_CAPTURE:
                moveVals[i] += see(pos, to_sq, to_piece_i, fr_sq, fr_piece_i) + KnightValue;
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
