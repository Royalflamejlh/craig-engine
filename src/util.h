#ifndef UTIL_H
#define UTIL_H

#include "types.h"
void printMove(Move move);
uint64_t perft(int depth, Position pos);
int checkMoveCount(Position pos);
int python_init();
int python_close();
#endif