#pragma once
#include "types.h"

#define TRACE 0

i32 eval(Position pos);
i32 move_eval(Move move, Position* pos);
i32 quick_eval(Position pos);
i32 see(Position pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece);
void movelist_eval(Move* moveList, i32* moveVals, i32 size, Position pos);
void init_pst();


