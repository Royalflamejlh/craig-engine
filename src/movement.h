#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "types.h"
u16 generateLegalMoves(Position position,  Move* moveList);
u16 generateThreatMoves(Position position,  Move* moveList);
u64 generatePinnedPieces(Position pos);
i32 makeMove(Position *pos, Move move);
i32 makeNullMove(Position *pos);
#endif
