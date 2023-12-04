#ifndef mempool_h
#define mempool_h

#include <stdio.h>
#include "types.h"

#define INITIAL_POOL_SIZE 100
#define NULL_NODE (size_t)(-1)

struct DynamicNodePool {
    Node* nodes;
    int* used;
    size_t size;
    size_t usedCount;
    size_t nextAvailable;
};
size_t allocateNode(void);
void freeNode(size_t index);
void initializeNodePool(void);
Node* getNode(size_t index);


#endif /* mempool_h */
