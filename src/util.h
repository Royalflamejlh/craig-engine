#ifndef UTIL_H
#define UTIL_H
#include <stdlib.h>


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#include "types.h"
void printMove(Move move);
uint64_t perft(int depth, Position pos);
int checkMoveCount(Position pos);
int python_init();
int python_close();

Move moveStrToType(Position pos, char* str);
Stage calculateStage(Position pos);

void printPV(Move *pvArray, int depth);
static inline int count_bits(uint64_t v){
    unsigned int c;
    for (c = 0; v; c++){
        v &= v - 1;
    }
    return c;
}

static inline uint64_t random_uint64() {
  uint64_t u1, u2, u3, u4;
  u1 = (uint64_t)(rand()) & 0xFFFF; u2 = (uint64_t)(rand()) & 0xFFFF;
  u3 = (uint64_t)(rand()) & 0xFFFF; u4 = (uint64_t)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

static inline void movcpy (Move* pTarget, const Move* pSource, int n) {
   while (n-- && (*pTarget++ = *pSource++));
}

/*
*
* Hash Stack Stuff
*
*/

HashStack createHashStack(void);
int removeHashStack(HashStack *hashStack);
void doubleHashStack(HashStack *hs);
#endif