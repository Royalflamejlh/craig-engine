#include "transposition.h"
#include <stdlib.h>

#define KEY_MASK 0xFF


#ifdef DEBUG
#include <stdio.h>
static uint64_t get_suc, get_rej, store_cnt, store_rej;
void startTTDebug(void){
    get_suc = 0;
    get_rej = 0;
    store_cnt = 0;
    store_rej = 0;
}
void printTTDebug(void){
    printf("TT GET Suc: %llu Fail: %llu, TT STORE Suc: %llu Fail: %llu\n", get_suc, get_rej, store_cnt, store_rej);
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
    get_rej++;
    #endif
    return NULL;
}

void storeTTEntry(uint64_t hash, char depth, int eval, char nodeType, Move move){
    if(nodeType != PV_NODE && table[hash & KEY_MASK].nodeType == PV_NODE){
        #ifdef DEBUG
        store_rej++;
        #endif
        return;
    }
    if(depth < table[hash & KEY_MASK].depth){
        #ifdef DEBUG
        store_rej++;
        #endif
        return;
    }
    #ifdef DEBUG
    store_cnt++;
    #endif
    table[hash & KEY_MASK].eval = eval;
    table[hash & KEY_MASK].depth = depth;
    table[hash & KEY_MASK].move = move;
    table[hash & KEY_MASK].nodeType = nodeType;
    table[hash & KEY_MASK].hash = hash;
}