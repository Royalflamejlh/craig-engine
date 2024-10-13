#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "types.h"
u16 generateLegalMoves(Position* pos,  Move* moveList);
u16 generateThreatMoves(Position* pos,  Move* moveList);
u64 generatePinnedPieces(Position* pos);
void make_move(ThreadData *td, Move move);
void _make_move(Position *pos,  Move move);
void unmake_move(ThreadData *td, Move move);
void make_null_move(ThreadData *td);
void unmake_null_move(ThreadData *td);
#endif
