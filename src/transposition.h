#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "types.h"

enum {
    NO_NODE,
    PV_NODE,
    CUT_NODE,
    ALL_NODE,
};

#pragma pack(1)
struct TTEntry{
    u8 depth;
    Move move;
    u8 nodeType;
    u32 hash;
    i32 eval;
};
#pragma pack()

typedef struct TTEntry TTEntry;

i32 initTT();
i32 freeTT();

TTEntry* getTTEntry(u64 hash);

void storeTTEntry(u64 hash, char depth, i32 eval, char nodeType, Move move);


#ifdef DEBUG
void startTTDebug(void);
void printTTDebug(void);
#endif

#endif