#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H
#include "types.h"
#include <stdatomic.h>
#include <stdalign.h>
#include <stdint.h>

enum {
    NO_NODE  = 0,
    PV_NODE  = 1,
    CUT_NODE = 2,
    ALL_NODE = 3,

    TT_ROTATION = 3,
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

i32 init_tt(i32 size_mb);
i32 tt_free();
void tt_clear();
void store_tt_entry(u64 hash, char depth, i32 eval, char node_type, Move move);

TTEntryData get_tt_entry(u64 hash);
#endif