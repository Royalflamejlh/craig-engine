#ifndef TABLES_H
#define TABLES_H
#include "types.h"

#define KMV_CNT 3

void storeKillerMove(i32 ply, Move move);
u8 isKillerMove(Move move, int ply);
void clearKillerMoves(void);
void storeHistoryMove(char pos_flags, Move move, char depth);
u32 getHistoryScore(char pos_flags, Move move);

#endif // TABLES_H