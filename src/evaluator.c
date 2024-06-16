#include "evaluator.h"
#include "types.h"
#include "util.h"
#include "tables.h"
#include "movement.h"
#include "bitboard/bbutils.h"
#include "bitboard/bitboard.h"

i32 PST[3][12][64];

/* Material Values */

const i32 PawnValue   =   1000;
const i32 KnightValue =   3500;
const i32 BishopValue =   3600;
const i32 RookValue   =   5000;
const i32 QueenValue  =  10000;
const i32 KingValue   = 100000;

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

/* Adjustment of piece values based on the number of pawns */
i32 KnightAdjust[9] = { -200, -160, -120, -80, -40,  0,  40,  80, 120 };
i32   RookAdjust[9] = { 150,   120,   90,  60,  30,  0, -30, -60, -90 };

static const int SafetyTable[100] = {
  0,  0,   1,   2,   3,   5,   7,   9,  12,  15,
  18,  22,  26,  30,  35,  39,  44,  50,  56,  62,
  68,  75,  82,  85,  89,  97, 105, 113, 122, 131,
  140, 150, 169, 180, 191, 202, 213, 225, 237, 248,
  260, 272, 283, 295, 307, 319, 330, 342, 354, 366,
  377, 389, 401, 412, 424, 436, 448, 459, 471, 483,
  494, 500, 500, 500, 500, 500, 500, 500, 500, 500,
  500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
  500, 500, 500, 500, 500, 500, 500, 500, 500, 500,
  500, 500, 500, 500, 500, 500, 500, 500, 500, 500
};

/* Mobility Bonus Values */
const i32 PawnMobilityBonus   =  10;
const i32 KnightMobilityBonus =  30;
const i32 BishopMobilityBonus =  35;
const i32 RookMobilityBonus   =  50;
const i32 QueenMobilityBonus  =  60;
const i32 KingMobilityBonus   =  90;

/* Hanging Penalty Values */
const i32 PawnHangingPenalty   =   -5;
const i32 KnightHangingPenalty =  -10;
const i32 BishopHangingPenalty =  -10;
const i32 RookHangingPenalty   =  -15;
const i32 QueenHangingPenalty  =  -20;
const i32 KingHangingPenalty   =    0;

/* Bonus for number of squares under attack */
const i32 PawnAttackSquareBonus   = 50;
const i32 KnightAttackSquareBonus = 25;
const i32 BishopAttackSquareBonus = 10;
const i32 RookAttackSquareBonus   =  5;
const i32 QueenAttackSquareBonus  =  1;
const i32 KingAttackSquareBonus   =  0;

/* Penalties for being attacked by a lesser piece */
const i32 KnightAttackedByLesser = -10;
const i32 BishopAttackedByLesser = -10;
const i32 RookAttackedByLesser   = -30;
const i32 QueenAttackedByLesser  = -100;

/* Pawn Specific Values */
const i32 DoubledPawnPenalty  = -100;
const i32 IsolatedPawnPenalty =  -50;
const i32 RammedPawnPenalty   =  -20; 
const i32 PassedPawnBonus     =  300;

/* Knight Specific Values */
const i32 OutpostKnightBonus = 100;

/* Bishop Specific Values */
const i32 OutpostBishopBonus  =  20;
const i32 OppositeBishopBonus = 200;

/* Rook Specific Values */
const i32 ConnectedRookBonus  = 100;
const i32 TwoRookPenalty      = -20;

/* Queen Specific Values */

/* King Saftey Values */
const i32 KingPawnDistancePenalty = -20;

/* Positional Values */
const i32 CastleAbilityBonus = 60;

/* Returns a material-only based evaluation */
i32 eval_material(Position* pos){
    i32 eval_val = 0;
    Turn turn = pos->flags & TURN_MASK;
    eval_val += KingValue   * (count_bits(pos->king[turn])   - count_bits(pos->king[!turn]));
    eval_val += QueenValue  * (count_bits(pos->queen[turn])  - count_bits(pos->queen[!turn]));
    eval_val += RookValue   * (count_bits(pos->rook[turn])   - count_bits(pos->rook[!turn]));
    eval_val += BishopValue * (count_bits(pos->bishop[turn]) - count_bits(pos->bishop[!turn]));
    eval_val += KnightValue * (count_bits(pos->knight[turn]) - count_bits(pos->knight[!turn]));
    eval_val += PawnValue   * (count_bits(pos->pawn[turn])   - count_bits(pos->pawn[!turn]));
    return eval_val;
};

/* 
 * Evaluates a position
 */
