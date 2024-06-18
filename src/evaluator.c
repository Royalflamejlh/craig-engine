#include "evaluator.h"
#include "bitboard/magic.h"
#include "types.h"
#include "util.h"
#include "masks.h"
#include "movement.h"
#include "bitboard/bbutils.h"
#include "bitboard/bitboard.h"

i32 PST[2][12][64];

/* Phase Information */
enum Phase{
    PHASE_MG = 0,
    PHASE_EG = 1,

    MINOR_PHASE = 1,
    ROOK_PHASE  = 2,
    QUEEN_PHASE = 4,
    TOTAL_PHASE_VALUE = 24
};


/* Material Values */

const i32 PawnValue   =   1000;
const i32 KnightValue =   3500;
const i32 BishopValue =   3600;
const i32 RookValue   =   5000;
const i32 QueenValue  =  10000;
const i32 KingValue   = 100000;

/* Piece-Square Tables */

const i32 PSTPawn[2][64] = {
    [PHASE_MG] =
    {
          0,   0,   0,   0,   0,   0,   0,   0,
         10,   0,   0, -10, -10,   0,   0,  10,
         10,  20,  20,  20,  20,  20,  20,  10,
          0,  10,  10,  30,  30,  10,  10,   0,
          0,   0,   0,  30,  30,   0,   0,   0,
         30,  30,  30,  40,  40,  30,  30,  30,
         90,  90,  90,  90,  90,  90,  90,  90,
          0,   0,   0,   0,   0,   0,   0,   0
    },
    [PHASE_EG] =
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

const i32 PSTKnight[2][64] = {
    [PHASE_MG] = 
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
    [PHASE_EG] =
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

const i32 PSTBishop[2][64] = {
    [PHASE_MG] = 
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
    [PHASE_EG] =
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

const i32 PSTRook[2][64] = {
    [PHASE_MG] =
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
    [PHASE_EG] =
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

const i32 PSTQueen[2][64] = {
    [PHASE_MG] =
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
    [PHASE_EG] =
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

const i32 PSTKing[2][64] = {
    [PHASE_MG] =
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
    [PHASE_EG] =
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
const i32 KnightAdjust[9] = { -200, -160, -120, -80, -40,  0,  40,  80, 120 };
const i32   RookAdjust[9] = {  150,  120,   90,  60,  30,  0, -30, -60, -90 };

/* King Saftey Values */
const i32 KingPawnDistancePenalty[2] = { -20, -200 };

const i32 OpenFileNearKingPenalty[2] = { -200, -100 };

const i32 VirtualMobility[2][28] = {
    {
        0,    0,    0,    0,    0,    0,    0,  -50, -100, -125, -150, -175,  -200,  -250,
        -300, -350, -400, -500, -500, -500, -500, -500, -500, -500, -500,  -500, -500, -500,
    },
    {
          0,    0,    0,    0,    0,    0,    0,  -5, -15, -20, -30, -40, -50, -60,
        -70,  -70,  -70,  -70,  -70,  -70,  -70, -70, -70, -70, -70, -70, -70, -70,
    },
};

const i32 PawnsInKingArea[2][9] = {
    {  -50, -10, 25,  20,  30,  30,  30,  30,  30 },
    { -100, -50,  0,   0,   0,   0,   0,   0,   0 },
};

enum AttackUnits {
    ATTACK_UNIT_PAWN   = 2,
    ATTACK_UNIT_KNIGHT = 2,
    ATTACK_UNIT_BISHOP = 2,
    ATTACK_UNIT_ROOK   = 5,
    ATTACK_UNIT_QUEEN  = 6,
};

const i32 SafetyTable[2][100] = {
    [PHASE_MG] =
    {
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
    },
    [PHASE_EG] =
    {
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
          0,   0,   0,   1,   1,   2,   3,   4,   5,   6,
          8,  10,  13,  16,  20,  25,  30,  36,  42,  48,
         55,  62,  70,  80,  90,  90,  90,  90,  90,  90,  
         90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  
         90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  
         90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  
         90,  90,  90,  90,  90,  90,  90,  90,  90,  90,  
         90,  90,  90,  90,  90,  90,  90,  90,  90,  90,
    }
};

/* Pawn Specific Values {Mid game, End game} */

const i32 PawnHangingPenalty[2] = { -10, 0 };

const i32 DoubledPawnPenalty[2] = { -20, -200 };

const i32 IsolatedPawnPenalty[2] = { -50, -100 };

const i32 RammedPawnPenalty[2] = { -20, -100 };

const i32 PassedPawnBonus[2] = { 25, 500 };

const i32 ConnectedPawnBonus[2] = { 20, 60 };

/* Knight Specific Values {Mid game, End game} */

const i32 KnightHangingPenalty[2] =  { -20, 0 };

const i32 KnightMobility[2][9] = 
{
    {  -104,  -45,  -22,  -8,  6,  11,  19,  30,  43 },
    { -1040, -450, -220, -80, 60, 110, 190, 300, 430 }
};

const i32 OutpostKnightBonus[2] = { 50, 0 };

const i32 OutpostKnightExtraBonus[2] = { 100, 0 };

/* Bishop Specific Values */

const i32 BishopHangingPenalty[2] = { -10, 0 };

const i32 BishopMobility[2][14] = 
{
    {  -99,  -46,  -16,  -4,  6,  14,  17,  19,  19 , 27 , 26,  52,  55,  83 },
    { -990, -460, -160, -40, 60, 140, 170, 190, 190, 270, 260, 520, 550, 830 },
};

const i32 OutpostBishopBonus[2] = { 25, 0 };

const i32 OutpostBishopExtraBonus[2] = { 30, 0 };

const i32 OppositeBishopBonus[2] = { 75, 300 };

const i32 BishopPawnWeakPenalty[2] = { -75, 0 };

/* Rook Specific Values */

const i32 RookHangingPenalty[2] = { -15, 0 };

const i32 RookMobility[2][15] = 
{
    {  -127,  -56,  -25,  -12,  -10,  -12,  -11,  -4,  4,  9,  11,  19,  19,  37,  97 },
    { -1270, -560, -250, -120, -100, -120, -110, -40, 40, 90, 110, 190, 190, 370, 970 },
};

const i32 ConnectedRookBonus[2] = { 15, 0 };

const i32 TwoRookPenalty[2] = { -5, -35 };

/* Queen Specific Values */

const i32 QueenHangingPenalty[2] = { -60, 0 };

const i32 QueenMobility[2][28] = 
{
    {
        -111, -111, -104, -46, -20, -9, -1,  2,  8, 10, 15, 17,  20,  23,
        22,   21,   24,  16,  13, 18, 25, 38, 34, 28, 10,  7, -42, -23,
    },
    {
        -1110, -2530, -1270, -460, -200, -90, -10,  20,  80, 100, 150, 170,  200,  230,
        220,   210,   240,  160,  130, 180, 250, 380, 340, 280, 100,  70, -420, -230,
    },
};

const i32 QueenPinPenalty[2] = { -100, -300};

/* Positional Values */
const i32 CastleAbilityBonus[2] = { 0, 0 };

void init_pst(){
    for (int i = 0; i < 64; i++) {
        for(int phase = 0; phase < 2; phase++){
            // Pawn
            PST[phase][WHITE_PAWN][i]        = PSTPawn[phase][i];
            PST[phase][BLACK_PAWN][63 - i]   = PSTPawn[phase][i];
            // Knight
            PST[phase][WHITE_KNIGHT][i]      = PSTKnight[phase][i];
            PST[phase][BLACK_KNIGHT][63 - i] = PSTKnight[phase][i];
            // Bishop
            PST[phase][WHITE_BISHOP][i]      = PSTBishop[phase][i];
            PST[phase][BLACK_BISHOP][63 - i] = PSTBishop[phase][i];
            // Rook
            PST[phase][WHITE_ROOK][i]        = PSTRook[phase][i];
            PST[phase][BLACK_ROOK][63 - i]   = PSTRook[phase][i];
            // Queen
            PST[phase][WHITE_QUEEN][i]       = PSTQueen[phase][i];
            PST[phase][BLACK_QUEEN][63 - i]  = PSTQueen[phase][i];
            // King
            PST[phase][WHITE_KING][i]        = PSTKing[phase][i];
            PST[phase][BLACK_KING][63 - i]   = PSTKing[phase][i];
        }
    }
}

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


void init_eval_data(Position * pos, EvalData* eval_data, Turn turn){
    // Get the saftey region for the king
    eval_data->king_area[turn] = KingAreaMask[getlsb(pos->king[turn])];
}

void eval_pawns(Position * pos, EvalData* eval_data, Turn turn){
    const PieceIndex piece = turn ? WHITE_PAWN : BLACK_PAWN;
    eval_data->pawn_count[turn] = 0;

    // Iterate through the pawns
    u64 pieces = pos->pawn[turn];
    while (pieces) {
        i32 square = getlsb(pieces);
        u32 file = square % 8;
        u32 promo_square = turn ? A8 + file : A1 + file;
        u64 attacks = pawnAttacks(square, turn);

        // Material Value
        eval_data->eval[PHASE_MG][turn] += PawnValue;
        eval_data->eval[PHASE_EG][turn] += PawnValue;

        // PST
        eval_data->eval[PHASE_MG][turn] += PST[PHASE_MG][piece][square];
        eval_data->eval[PHASE_EG][turn] += PST[PHASE_EG][piece][square];

        // Passed Pawn Bonus
        if((PassedPawnMask[turn][square] & pos->pawn[!turn])){
            eval_data->eval[PHASE_MG][turn] += PassedPawnBonus[PHASE_MG];
            eval_data->eval[PHASE_EG][turn] += PassedPawnBonus[PHASE_EG];
        }

        // Doubled Pawn Penalty
        // Applied for the pawn in the back
        if(!(betweenMask[square][promo_square] & pos->pawn[turn])){
            eval_data->eval[PHASE_MG][turn] += DoubledPawnPenalty[PHASE_MG];
            eval_data->eval[PHASE_EG][turn] += DoubledPawnPenalty[PHASE_EG];
        }

        // Isolated pawn penalty
        // When there are no pawns on either of the neighboring files
        if(     ( file == 0 && !(fileMask[file + 1] & pos->pawn[turn]) )
            ||  ( file == 7 && !(fileMask[file - 1] & pos->pawn[turn]) )
            ||  ( !(fileMask[file + 1] & pos->pawn[turn] || fileMask[file - 1] & pos->pawn[turn]) ) ){
            eval_data->eval[PHASE_MG][turn] += IsolatedPawnPenalty[PHASE_MG];
            eval_data->eval[PHASE_EG][turn] += IsolatedPawnPenalty[PHASE_EG];
        }

        // Update King saftey data
        eval_data->attack_units[turn] += count_bits(eval_data->king_area[!turn] & attacks) * ATTACK_UNIT_PAWN;

        // Update evaluation data
        eval_data->pawn_attacks[turn] |= attacks;
        eval_data->pawn_count[turn]++;
        if(is_square_light(square))
            eval_data->light_pawn_count[turn]++;
        else
            eval_data->dark_pawn_count[turn]++;

        pieces &= pieces - 1;
    }

    // Count the rammed pawns by shifting the pawn bitboard one move
    // forward relative to the pawn type and comparing with enemy pawns
    // we dont need to use masks because pawns cant be on those rows
    pieces = pos->pawn[turn];
    pieces = turn ? northOne(pieces) : southOne(pieces);
    i32 rammed_cnt = count_bits(pieces & pos->pawn[!turn]);
    eval_data->eval[PHASE_MG][turn] += rammed_cnt * RammedPawnPenalty[PHASE_MG];
    eval_data->eval[PHASE_EG][turn] += rammed_cnt * RammedPawnPenalty[PHASE_EG];

    // Bonus for connected pawns
    // Calculate from looking at the pawns that attack friendly pawns
    i32 connected_cnt = count_bits(eval_data->pawn_attacks[turn] & pos->pawn[turn]);
    eval_data->eval[PHASE_MG][turn] += connected_cnt * ConnectedPawnBonus[PHASE_MG];
    eval_data->eval[PHASE_EG][turn] += connected_cnt * ConnectedPawnBonus[PHASE_EG];

    // Penalty for hanging pawns
    i32 hanging_cnt = count_bits(~pos->attack_mask[turn] & pos->pawn[turn]);
    eval_data->eval[PHASE_MG][turn] += hanging_cnt * PawnHangingPenalty[PHASE_MG];
    eval_data->eval[PHASE_EG][turn] += hanging_cnt * PawnHangingPenalty[PHASE_EG];

    return;
}

void eval_knights(Position * pos, EvalData* eval_data, Turn turn){
    const PieceIndex piece = turn ? WHITE_KNIGHT : BLACK_KNIGHT;

    // Update evaluation attack mask
    eval_data->knight_attacks[turn] = getKnightAttacks(pos->knight[turn]);
   
    u64 pieces = pos->knight[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Knight material value
        // Changes with the number of pawns we have
        eval_data->eval[PHASE_MG][turn] += KnightValue + KnightAdjust[eval_data->pawn_count[turn]];
        eval_data->eval[PHASE_EG][turn] += KnightValue + KnightAdjust[eval_data->pawn_count[turn]];

        // PST Values
        eval_data->eval[PHASE_MG][turn] += PST[PHASE_MG][piece][square];
        eval_data->eval[PHASE_EG][turn] += PST[PHASE_EG][piece][square];

        // Calculate the knight mobility by looking at
        // where it can move thats not under attack by opponenet
        // first we filter out moves where it attacks friendly
        // and then and it with the inverse opponent attack mask
        u64 knight_moves = knightAttacks(square) & ~pos->color[turn];
        i32 mobility = count_bits(knight_moves & ~pos->attack_mask[!turn]);
        eval_data->eval[PHASE_MG][turn] += KnightMobility[PHASE_MG][mobility];
        eval_data->eval[PHASE_EG][turn] += KnightMobility[PHASE_EG][mobility];

        // Update King saftey data
        eval_data->attack_units[turn] += count_bits(eval_data->king_area[!turn] & knight_moves) * ATTACK_UNIT_KNIGHT;

        // Update the phase
        eval_data->phase_value += MINOR_PHASE;

        pieces &= pieces - 1;
    }

    // Knight outpost bonus
    // it an outpost if not attacked by enemy pawns
    // and is in an outpost square, bonus for knights protected by pawn
    u64 outpost_knights = pos->knight[turn] & ~eval_data->pawn_attacks[!turn] & KnightOutpostMask[turn];
    i32 outpost_count = count_bits(outpost_knights);
    i32 extra_outpost_count = count_bits(outpost_knights & eval_data->pawn_attacks[turn]);

    eval_data->eval[PHASE_MG][turn] += OutpostKnightBonus[PHASE_MG] * outpost_count;
    eval_data->eval[PHASE_EG][turn] += OutpostKnightBonus[PHASE_EG] * outpost_count;

    eval_data->eval[PHASE_MG][turn] += OutpostKnightExtraBonus[PHASE_MG] * extra_outpost_count;
    eval_data->eval[PHASE_EG][turn] += OutpostKnightExtraBonus[PHASE_EG] * extra_outpost_count;

    // Penalty for hanging knights
    i32 handing_cnt = count_bits(~pos->attack_mask[turn] & pos->knight[turn]);
    eval_data->eval[PHASE_MG][turn] += KnightHangingPenalty[PHASE_MG] * handing_cnt;
    eval_data->eval[PHASE_EG][turn] += KnightHangingPenalty[PHASE_EG] * handing_cnt;

    return;
}

void eval_bishops(Position * pos, EvalData* eval_data, Turn turn){
    const PieceIndex piece = turn ? WHITE_BISHOP : BLACK_BISHOP;
    i32 light_bishops = 0, dark_bishops = 0;

    // Update evaluation attack mask
    eval_data->bishop_attacks[turn] = getBishopAttacks(pos->bishop[turn], pos->color[turn], pos->color[!turn]);
    
    u64 pieces = pos->bishop[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Material Value
        eval_data->eval[PHASE_MG][turn] += BishopValue;
        eval_data->eval[PHASE_EG][turn] += BishopValue;

        // PST Values
        eval_data->eval[PHASE_MG][turn] += PST[PHASE_MG][piece][square];
        eval_data->eval[PHASE_EG][turn] += PST[PHASE_EG][piece][square];

        // Calculate the bishop mobility by looking at
        // where it can move thats not under attack by opponenet
        // first we filter out moves where it attacks friendly
        // and then and it with the inverse opponent attack mask
        u64 bishop_moves = bishopAttacks(pos->color[turn] | pos->color[!turn], square) & ~pos->color[turn];
        i32 mobility = count_bits(bishop_moves & ~pos->attack_mask[!turn]);
        eval_data->eval[PHASE_MG][turn] += BishopMobility[PHASE_MG][mobility];
        eval_data->eval[PHASE_EG][turn] += BishopMobility[PHASE_EG][mobility];
        
        // Here we do some square color evaluation,
        // if the bishop is on the same square as a majority
        // of its own pawns it gets penalized
        if(is_square_light(square)){
            light_bishops++;
            if(eval_data->light_pawn_count[turn] > eval_data->dark_pawn_count[turn] + 1){
                eval_data->eval[PHASE_MG][turn] += BishopPawnWeakPenalty[PHASE_MG];
                eval_data->eval[PHASE_EG][turn] += BishopPawnWeakPenalty[PHASE_EG];
            }
        }
        else{
            dark_bishops++;
            if(eval_data->dark_pawn_count[turn] > eval_data->light_pawn_count[turn] + 1){
                eval_data->eval[PHASE_MG][turn] += BishopPawnWeakPenalty[PHASE_MG];
                eval_data->eval[PHASE_EG][turn] += BishopPawnWeakPenalty[PHASE_EG];
            }
        }

        // Update King saftey data
        eval_data->attack_units[turn] += count_bits(eval_data->king_area[!turn] & bishop_moves) * ATTACK_UNIT_BISHOP;

        // Update the phase
        eval_data->phase_value += MINOR_PHASE;

        pieces &= pieces - 1;
    }

    // Give a bonus if there are bishops on opposite colors
    if (light_bishops >= 1 && dark_bishops >= 1){
        eval_data->eval[PHASE_MG][turn] += OppositeBishopBonus[PHASE_MG];
        eval_data->eval[PHASE_EG][turn] += OppositeBishopBonus[PHASE_EG];
    } 

    // Bishop outpost bonus
    // it an outpost if not attacked by enemy pawns
    // and is in an outpost square, bonus for knights protected by pawn
    u64 outpost_bishops = pos->bishop[turn] & ~eval_data->pawn_attacks[!turn] & BishopOutpostMask[turn];
    i32 outpost_cnt = count_bits(outpost_bishops);
    i32 extra_outpost_cnt = count_bits(outpost_bishops & eval_data->pawn_attacks[turn]);
    eval_data->eval[PHASE_MG][turn] += OutpostBishopBonus[PHASE_MG] * outpost_cnt;
    eval_data->eval[PHASE_EG][turn] += OutpostBishopBonus[PHASE_EG] * outpost_cnt;
    eval_data->eval[PHASE_MG][turn] += OutpostBishopExtraBonus[PHASE_MG] * extra_outpost_cnt;
    eval_data->eval[PHASE_EG][turn] += OutpostBishopExtraBonus[PHASE_EG] * extra_outpost_cnt;

    // Penalty for hanging bishops
    i32 hanging_cnt = count_bits(~pos->attack_mask[turn] & pos->bishop[turn]);
    eval_data->eval[PHASE_MG][turn] += BishopHangingPenalty[PHASE_MG] * hanging_cnt;
    eval_data->eval[PHASE_EG][turn] += BishopHangingPenalty[PHASE_EG] * hanging_cnt;

    return;
}

void eval_rooks(Position * pos, EvalData* eval_data, Turn turn){
    const PieceIndex piece = turn ? WHITE_ROOK : BLACK_ROOK;

    // Update evaluation attack mask
    eval_data->rook_attacks[turn] = getRookAttacks(pos->rook[turn], pos->color[turn], pos->color[!turn]);

    // PST Values
    u64 pieces = pos->rook[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Rook material value
        // Changes with the number of pawns we have
        eval_data->eval[PHASE_MG][turn] += RookValue + RookAdjust[eval_data->pawn_count[turn]];
        eval_data->eval[PHASE_EG][turn] += RookValue + RookAdjust[eval_data->pawn_count[turn]];

        // PST Values
        eval_data->eval[PHASE_MG][turn] += PST[PHASE_MG][piece][square];
        eval_data->eval[PHASE_EG][turn] += PST[PHASE_EG][piece][square];

        // Calculate the rook mobility by looking at
        // where it can move thats not under attack by opponenet
        u64 rook_moves = rookAttacks(pos->color[turn] | pos->color[!turn], square) & ~pos->color[turn];
        i32 mobility = count_bits(rook_moves & ~pos->attack_mask[!turn]);
        eval_data->eval[PHASE_MG][turn] += RookMobility[PHASE_MG][mobility];
        eval_data->eval[PHASE_EG][turn] += RookMobility[PHASE_EG][mobility];

        // Update King saftey data
        eval_data->attack_units[turn] += count_bits(eval_data->king_area[!turn] & rook_moves) * ATTACK_UNIT_ROOK;

        // Update the phase
        eval_data->phase_value += ROOK_PHASE;

        pieces &= pieces - 1;
    }

    // Connected Rook Bonus
    // Given if one of the rooks is attacking the other
    if(count_bits(eval_data->rook_attacks[turn] & pos->rook[turn]) >= 2){
        eval_data->eval[PHASE_MG][turn] += ConnectedRookBonus[PHASE_MG];
        eval_data->eval[PHASE_EG][turn] += ConnectedRookBonus[PHASE_EG];
    }

    // Double Rook Penalty
    // having two rooks is not that great or something not sure abt this one
    if (count_bits(pos->rook[turn]) >= 2){
        eval_data->eval[PHASE_MG][turn] += TwoRookPenalty[PHASE_MG];
        eval_data->eval[PHASE_EG][turn] += TwoRookPenalty[PHASE_EG];
    }

    // Penalty for hanging rooks
    i32 hanging_cnt = count_bits(~pos->attack_mask[turn] & pos->rook[turn]);
    eval_data->eval[PHASE_MG][turn] += RookHangingPenalty[PHASE_MG] * hanging_cnt;
    eval_data->eval[PHASE_EG][turn] += RookHangingPenalty[PHASE_EG] * hanging_cnt;

    return;
}

void eval_queens(Position * pos, EvalData* eval_data, Turn turn){
    const PieceIndex piece = turn ? WHITE_QUEEN : BLACK_QUEEN;
    
    u64 pieces = pos->queen[turn];
    while (pieces) {
        i32 square = getlsb(pieces);

        // Material Value
        eval_data->eval[PHASE_MG][turn] += QueenValue;
        eval_data->eval[PHASE_EG][turn] += QueenValue;

        // PST Values
        eval_data->eval[PHASE_MG][turn] += PST[PHASE_MG][piece][square];
        eval_data->eval[PHASE_EG][turn] += PST[PHASE_EG][piece][square];

        // Calculate the queen mobility by looking at
        // where it can move thats not under attack by opponenet
        u64 queen_moves  = rookAttacks(  pos->color[turn] | pos->color[!turn], square) & ~pos->color[turn];
            queen_moves |= bishopAttacks(pos->color[turn] | pos->color[!turn], square) & ~pos->color[turn];
        eval_data->eval[PHASE_MG][turn] += QueenMobility[PHASE_MG][count_bits(queen_moves & ~pos->attack_mask[!turn])];
        eval_data->eval[PHASE_EG][turn] += QueenMobility[PHASE_EG][count_bits(queen_moves & ~pos->attack_mask[!turn])];

        // Update King saftey data
        eval_data->attack_units[turn] += count_bits(eval_data->king_area[!turn] & queen_moves) * ATTACK_UNIT_QUEEN;

        // Update the phase
        eval_data->phase_value += QUEEN_PHASE;

        pieces &= pieces - 1;
    }

    // Penalty for hanging queens
    i32 hanging_cnt = count_bits(~pos->attack_mask[turn] & pos->queen[turn]);
    eval_data->eval[PHASE_MG][turn] += QueenHangingPenalty[PHASE_MG] * hanging_cnt;
    eval_data->eval[PHASE_EG][turn] += QueenHangingPenalty[PHASE_EG] * hanging_cnt;

    return;
}

void eval_kings(Position * pos, EvalData* eval_data, Turn turn){
    const PieceIndex piece = turn ? WHITE_KING : BLACK_KING;
    const u32 square = getlsb(pos->king[turn]);
    i32 file = square % 8;

    // PST Values
    eval_data->eval[PHASE_MG][turn] += PST[PHASE_MG][piece][square];
    eval_data->eval[PHASE_EG][turn] += PST[PHASE_EG][piece][square];

    // Mobility
    i32 move_count = 0;
    getKingMoves(pos, turn, &move_count);
    if (turn  && pos->flags & W_SHORT_CASTLE) move_count++;
    if (turn  && pos->flags & W_LONG_CASTLE ) move_count++;
    if (!turn && pos->flags & B_SHORT_CASTLE) move_count++;
    if (!turn && pos->flags & B_LONG_CASTLE ) move_count++;

    // Penalty for when there are no pawns on a file near the king
    for(i32 i = MAX(0, file-1); i <= MIN(7, file+1); i++){
        if(fileMask[file] & (pos->pawn[turn] | pos->pawn[!turn]) ){
            eval_data->eval[PHASE_MG][turn] += OpenFileNearKingPenalty[PHASE_MG];
            eval_data->eval[PHASE_EG][turn] += OpenFileNearKingPenalty[PHASE_EG];
        } 
    }

    // King gets a bonus or a penalty for the
    // number of friendly pawns in its area
    i32 pawns_near_cnt = count_bits(eval_data->king_area[turn] & pos->pawn[turn]);
    eval_data->eval[PHASE_MG][turn] += PawnsInKingArea[PHASE_MG][pawns_near_cnt];

    // The king loses eval if its very susceptible to sliding attacks, to do this we
    // look at how it can move as a queen
    u64 virt_moves  = rookAttacks(  pos->color[turn] | pos->color[!turn], square) & ~pos->color[turn];
        virt_moves |= bishopAttacks(pos->color[turn] | pos->color[!turn], square) & ~pos->color[turn];
    eval_data->eval[PHASE_MG][turn] += VirtualMobility[PHASE_MG][count_bits(virt_moves)];
    eval_data->eval[PHASE_EG][turn] += VirtualMobility[PHASE_EG][count_bits(virt_moves)];

    // Saftey information from enemy pieces attacking the kings area
    eval_data->eval[PHASE_MG][turn] -= SafetyTable[PHASE_MG][eval_data->attack_units[!turn]];
    eval_data->eval[PHASE_EG][turn] -= SafetyTable[PHASE_EG][eval_data->attack_units[!turn]];

    return;
}

/* 
 * Evaluates a position
 */
i32 eval_position(Position* pos){
    i32 eval = 0;
    EvalData eval_data = {0};
    Turn turn = pos->flags & TURN_MASK;

    // Check for insufficient material
    if(isInsufficient(pos)) return 0;

    // Set up the evaluation data structure
    init_eval_data(pos, &eval_data, WHITE);
    init_eval_data(pos, &eval_data, BLACK);

    // Evaluate pieces
    eval_pawns(pos, &eval_data, WHITE);
    eval_pawns(pos, &eval_data, BLACK);

    // i32 print_mg_score = eval_data.eval[PHASE_MG][WHITE_TURN] - eval_data.eval[PHASE_MG][BLACK_TURN];
    // i32 print_eg_score = eval_data.eval[PHASE_EG][WHITE_TURN] - eval_data.eval[PHASE_EG][BLACK_TURN];
    // printf("after pawns: mg_score %d  eg_score %d \n", print_mg_score, print_eg_score);

    eval_knights(pos, &eval_data, WHITE);
    eval_knights(pos, &eval_data, BLACK);

    // print_mg_score = eval_data.eval[PHASE_MG][WHITE_TURN] - eval_data.eval[PHASE_MG][BLACK_TURN];
    // print_eg_score = eval_data.eval[PHASE_EG][WHITE_TURN] - eval_data.eval[PHASE_EG][BLACK_TURN];
    // printf("after knights: mg_score %d  eg_score %d \n", print_mg_score, print_eg_score);

    eval_bishops(pos, &eval_data, WHITE);
    eval_bishops(pos, &eval_data, BLACK);

    // print_mg_score = eval_data.eval[PHASE_MG][WHITE_TURN] - eval_data.eval[PHASE_MG][BLACK_TURN];
    // print_eg_score = eval_data.eval[PHASE_EG][WHITE_TURN] - eval_data.eval[PHASE_EG][BLACK_TURN];
    // printf("after bishops: mg_score %d  eg_score %d \n", print_mg_score, print_eg_score);

    eval_rooks(pos, &eval_data, WHITE);
    eval_rooks(pos, &eval_data, BLACK);

    // print_mg_score = eval_data.eval[PHASE_MG][WHITE_TURN] - eval_data.eval[PHASE_MG][BLACK_TURN];
    // print_eg_score = eval_data.eval[PHASE_EG][WHITE_TURN] - eval_data.eval[PHASE_EG][BLACK_TURN];
    // printf("after rooks: mg_score %d  eg_score %d \n", print_mg_score, print_eg_score);

    eval_queens(pos, &eval_data, WHITE);
    eval_queens(pos, &eval_data, BLACK);

    // print_mg_score = eval_data.eval[PHASE_MG][WHITE_TURN] - eval_data.eval[PHASE_MG][BLACK_TURN];
    // print_eg_score = eval_data.eval[PHASE_EG][WHITE_TURN] - eval_data.eval[PHASE_EG][BLACK_TURN];
    // printf("after queens: mg_score %d  eg_score %d \n", print_mg_score, print_eg_score);

    eval_kings(pos, &eval_data, WHITE);
    eval_kings(pos, &eval_data, BLACK);

    /* Interpolate the evaluation based on the calculated game phase */
    i32 mg_score = eval_data.eval[PHASE_MG][WHITE_TURN] - eval_data.eval[PHASE_MG][BLACK_TURN];
    i32 eg_score = eval_data.eval[PHASE_EG][WHITE_TURN] - eval_data.eval[PHASE_EG][BLACK_TURN];

    i32 mg_weight = MIN(eval_data.phase_value, TOTAL_PHASE_VALUE);
    i32 eg_weight = TOTAL_PHASE_VALUE - mg_weight;
    eval += ((mg_score * mg_weight) + (eg_score * eg_weight)) / TOTAL_PHASE_VALUE;

    //printf("at the end: mg_score %d mg_wieght %d eg_score %d eg_weight %d\n", mg_score, mg_weight, eg_score, eg_weight);
    
    return turn ? eval : -eval;
}