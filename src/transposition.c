#include "transposition.h"
#include <stdatomic.h>
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

TTEntryData getTTEntry(u64 hash){
    TTEntryData tt_data;
    TTEntry tt_entry;
    for(i32 i = 0; i < ROTATION; i++){
        u64 key = (hash + i) & KEY_MASK; // Ensure wrapping around
        tt_entry.data = atomic_load(&table[key].data);
        tt_entry.hash = atomic_load(&table[key].hash);
        if((tt_entry.data ^ tt_entry.hash) == hash){
            #ifdef DEBUG
            get_suc++;
            #endif
            tt_data.data = tt_entry.data;
            return tt_data;
        }   
    }
    #ifdef DEBUG
    get_rej++;
    #endif
    tt_data.data = 0;
    return tt_data;
}

void storeTTEntry(u64 hash, char depth, i32 eval, char node_type, Move move){
    TTEntryData tt_data;
    for(i32 i = 0; i < ROTATION; i++){
        u64 key = (hash + i) & KEY_MASK; // Ensure wrapping around
        tt_data.data = atomic_load(&table[key].data);
        if(node_type != PV_NODE && (tt_data.fields.node_type == PV_NODE)){
            continue;
        }
        if(depth < tt_data.fields.depth){
            continue;
        }
        #ifdef DEBUG
        store_cnt++;
        #endif
        tt_data.fields.eval = eval;
        tt_data.fields.depth = depth;
        tt_data.fields.move = move;
        tt_data.fields.node_type = node_type;
        hash ^= tt_data.data;
        atomic_store(&table[key].data, tt_data.data);
        atomic_store(&table[key].hash, hash);
        return;
    }
    #ifdef DEBUG
    store_rej++;
    #endif
}

    