i32 eval_position(Position* pos){
    Turn turn = pos->flags & TURN_MASK;
    i32 stage = pos->stage;
    EvalData eval_data = {0};
    // Start with the material
    i32 eval_val = 0;

    // Check for insufficient material
    if(isInsufficient(pos)) return 0;

    eval_val +=   eval_pawns(pos, &eval_data, WHITE) -   eval_pawns(pos, &eval_data, BLACK);
    eval_val += eval_knights(pos, &eval_data, WHITE) - eval_knights(pos, &eval_data, BLACK);
    eval_val += eval_bishops(pos, &eval_data, WHITE) - eval_bishops(pos, &eval_data, BLACK);
    eval_val +=   eval_rooks(pos, &eval_data, WHITE) -   eval_rooks(pos, &eval_data, BLACK);
    eval_val +=  eval_queens(pos, &eval_data, WHITE) -  eval_queens(pos, &eval_data, BLACK);
    eval_val +=   eval_kings(pos, &eval_data, WHITE) -   eval_pawns(pos, &eval_data, BLACK);

    return eval_val;
}

i32 eval_pawns(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece    = turn ? WHITE_PAWN : BLACK_PAWN;
    const PieceIndex piece_op = turn ? BLACK_PAWN : WHITE_PAWN;
    eval_data->pawn_count = 0;
    i32 eval = 0;

    // Iterate through the pawns
    u64 pieces = pos->pawn[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Material Value
        eval += PawnValue;

        // PST
        eval += PST[stage][piece][square];

        // Mobility Bonus
        getPawnMovesAppend(u64 pawns, u64 ownPieces, u64 oppPieces, u64 enPassant, char flags, Move *moveList, i32 *idx)

        // Update evaluation data
        eval_data->pawn_count++;

        pieces &= pieces - 1;
    }

    


    return eval;
}

i32 eval_knights(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece    = turn ? WHITE_KNIGHT : BLACK_KNIGHT;
    const PieceIndex piece_op = turn ? BLACK_KNIGHT : WHITE_KNIGHT;
    i32 eval = 0;

    // PST Values
    u64 pieces = pos->knight[turn];
    while (pieces) {
        i32 square = getlsb(pieces);
        eval += PST[stage][piece][square];
        pieces &= pieces - 1;
    }


    return eval;
}

i32 eval_bishops(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece    = turn ? WHITE_BISHOP : BLACK_BISHOP;
    const PieceIndex piece_op = turn ? BLACK_BISHOP : WHITE_BISHOP;
    i32 eval = 0;

    // PST Values
    u64 pieces = pos->bishop[turn];
    while (pieces) {
        i32 square = getlsb(pieces);
        eval += PST[stage][piece][square];
        pieces &= pieces - 1;
    }

    if (count_bits(pos->bishop[turn]) == 2)  eval += OppositeBishopBonus;

    return eval;
}

i32 eval_rooks(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece    = turn ? WHITE_ROOK : BLACK_ROOK;
    const PieceIndex piece_op = turn ? BLACK_ROOK : WHITE_ROOK;
    i32 eval = 0;

    // PST Values
    u64 pieces = pos->rook[turn];
    while (pieces) {
        i32 square = getlsb(pieces);
        eval += PST[stage][piece][square];
        pieces &= pieces - 1;
    }

    // Connected Rook Bonus
    u64 rook1 = pos->rook[turn] & -pos->rook[turn];
    if(getRookAttacks(rook1, pos->color[turn], pos->color[!turn]) & (pos->rook[turn] & ~rook1))  eval += ConnectedRookBonus;

    // Double Rook Penalty
    if (count_bits(pos->rook[turn]) >= 2) eval -= TwoRookPenalty;

    return eval;
}

i32 eval_queens(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece    = turn ? WHITE_QUEEN : BLACK_QUEEN;
    const PieceIndex piece_op = turn ? BLACK_QUEEN : WHITE_QUEEN;
    i32 eval = 0;

    // PST Values
    u64 pieces = pos->queen[turn];
    while (pieces) {
        i32 square = getlsb(pieces);
        eval += PST[stage][piece][square];
        pieces &= pieces - 1;
    }

    return eval;
}

i32 eval_kings(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece    = turn ? WHITE_KING : BLACK_KING;
    const PieceIndex piece_op = turn ? BLACK_KING : WHITE_KING;
    const u32 square = getlsb(pos->king[turn]);
    i32 eval = 0;

    // PST Values
    eval += PST[stage][piece][square];

    // Mobility
    i32 move_count;
    u64 king_move_sq = getKingMoves(pos, turn, &move_count);
    if (turn  && pos->flags & W_SHORT_CASTLE) move_count++;
    if (turn  && pos->flags & W_LONG_CASTLE ) move_count++;
    if (!turn && pos->flags & B_SHORT_CASTLE) move_count++;
    if (!turn && pos->flags & B_LONG_CASTLE ) move_count++;

    eval += move_count * KingMobilityBonus;

    // Hanging
    if(pos->king[turn] & pos->attack_mask[turn]) eval += KingHangingPenalty;

    // Nearby Enemies
    // if(NearbyMask[square] & pos->queen[!turn]) eval += EnemyNearKingPenalty;


    return eval;
}

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