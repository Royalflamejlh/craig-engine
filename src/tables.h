#ifndef TABLES_H
#define TABLES_H
#include "types.h"

void storeKillerMove(KillerMoves* km, i32 ply, Move move);
u8 isKillerMove(KillerMoves* km, Move move, int ply);
void clearKillerMoves(KillerMoves* km);

void storeHistoryMove(char pos_flags, Move move, char depth);
u32 getHistoryScore(char pos_flags, Move move);

#endif // TABLES_H
