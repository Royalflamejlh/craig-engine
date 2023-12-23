#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "types.h"
uint16_t generateLegalMoves(Position position,  Move* moveList);
uint64_t generatePinnedPieces(Position pos);
int makeMove(Position *pos, Move move);
int makeNullMove(Position *pos);
#endif