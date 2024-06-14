#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "types.h"
#include <stdatomic.h>
#include <stdalign.h>

enum {
    NO_NODE,
    PV_NODE,
    CUT_NODE,
    ALL_NODE,
};

#pragma pack(1)
typedef struct {
    u8 depth;
    Move move;
    u8 node_type;
    i32 eval;
} TTEntryFields;
#pragma pack()

typedef struct {
    alignas(8) _Atomic u64 data;
    alignas(8) _Atomic u64 hash;
} TTEntry;

typedef union {
    u64 data;
    TTEntryFields fields;
} TTEntryData;

_Static_assert(sizeof(TTEntryData)   == 8, "Size of TTEntryData is not 64 bits");
_Static_assert(sizeof(TTEntryFields) == 8, "Size of TTEntryFields is not 64 bits");

i32 initTT();
i32 freeTT();

TTEntryData getTTEntry(u64 hash);

void storeTTEntry(u64 hash, char depth, i32 eval, char node_type, Move move);

#ifdef DEBUG
void startTTDebug(void);
void printTTDebug(void);
#endif

#endif