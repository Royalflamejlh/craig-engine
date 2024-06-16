#pragma once
#include "types.h"

#define TRACE 0

struct EvalData{
    u32 pawn_count[2];
};

// Evaluation functions for a single position
i32 eval_position(Position* pos);
i32 eval_pawns(Position * pos, EvalData* eval_data, Turn turn);
i32 eval_knights(Position * pos, EvalData* eval_data, Turn turn);
i32 eval_bishops(Position * pos, EvalData* eval_data, Turn turn);
i32 eval_rooks(Position * pos, EvalData* eval_data, Turn turn);
i32 eval_queens(Position * pos, EvalData* eval_data, Turn turn);
i32 eval_kings(Position * pos, EvalData* eval_data, Turn turn);

// Quickly evaluate a position based on the material
i32 eval_material(Position* pos);

void init_pst();

// Global data
extern i32 PST[3][12][64];


