#include "evaluator.h"
#include "types.h"
#include "util.h"
#include "masks.h"
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

/* King Saftey Values */
const i32 KingPawnDistancePenalty =  -20;
const i32 OpenFileNearKingPenalty = -200;
const i32 VirtKingMobilityPenalty =  -10;

static const int SafetyTable[100] = {
       0,    0,   10,   20,   30,   50,   70,   90,  120,  150,
     180,  220,  260,  300,  350,  390,  440,  500,  560,  620,
     680,  750,  820,  850,  890,  970, 1050, 1130, 1220, 1310,
    1400, 1500, 1690, 1800, 1910, 2020, 2130, 2250, 2370, 2480,
    2600, 2720, 2830, 2950, 3070, 3190, 3300, 3420, 3540, 3660,
    3770, 3890, 4010, 4120, 4240, 4360, 4480, 4590, 4710, 4830,
    4940, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000,
    5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000,
    5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000,
    5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000
};

/* Pawn Specific Values */

const i32 PawnHangingPenalty = -5;

const i32 PawnAttackSquareBonus = 50;

const i32 DoubledPawnPenalty = -100;

const i32 IsolatedPawnPenalty = -50;

const i32 RammedPawnPenalty = -20;

const i32 PassedPawnBonus = 300;

const i32 ConnectedPawnBonus = 100;

/* Knight Specific Values */

const i32 KnightHangingPenalty =  -10;

const i32 KnightMobility[9] = { -1040, -450, -220, -80, 60, 110, 190, 300, 430 };

const i32 KnightAttackedByLesser = -10;

const i32 OutpostKnightBonus = 100;

/* Bishop Specific Values */

const i32 BishopHangingPenalty = -10;

const i32 BishopMobility[14] = { -990, -460, -160, -40, 60, 140, 170, 190, 190, 270, 260, 520, 550, 830 };

const i32 OutpostBishopBonus = 20;

const i32 OppositeBishopBonus = 200;

/* Rook Specific Values */

const i32 RookHangingPenalty = -15;

const i32 RookMobility[15] = { -1270, -560, -250, -120, -100, -120, -110, -40, 40, 90, 110, 190, 190, 370, 970 };

const i32 ConnectedRookBonus = 100;

const i32 TwoRookPenalty = -20;

/* Queen Specific Values */

const i32 QueenHangingPenalty = -20;

const i32 QueenMobility[28] = 
{
    -1110, -2530, -1270, -460, -200, -90, -10,  20,  80, 100, 150, 170,  200,  230,
      220,   210,   240,  160,  130, 180, 250, 380, 340, 280, 100,  70, -420, -230,
};

const i32 QueenPinPenalty = -200;


/* Positional Values */
const i32 CastleAbilityBonus = 60;

/* Returns a material-only based evaluation */
i32 eval_material(Position* pos){
    i32 eval = 0;
    Turn turn = pos->flags & TURN_MASK;
    eval += KingValue   * (count_bits(pos->king[turn])   - count_bits(pos->king[!turn]));
    eval += QueenValue  * (count_bits(pos->queen[turn])  - count_bits(pos->queen[!turn]));
    eval += RookValue   * (count_bits(pos->rook[turn])   - count_bits(pos->rook[!turn]));
    eval += BishopValue * (count_bits(pos->bishop[turn]) - count_bits(pos->bishop[!turn]));
    eval += KnightValue * (count_bits(pos->knight[turn]) - count_bits(pos->knight[!turn]));
    eval += PawnValue   * (count_bits(pos->pawn[turn])   - count_bits(pos->pawn[!turn]));
    return eval;
};

/* 
 * Evaluates a position
 */
i32 eval_position(Position* pos){
    EvalData eval_data = {0};
    Turn turn = pos->flags & TURN_MASK;
    i32 eval = 0;

    // Check for insufficient material
    if(isInsufficient(pos)) return 0;

    eval +=   eval_pawns(pos, &eval_data, WHITE) - eval_pawns(pos, &eval_data, BLACK);
    //printf("Eval after pawns %d\n", eval);

    eval += eval_knights(pos, &eval_data, WHITE) - eval_knights(pos, &eval_data, BLACK);
    //printf("Eval after knights %d\n", eval);

    eval += eval_bishops(pos, &eval_data, WHITE) - eval_bishops(pos, &eval_data, BLACK);
    //printf("Eval after bishops %d\n", eval);

    eval +=   eval_rooks(pos, &eval_data, WHITE) - eval_rooks(pos, &eval_data, BLACK);
    //printf("Eval after rooks %d\n", eval);

    eval +=  eval_queens(pos, &eval_data, WHITE) - eval_queens(pos, &eval_data, BLACK);
    //printf("Eval after queens %d\n", eval);

    eval +=   eval_kings(pos, &eval_data, WHITE) - eval_kings(pos, &eval_data, BLACK);
    //printf("Eval after kings %d\n", eval);

    return turn ? eval : -eval;
}

