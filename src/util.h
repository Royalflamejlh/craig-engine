#ifndef UTIL_H
#define UTIL_H

#include "types.h"
void printMove(Move move);
uint64_t perft(int depth, Position pos);
int checkMoveCount(Position pos);
int python_init();
int python_close();

static inline int count_bits(uint64_t v){
    unsigned int c;
    for (c = 0; v; c++){
        v &= v - 1;
    }
    return c;
}
#endif