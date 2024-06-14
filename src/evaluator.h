#pragma once
#include "types.h"

#define TRACE 0

i32 eval_material(Position* pos);
i32 eval_position(Position* pos);
i32 see(Position* pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece);
i32 eval_move(Move move, Position* pos);
void eval_movelist(Position* pos, Move* moveList, i32* moveVals, i32 size);
void init_pst();


