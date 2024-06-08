#ifndef tree_h
#define tree_h
#include "types.h"

u32 getHistoryScore(char pos_flags, Move move);

i32 searchTree(Position pos, u32 depth, Move *pvArray, i32 eval_prev, SearchStats* stats);

i32 pvSearch( Position* pos, i32 alpha, i32 beta, char depth, char ply, Move* pvArray, i32 pvIndex, SearchStats* stats);
i32 zwSearch( Position* pos, i32 beta, char depth, char ply, Move* pvArray, SearchStats* stats);
i32 quiesce( Position* pos, i32 alpha, i32 beta, char ply, char q_ply, Move* pvArray, SearchStats* stats);

void selectSort(i32 i, Move *moveList, i32 *moveVals, i32 size, Move ttMove, Move *killerMoves);
#endif