#pragma once
#include "types.h"

i32 eval_move(Move move, Position* pos);
i32 see(Position* pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece);
void eval_movelist(Position* pos, Move* moveList, i32* moveVals, i32 size);