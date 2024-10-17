#pragma once
#include "types.h"
#include "evaluator.h"

/**
 * Tree Parameters
 */
static const u32 PV_PRUNE_MOVE_IDX      = 5;    // Move to start pruning on in PV
static const u32 PRUNE_MOVE_IDX         = 2;    // Move to start pruning on otherwise
static const u32 MAX_QUIESCE_PLY        = 5;    // How far quiescence search can go
static const u32 LMR_DEPTH              = 3;    // LMR not performed if depth < LMR_DEPTH
static const i32 ASP_EDGE               = 250;  // Buffer size of aspiration window
static const i32 HELPER_ASP_EDGE        = 500;  // Buffer size of aspiration window in helper search
static const i32 PV_FUTIL_MARGIN        = 300;  // Score difference for a node to be futility pruned
static const i32 ZW_FUTIL_MARGIN        = 200;  // Futility pruning margin for zero-window nodes
static const i8  NULL_PRUNE_R           = 3;    // How much null pruning reduces depth
static const i32 NMR_MARGIN             = 2000; // Threshold for applying null move pruning
static const u32 HELPER_MOVE_DISORDER   = 3;    // Degree of move ordering disruption in helper searches
static const u32 HELPER_THREAD_DISORDER = 3;    // How differently each helper thread searches from one another
static const i32 DeltaValue             = 750;  // Difference for delta pruning in Q search
static const i32 EarlyDeltaValue        = 9000; // Difference for delta pruning in Q search before move is made
static const i32 PromotionBuffer        = 9000; // What to add to delta pruning in case move is a promotion move
static const i32 OPN_GAME_MOVES         = 8;    // Moves that count as early game (fullmoves)
static const u32 END_GAME_PIECES        = 16;   // Pieces left to count as late game

/**
 * Search Parameters
 */
static const real64 SEARCH_REDUCTION_LEVEL = 0.75; // How much time is reduced when search finds move to reduce time on
static const real64 SEARCH_EXTENSION_LEVEL = 1.5;  // How much time is expanded when search finds move to extend time on

/**
 * Move Ordering Parameters
 */
static const i32 MovePawnValue   =   1000;
static const i32 MoveKnightValue =   3500;
static const i32 MoveBishopValue =   3600;
static const i32 MoveRookValue   =   5000;
static const i32 MoveQueenValue  =  10000;
static const i32 MoveKingValue   = 100000;
static const i32 MoveCastleBonus =     30;

/**
 * Evaluation Parameters
 */
static const i32 PawnValue   =   1000;
static const i32 KnightValue =   3500;
static const i32 BishopValue =   3600;
static const i32 RookValue   =   5000;
static const i32 QueenValue  =  10000;
static const i32 KingValue   = 100000;

/* Piece-Square Tables */
static const i32 PSTPawn[2][64] = {
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
static const i32 PSTKnight[2][64] = {
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
static const i32 PSTBishop[2][64] = {
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
static const i32 PSTRook[2][64] = {
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
static const i32 PSTQueen[2][64] = {
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
static const i32 PSTKing[2][64] = {
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
static const i32 KnightAdjust[9] = { -200, -160, -120, -80, -40,  0,  40,  80, 120 };
static const i32   RookAdjust[9] = {  150,  120,   90,  60,  30,  0, -30, -60, -90 };
static const i32 KingPawnDistancePenalty[2] = { -20, -200 };
static const i32 OpenFileNearKingPenalty[2] = { -200, -100 };
static const i32 VirtualMobility[2][28] = {
    {
        0,    0,    0,    0,    0,    0,    0,  -50, -100, -125, -150, -175,  -200,  -250,
        -300, -350, -400, -500, -500, -500, -500, -500, -500, -500, -500,  -500, -500, -500,
    },
    {
          0,    0,    0,    0,    0,    0,    0,  -5, -15, -20, -30, -40, -50, -60,
        -70,  -70,  -70,  -70,  -70,  -70,  -70, -70, -70, -70, -70, -70, -70, -70,
    },
};

static const i32 PawnsInKingArea[2][9] = {
    {  -50, -10, 25,  20,  30,  30,  30,  30,  30 },
    { -100, -50,  0,   0,   0,   0,   0,   0,   0 },
};
static const i32 ATTACK_UNIT_PAWN   = 2;
static const i32 ATTACK_UNIT_KNIGHT = 2;
static const i32 ATTACK_UNIT_BISHOP = 2;
static const i32 ATTACK_UNIT_ROOK   = 5;
static const i32 ATTACK_UNIT_QUEEN  = 6;
static const i32 SafetyTable[2][100] = {
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
static const i32 PawnHangingPenalty[2] = { -10, 0 };
static const i32 DoubledPawnPenalty[2] = { -20, -200 };
static const i32 IsolatedPawnPenalty[2] = { -50, -100 };
static const i32 RammedPawnPenalty[2] = { -20, -100 };
static const i32 PassedPawnBonus[2] = { 25, 500 };
static const i32 ConnectedPawnBonus[2] = { 20, 60 };
static const i32 KnightHangingPenalty[2] =  { -20, 0 };
static const i32 KnightMobility[2][9] = 
{
    {  -104,  -45,  -22,  -8,  6,  11,  19,  30,  43 },
    { -1040, -450, -220, -80, 60, 110, 190, 300, 430 }
};

static const i32 OutpostKnightBonus[2] = { 50, 0 };
static const i32 OutpostKnightExtraBonus[2] = { 100, 0 };

static const i32 BishopHangingPenalty[2] = { -10, 0 };
static const i32 BishopMobility[2][14] = 
{
    {  -99,  -46,  -16,  -4,  6,  14,  17,  19,  19 , 27 , 26,  52,  55,  83 },
    { -990, -460, -160, -40, 60, 140, 170, 190, 190, 270, 260, 520, 550, 830 },
};
static const i32 OutpostBishopBonus[2] = { 25, 0 };
static const i32 OutpostBishopExtraBonus[2] = { 30, 0 };
static const i32 OppositeBishopBonus[2] = { 75, 300 };
static const i32 BishopPawnWeakPenalty[2] = { -75, 0 };

static const i32 RookHangingPenalty[2] = { -15, 0 };
static const i32 RookMobility[2][15] = 
{
    {  -127,  -56,  -25,  -12,  -10,  -12,  -11,  -4,  4,  9,  11,  19,  19,  37,  97 },
    { -1270, -560, -250, -120, -100, -120, -110, -40, 40, 90, 110, 190, 190, 370, 970 },
};
static const i32 ConnectedRookBonus[2] = { 15, 0 };
static const i32 TwoRookPenalty[2] = { -5, -35 };

static const i32 QueenHangingPenalty[2] = { -60, 0 };
static const i32 QueenMobility[2][28] = 
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
static const i32 QueenPinPenalty[2] = { -100, -300};
static const i32 CastleAbilityBonus[2] = { 0, 0 };