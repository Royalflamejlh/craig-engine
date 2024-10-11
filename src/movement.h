#ifndef MOVEMENT_H
#define MOVEMENT_H

#include "types.h"
u16 generateLegalMoves(Position* pos,  Move* moveList);
u16 generateThreatMoves(Position* pos,  Move* moveList);
u64 generatePinnedPieces(Position* pos);
i32 make_move(Position *pos, UndoStack *undo_stack, Move move);
i32 unmake_move(Position *pos, UndoStack *undo_stack, Move move);
i32 makeNullMove(Position *pos);
#endif
