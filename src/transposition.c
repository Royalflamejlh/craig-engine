#include "transposition.h"
#include <stdlib.h>

#define KEY_MASK 0xFFF

static TTEntry* table;

int initTT(){
    table = (TTEntry*)calloc((KEY_MASK + 1), sizeof(TTEntry));
    if(!table) return -1;
    return 0;
}

TTEntry getTTEntry(uint64_t hash){
    return table[hash & KEY_MASK];
}

void setTTEntry(int eval, char depth, Move move, char nodeType, uint64_t hash){
    table[hash & KEY_MASK].eval = eval;
    table[hash & KEY_MASK].depth = depth;
    table[hash & KEY_MASK].move = move;
    table[hash & KEY_MASK].nodeType = nodeType;
    table[hash & KEY_MASK].hash = hash;
}