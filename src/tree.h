#ifndef TREE_H
#define TREE_H

#include <stdint.h>
#include "util.h"

#define DEBUG 1
#define STATUS_PLAYED 0
#define STATUS_CURRENT 2
#define STATUS_PREDICTED 4
#define STATUS_ROOT 8

#define DEEP_SEARCH_WIDTH 2

void initializeTree(void);
void initializeTreeFEN(char* FEN);
size_t getTreeRoot(void);
size_t addTreeNode(size_t parent, struct Move move, char status);
void updateNodeStatus(size_t node, char status);
size_t iterateTree(size_t it, struct Move move);
int pruneNode(size_t it, size_t nextIt);
void pruneNodeExceptFor(size_t node, size_t exceptNode);
void pruneAbove(size_t current);
size_t getBestCurChild(void);
void buildTreeMoves(int depth);

void deepSearchTree(int starting_depth, int depth);

#ifdef DEBUG
void printNode(size_t, int level, int depth);
void printTree(void);
void printCurNode(void);
#endif

#endif // TREE_H

