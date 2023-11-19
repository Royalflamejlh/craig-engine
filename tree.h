#ifndef TREE_H
#define TREE_H

#include <stdint.h>

#define DEBUG 1
#define STATUS_PLAYED 0
#define STATUS_CURRENT 2
#define STATUS_PREDICTED 4
#define STATUS_ROOT 8

struct Node {
    int64_t move; // the move played
    char status; // 0 means already played, 2 means the last played move, 4 means a predicted, 8 means root
    char color;
    char board[8][8];
    int rating; // the rating of the move
    struct Node *parent; // parent node
    struct Node **children; // children
    int childrenCount; // children count
};

void initializeTree(void);
struct Node* getTreeRoot(void);
struct Node* addTreeNode(struct Node* parent, int64_t move, char status, int rating);
void updateNodeStatus(struct Node* node, char status);
struct Node* iterateTree(struct Node* cur, int64_t move);
int pruneNode(struct Node* it, struct Node* nextIt);
void pruneNodeExceptFor(struct Node* node, struct Node* exceptNode);
void pruneAbove(struct Node* current);
int64_t getBestCurChild(void);

#ifdef DEBUG
void printNode(struct Node* node, int level);
void printTree(void);
void moveIntToChar(int64_t move, char* result);
#endif

#endif // TREE_H

