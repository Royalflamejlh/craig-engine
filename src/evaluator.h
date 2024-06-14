#pragma once
#include "types.h"

i32 evaluate(Position pos);
i32 evalMove(Move move, Position* pos);
void q_evalMoves(Move* moveList, i32* moveVals, i32 size, Position pos);
i32 quickEval(Position pos);
i32 see ( Position pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece);

