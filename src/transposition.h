#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "types.h"

enum {
    NO_NODE,
    PV_NODE,
    CUT_NODE,
    ALL_NODE,
    Q_ALL_NODE,
    Q_EXACT_NODE
};

typedef struct {
    i32 eval;
    char depth;
    Move move;
    char nodeType;
    u64 hash;
} TTEntry;

i32 initTT();

TTEntry* getTTEntry(u64 hash);

void storeTTEntry(u64 hash, char depth, i32 eval, char nodeType, Move move);


#ifdef DEBUG
void startTTDebug(void);
void printTTDebug(void);
#endif

#endif