#include "transposition.h"
#include "types.h"
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__linux__)
    #include <sys/mman.h>
#endif

TTEntry* table = NULL;
u64 key_mask = 0;

i32 init_tt(i32 size_mb){
    const uint64_t MB = 1ull << 20;
    if(table) tt_free();

    // Calculate the table size that is less than or equal to the requested size in MB
    u64 table_size = 1;
    while ((table_size) * sizeof(TTEntry) <= size_mb * MB / 2) table_size = table_size << 1;
    key_mask = table_size - 1;

    // On linux systems we want to specify the page for better perfomance
#if defined(__linux__) && !defined(__ANDROID__)
    if(size_mb >= 2){
        table = aligned_alloc(2 * MB, table_size * sizeof(TTEntry));
        madvise(table, table_size * sizeof(TTEntry), MADV_HUGEPAGE);
    }
    else table = (TTEntry*)calloc(table_size, sizeof(TTEntry));
#else
    table = (TTEntry*)calloc(table_size, sizeof(TTEntry));
#endif
    
    if(!table){
        printf("info string Failure to allocate Transposition table");
        return -1;
    }
    
    tt_clear();

    long long tt_size = table_size * sizeof(TTEntry);
    printf("info string TTEntry Size: %d, Transposition table size: %lld Mb\n", (int)sizeof(TTEntry), tt_size/MB);
    return 0;
}

i32 tt_free(){
    free(table);
    return 0;
}

void tt_clear(){
    memset(table, 0, (key_mask+1)*sizeof(TTEntry));
}

TTEntryData get_tt_entry(u64 hash){
    TTEntryData tt_data;
    TTEntry tt_entry;
    for(i32 i = 0; i < TT_ROTATION; i++){
        u64 key = (hash + i) & key_mask;
        tt_entry.data = atomic_load(&table[key].data);
        tt_entry.hash = atomic_load(&table[key].hash);
        if((tt_entry.data ^ tt_entry.hash) == hash){
            tt_data.data = tt_entry.data;
            return tt_data;
        }   
    }
    tt_data.data = 0;
    return tt_data;
}

void store_tt_entry(u64 hash, char depth, i32 eval, char node_type, Move move){
    TTEntryData tt_data;
    for(i32 i = 0; i < TT_ROTATION; i++){
        u64 key = (hash + i) & key_mask;
        tt_data.data = atomic_load(&table[key].data);
        if(node_type != PV_NODE && (tt_data.fields.node_type == PV_NODE)){
            continue;
        }
        if(depth < tt_data.fields.depth){
            continue;
        }
        tt_data.fields.eval = eval;
        tt_data.fields.depth = depth;
        tt_data.fields.move = move;
        tt_data.fields.node_type = node_type;
        hash ^= tt_data.data;
        atomic_store(&table[key].data, tt_data.data);
        atomic_store(&table[key].hash, hash);
        return;
    }
}

    
