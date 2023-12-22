#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "types.h"

enum {
    NO_NODE,
    PV_NODE,
    CUT_NODE,
    ALL_NODE,
    Q_NODE
};

typedef struct {
    int eval;
    char depth;
    Move move;
    char nodeType;
    uint64_t hash;
} TTEntry;

int initTT();

TTEntry* getTTEntry(uint64_t hash);

void storeTTEntry(uint64_t hash, char depth, int eval, char nodeType, Move move);

#ifdef __COMPILE_DEBUG
#define TT_DEBUG
#endif

#ifdef TT_DEBUG
void startTTDebug(void);
void printTTDebug(void);
#endif

#endif