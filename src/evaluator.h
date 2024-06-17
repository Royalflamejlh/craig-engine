#pragma once
#include "types.h"

#define TRACE 0

struct EvalData{
    i32 eval[2][2];

    i32 phase_value;

    u32 pawn_count[2];
    u32 light_pawn_count[2];
    u32 dark_pawn_count[2];

    u64 pawn_attacks[2];
    u64 knight_attacks[2];
    u64 bishop_attacks[2];
    u64 rook_attacks[2];
    u64 queen_attacks[2];

    u64 king_area[2];
    
    u64 attack_units[2];
};

// Evaluation functions for a single position
i32 eval_position(Position* pos);

// Quickly evaluate a position based on the material
i32 eval_material(Position* pos);

void init_pst();

// Global data
extern i32 PST[2][12][64];


