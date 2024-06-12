#ifndef tree_h
#define tree_h
#include "types.h"

u32 getHistoryScore(char pos_flags, Move move);

i32 searchTree(Position pos, u32 depth, Move *pvArray, KillerMoves* km, i32 eval_prev, SearchStats* stats);
i32 pvSearch( Position* pos, i32 alpha, i32 beta, i8 depth, u8 ply, Move* pvArray, KillerMoves* km, SearchStats* stats);

i32 zwSearch( Position* pos, i32 beta, i8 depth, u8 ply, KillerMoves* km, SearchStats* stats, u8 isNull);
i32 quiesce( Position* pos, i32 alpha, i32 beta, u8 ply, u8 q_ply, SearchStats* stats);

#endif