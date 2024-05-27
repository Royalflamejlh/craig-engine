#include "transposition.h"
#include <stdlib.h>

#ifdef __FAST_AS_POOP
#define KEY_MASK 0xFFFFFFF
#else

#define KEY_MASK 0xFFFFFFF

#endif

#define ROTATION 3 //How many slots to rotate to try to insert / find

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
    printf("TT GET Suc: %" PRIu64 " Fail: %" PRIu64 ", TT STORE Suc: %" PRIu64 " Fail: %" PRIu64 "\n", get_suc, get_rej, store_cnt, store_rej);
}
#endif

static TTEntry* table;

int initTT(){
    table = (TTEntry*)calloc((KEY_MASK + 1), sizeof(TTEntry));
    if(!table) return -1;
    return 0;
}

TTEntry* getTTEntry(uint64_t hash){
    for(int i = 0; i < ROTATION; i++){
        uint64_t key = (hash + i) & KEY_MASK; // Ensure wrapping around
        if(table[key].hash == hash){
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

void storeTTEntry(uint64_t hash, char depth, int eval, char nodeType, Move move){
    for(int i = 0; i < ROTATION; i++){
        uint64_t key = (hash + i) & KEY_MASK; // Ensure wrapping around
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
        table[key].hash = hash;
        return;
    }
    #ifdef DEBUG
    store_rej++;
    #endif
}

    
