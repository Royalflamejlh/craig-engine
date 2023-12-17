#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "types.h"

typedef struct {
    int eval;
    char depth;
    Move move;
    char nodeType;
    uint64_t hash;
} TTEntry;

int initTT();

TTEntry getTTEntry(uint64_t hash);

void setTTEntry(int eval, char depth, Move move, char nodeType, uint64_t hash);
#endif