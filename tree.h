#ifndef TREE_H
#define TREE_H

#include <stdint.h>
#include "util.h"

#define DEBUG 1
#define STATUS_PLAYED 0
#define STATUS_CURRENT 2
#define STATUS_PREDICTED 4
#define STATUS_ROOT 8

#define DEEP_SEARCH_WIDTH 7

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))


struct Node {
    struct Move move; // the move played
    //TODO: Combine status and castle, as they can take up a full 1 byte.
    unsigned char status; // 0 means already played, 2 means the last played move, 4 means a predicted, 8 means root
    unsigned char castle; // bit encoded 1st: white long 2nd: white short, 3rd: black long, 4th: black short
    char color; //Who played the Move
    char board[8][8]; //The board state after the move
    int rating; //The rating of the move
    struct Node *parent; // Parent node
    struct Node **children; // Children nodes
    int childrenCount; //Children count
};

void initializeTree(void);
void initializeTreeFEN(char* FEN);
struct Node* getTreeRoot(void);
struct Node* addTreeNode(struct Node* parent, struct Move move, char status);
void updateNodeStatus(struct Node* node, char status);
struct Node* iterateTree(struct Node* cur, struct Move move);
int pruneNode(struct Node* it, struct Node* nextIt);
void pruneNodeExceptFor(struct Node* node, struct Node* exceptNode);
void pruneAbove(struct Node* current);
struct Node* getBestCurChild();
void buildTreeMoves(int depth);

void deepSearchTree(int starting_depth, int depth);

#ifdef DEBUG
void printNode(struct Node* node, int level, int depth);
void printTree(void);
void printCurNode(void);
#endif

#endif // TREE_H

