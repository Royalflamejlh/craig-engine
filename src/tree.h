#ifndef tree_h
#define tree_h
#include "types.h"

u32 getHistoryScore(char pos_flags, Move move);

i32 getBestMove(Position pos);
i32 pvSearch( Position* pos, i32 alpha, i32 beta, char depth, char ply, Move* pvArray, i32 pvIndex);
i32 zwSearch( Position* pos, i32 beta, char depth, char ply, Move* pvArray );
i32 quiesce( Position* pos, i32 alpha, i32 beta, char ply, char q_ply, Move* pvArray);
void selectSort(i32 i, Move *moveList, i32 *moveVals, i32 size, Move ttMove, Move *killerMoves, i32 kmv_size);
#endif