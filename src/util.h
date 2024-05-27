#ifndef UTIL_H
#define UTIL_H
#include <stdlib.h>


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#include "types.h"
void printMove(Move move);
void printMoveSpaced(Move move);
u64 perft(i32 depth, Position pos);
i32 checkMoveCount(Position pos);
i32 python_init();
i32 python_close();

Move moveStrToType(Position pos, char* str);
Stage calculateStage(Position pos);

void printPV(Move *pvArray, i32 depth);
static inline i32 count_bits(u64 v){
    u32 c;
    for (c = 0; v; c++){
        v &= v - 1;
    }
    return c;
}

static inline u64 random_uint64() {
  u64 u1, u2, u3, u4;
  u1 = (u64)(rand()) & 0xFFFF; u2 = (u64)(rand()) & 0xFFFF;
  u3 = (u64)(rand()) & 0xFFFF; u4 = (u64)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

static inline void movcpy (Move* pTarget, const Move* pSource, i32 n) {
   while (n-- && (*pTarget++ = *pSource++));
}

/*
*
* Hash Stack Stuff
*
*/

HashStack createHashStack(void);
i32 removeHashStack(HashStack *hashStack);
void doubleHashStack(HashStack *hs);
#endif