#pragma once
#include "types.h"

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