i32 eval_pawns(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece = turn ? WHITE_PAWN : BLACK_PAWN;
    eval_data->pawn_count[turn] = 0;
    i32 eval = 0;

    // Iterate through the pawns
    u64 pieces = pos->pawn[turn];
    while (pieces) {
        i32 square = getlsb(pieces);
        u32 file = square % 8;
        u32 promo_square = turn ? A8 + file : A1 + file;

        // Material Value
        eval += PawnValue;

        // PST
        eval += PST[stage][piece][square];

        // Passed Pawn Bonus
        if((PassedPawnMask[turn][square] & pos->pawn[!turn]))
            eval += PassedPawnBonus;

        // Doubled Pawn Penalty
        // Applied for the pawn in the back
        if(!(betweenMask[square][promo_square] & pos->pawn[turn]))
            eval += DoubledPawnPenalty;

        // Isolated pawn penalty
        // When there are no pawns on either of the neighboring files
        if(     ( file == 0 && !(fileMask[file + 1] & pos->pawn[turn]) )
            ||  ( file == 7 && !(fileMask[file - 1] & pos->pawn[turn]) )
            ||  ( !(fileMask[file + 1] & pos->pawn[turn] || fileMask[file - 1] & pos->pawn[turn]) ) )
            eval += IsolatedPawnPenalty;
        

        // Update evaluation data
        eval_data->pawn_count[turn]++;

        pieces &= pieces - 1;
    }

    return eval;
}

i32 eval_knights(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece = turn ? WHITE_KNIGHT : BLACK_KNIGHT;
    i32 eval = 0;

   
    u64 pieces = pos->knight[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Knight material value
        // Changes with the number of pawns we have
        eval += KnightValue + KnightAdjust[eval_data->pawn_count[turn]];

        // PST Values
        eval += PST[stage][piece][square];

        // Knight Outpost Bonus

        // Knight Forking bonus

        pieces &= pieces - 1;
    }


    return eval;
}

i32 eval_bishops(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece = turn ? WHITE_BISHOP : BLACK_BISHOP;
    i32 eval = 0, light_bishops = 0, dark_bishops = 0;

    
    u64 pieces = pos->bishop[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Material Value
        eval += BishopValue;

        // PST Values
        eval += PST[stage][piece][square];

        // Give a bonus if the bishops square colors is opposite that of the majority of pawns
        
        //Update evaluation info
        if(is_square_light(square)) light_bishops++;
        else                         dark_bishops++;

        pieces &= pieces - 1;
    }


    // Give a bonus if there are bishops on opposite colors
    if (light_bishops >= 1 && dark_bishops >= 1) eval += OppositeBishopBonus;

    return eval;
}

i32 eval_rooks(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece = turn ? WHITE_ROOK : BLACK_ROOK;
    i32 eval = 0;

    // PST Values
    u64 pieces = pos->rook[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Rook material value
        // Changes with the number of pawns we have
        eval += RookValue + RookAdjust[eval_data->pawn_count[turn]];

        eval += PST[stage][piece][square];
        pieces &= pieces - 1;
    }

    // Connected Rook Bonus
    // Given if one of the rooks is attacking the other
    u64 rook1 = pos->rook[turn] & -pos->rook[turn];
    if(getRookAttacks(rook1, pos->color[turn], pos->color[!turn]) & (pos->rook[turn] & ~rook1))  eval += ConnectedRookBonus;

    // Double Rook Penalty
    if (count_bits(pos->rook[turn]) >= 2) eval += TwoRookPenalty;

    return eval;
}

i32 eval_queens(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece = turn ? WHITE_QUEEN : BLACK_QUEEN;
    i32 eval = 0;
    
    u64 pieces = pos->queen[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Material Value
        eval += QueenValue;

        // PST Values
        eval += PST[stage][piece][square];

        pieces &= pieces - 1;
    }

    return eval;
}

i32 eval_kings(Position * pos, EvalData* eval_data, Turn turn){
    const Stage stage = pos->stage;
    const PieceIndex piece = turn ? WHITE_KING : BLACK_KING;
    const u32 square = getlsb(pos->king[turn]);
    i32 eval = 0;

    // PST Values
    eval += PST[stage][piece][square];

    // Mobility
    i32 move_count = 0;
    u64 king_move_sq = getKingMoves(pos, turn, &move_count);
    if (turn  && pos->flags & W_SHORT_CASTLE) move_count++;
    if (turn  && pos->flags & W_LONG_CASTLE ) move_count++;
    if (!turn && pos->flags & B_SHORT_CASTLE) move_count++;
    if (!turn && pos->flags & B_LONG_CASTLE ) move_count++;

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