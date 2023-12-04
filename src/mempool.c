#include "mempool.h"
#include <stdlib.h>

static struct DynamicNodePool nodePool = {NULL, NULL, 0, 0};

void initializeNodePool(void) {
    nodePool.size = INITIAL_POOL_SIZE;
    nodePool.usedCount = 0;
    nodePool.nextAvailable = 0;
    nodePool.nodes = (Node*)malloc(nodePool.size * sizeof(Node));
    nodePool.used = (int*)malloc(nodePool.size * sizeof(int));
    
    for (size_t i = 0; i < nodePool.size; i++) {
        nodePool.used[i] = 0;
    }
    
}

static void resizeNodePool(void) {
    size_t newSize = nodePool.size * 2;
    nodePool.nodes = (Node*)realloc(nodePool.nodes, newSize * sizeof(Node));
    nodePool.used = (int*)realloc(nodePool.used, newSize * sizeof(int));

    for (size_t i = nodePool.size; i < newSize; i++) {
        nodePool.used[i] = 0;
    }

    nodePool.size = newSize;
    nodePool.nextAvailable = nodePool.size;
}

size_t allocateNode(void) {
    if (nodePool.usedCount == nodePool.size) {
        resizeNodePool();
    }

    for (size_t i = nodePool.nextAvailable; i < nodePool.size; i++) {
        if (!nodePool.used[i]) {
            nodePool.used[i] = 1;
            nodePool.usedCount++;
            nodePool.nextAvailable = i + 1;
            return i;
        }
    }
    
    return NULL_NODE;
}

void freeNode(size_t index) {
    if (index < nodePool.size) {
        if (nodePool.used[index]) {
            nodePool.used[index] = 0;
            nodePool.usedCount--;
            if (index < nodePool.nextAvailable) {
                nodePool.nextAvailable = index;
            }
        }
    }
}

Node* getNode(size_t index) {
    if (index < nodePool.size && nodePool.used[index]) {
        return &nodePool.nodes[index];
    }
    return NULL; // Invalid index or node not in use
}
