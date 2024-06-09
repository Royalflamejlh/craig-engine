#include "transposition.h"
#include <stdlib.h>
#include <stdio.h>

#define TRANS_BITS 28

#define KEY_MASK (((1ULL << TRANS_BITS))-1)

#define ROTATION 3 //How many slots to rotate to try to insert / find

#ifdef DEBUG
static u64 get_suc, get_rej, store_cnt, store_rej;
void startTTDebug(void){
    get_suc = 0;
    get_rej = 0;
    store_cnt = 0;
    store_rej = 0;
}
void printTTDebug(void){
    printf("TT GET Suc: %" PRIu64 " Fail: %" PRIu64 ", TT STORE Suc: %" PRIu64 " Fail: %" PRIu64 "\n", get_suc, get_rej, store_cnt, store_rej);
}
#endif

static TTEntry* table;

i32 initTT(){
    table = (TTEntry*)calloc((KEY_MASK + 1), sizeof(TTEntry));
    if(!table) return -1;
    long long tt_size = (KEY_MASK + 1)* sizeof(TTEntry);
    printf("info string TTEntry Size: %d, Transposition table size: %lld Mb\n", (int)sizeof(TTEntry), tt_size/1000000);
    return 0;
}

i32 freeTT(){
    free(table);
    return 0;
}

TTEntry* getTTEntry(u64 hash){
    for(i32 i = 0; i < ROTATION; i++){
        u64 key = (hash + i) & KEY_MASK; // Ensure wrapping around
        if(table[key].hash == (u32)(hash >> 32)){
            #ifdef DEBUG
            get_suc++;
            #endif
            return &table[key];
        }   
    }
    #ifdef DEBUG
    get_rej++;
    #endif
    return NULL;
}

void storeTTEntry(u64 hash, char depth, i32 eval, char nodeType, Move move){
    for(i32 i = 0; i < ROTATION; i++){
        u64 key = (hash + i) & KEY_MASK; // Ensure wrapping around
        if(nodeType != PV_NODE && table[key].nodeType == PV_NODE){
            continue;
        }
        if(depth < table[key].depth){
            continue;
        }
        #ifdef DEBUG
        store_cnt++;
        #endif
        table[key].eval = eval;
        table[key].depth = depth;
        table[key].move = move;
        table[key].nodeType = nodeType;
        table[key].hash = hash >> 32;
        return;
    }
    #ifdef DEBUG
    store_rej++;
    #endif
}

    
