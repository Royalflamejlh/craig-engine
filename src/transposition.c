#include "transposition.h"
#include <stdlib.h>

#define KEY_MASK 0xFFF


#ifdef DEBUG
#include <stdio.h>
static uint64_t get_suc, get_col, store_cnt;
void startTTDebug(void){
    get_suc = 0;
    get_col = 0;
    store_cnt = 0;
}
void printTTDebug(void){
    printf("TT Collisions %llu, TT Sucesses %llu, TT Stores %llu\n", get_col, get_suc, store_cnt);
}
#endif

static TTEntry* table;

int initTT(){
    table = (TTEntry*)calloc((KEY_MASK + 1), sizeof(TTEntry));
    if(!table) return -1;
    return 0;
}

TTEntry* getTTEntry(uint64_t hash){
    if(table[hash & KEY_MASK].hash == hash){
        #ifdef DEBUG
        get_suc++;
        #endif
        return &table[hash & KEY_MASK];
    }
    #ifdef DEBUG
    get_col++;
    #endif
    return NULL;
}

void storeTTEntry(uint64_t hash, char depth, int eval, char nodeType, Move move){
    table[hash & KEY_MASK].eval = eval;
    table[hash & KEY_MASK].depth = depth;
    table[hash & KEY_MASK].move = move;
    table[hash & KEY_MASK].nodeType = nodeType;
    table[hash & KEY_MASK].hash = hash;
}