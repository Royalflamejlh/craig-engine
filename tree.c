#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include "board.h"
#include "util.h"
#include "evaluator.h"
#include "movement.h"
#include "tree.h"

static struct Node* root;
static struct Node* cur;

static void propagateRating(struct Node* node);
static void buildTreeMovesHelper(struct Node* node, int depth);

//Initialize the move tree at the start of the game
void initializeTree(void){
    root = malloc(sizeof(struct Node));
    root->move = 0;
    root->status = STATUS_ROOT;
    root->color = 'B';
    initializeBoard(root->board);
    root->rating = INT_MAX;
    root->parent = NULL;
    root->children = NULL;
    root->childrenCount = 0;
    cur = root;
}


/*
* Get the root node of a tree
*/
struct Node* getTreeRoot(void){
    return root;
}

/*
* Add a node to the tree
*/
struct Node* addTreeNode(struct Node* parent, int64_t move, char status, int rating) {
    if (parent == NULL) {
        return NULL;
    }

    struct Node* newNode = malloc(sizeof(struct Node));
    if (newNode == NULL) {
        return NULL;
    }
    
    newNode->move = move;
    newNode->status = status;
    newNode->parent = parent;
    newNode->children = NULL;
    newNode->childrenCount = 0;

    memcpy(&newNode->board[0][0], &parent->board[0][0], 8*8*sizeof(char));
    receiveMove(newNode->board, move);

    newNode->rating = getRating(newNode->board);

    newNode->color = 'W';
    if (parent->color == 'W') {
        newNode->color = 'B';
    }

    if (parent->children == NULL) {
        parent->children = malloc(sizeof(struct Node*) * (parent->childrenCount + 1));
    } else {
        parent->children = realloc(parent->children, sizeof(struct Node*) * (parent->childrenCount + 1));
    }


    //Error check the malloc
    if (parent->children == NULL) {
        printf("info string Error malloc freeing newnode %p\n\r", newNode);
        fflush(stdout);
        free(newNode);
        return NULL;
    }

    parent->children[parent->childrenCount] = newNode;
    parent->childrenCount++;

    if(status == STATUS_CURRENT){
        pruneAbove(newNode);
        cur = newNode;
    }
    propagateRating(newNode);

    return newNode;
}

/*
* Update the status of a node
*/
void updateNodeStatus(struct Node* node, char status) {
    if (node == NULL) {
        return;
    }
    if(status == STATUS_CURRENT){
        pruneAbove(node);
        cur = node;
    }
    node->status = status;
}

/*
* Iterate through a tree by putting in a move, 
*/
struct Node* iterateTree(struct Node* it, int64_t move) {
    if (it == NULL) {
        return NULL;
    }

    for (int i = 0; i < it->childrenCount; i++) {
        if (it->children[i]->move == move) {
            return it->children[i];
        }
    }

    return NULL;
}


 
/*
* Frees a tree from a node
*/
static void freeTree(struct Node* node) {
    if (node == NULL) return;

    for (int i = 0; i < node->childrenCount; i++) {
        freeTree(node->children[i]);
    }

    free(node->children);
    free(node);
}


void pruneNodeExceptFor(struct Node* node, struct Node* exceptNode) {
    if (node == NULL) return;

    for (int i = 0; i < node->childrenCount; ++i) {
        if (node->children[i] != exceptNode) {
            freeTree(node->children[i]);
        }
    }

    if (exceptNode != NULL) {
        node->children[0] = exceptNode;
        node->childrenCount = 1;

        struct Node** temp = realloc(node->children, sizeof(struct Node*));
        if (temp != NULL) {
            node->children = temp;
        } else {
            node->children = NULL;
            node->childrenCount = 0;
        }
    } else {
        free(node->children);
        node->children = NULL;
        node->childrenCount = 0;
    }
}

void pruneAbove(struct Node* current) {
    struct Node* child = current;
    struct Node* parent = current->parent;

    while (parent != NULL) {
        pruneNodeExceptFor(parent, child);

        child = parent;
        parent = parent->parent;
    }
}

struct Node* getBestChild(struct Node* node){
    if (node == NULL || node->childrenCount == 0) {
        return NULL;
    }

    struct Node *best = node->children[0];
    int bestRating = node->children[0]->rating;

    for (int i = 1; i < node->childrenCount; ++i) {
        int childRating = node->children[i]->rating;

        //Note if node->color='B' you are looking for the best move for 'W'
        if (((node->color == 'W') && childRating < bestRating) ||
            ((node->color == 'B') && childRating > bestRating)) {
            best = node->children[i];
            bestRating = childRating;
        }
    }

    return best;
}


struct Node* getBestCurChild(){
    return getBestChild(cur);
}


void buildTreeMoves(int depth){
    if(cur == NULL){
        return;
    }
    buildTreeMovesHelper(cur, depth);
}

static void buildTreeMovesHelper(struct Node* node, int depth){
    if(depth <= 0){
        return;
    }
    if(node->childrenCount == 0){
        buildLegalMoves(node);
    }
    for (int i = 0; i < node->childrenCount; i++) {
        buildTreeMovesHelper(node->children[i], depth - 1);
    }
}

static void propagateRating(struct Node* node) {
    if (node == NULL || node->parent == NULL) {
        return;
    }

    struct Node* parent = node->parent;

    if (((node->color == 'W') && parent->rating < node->rating) ||
        ((node->color == 'B') && parent->rating > node->rating)) {
        parent->rating = node->rating;
        propagateRating(parent);
    }
}



#ifdef DEBUG
void printNode(struct Node* node, int level, int depth) {
    if (node == NULL) {
        return;
    }
    if(depth == 0){
        return;
    }
    for (int i = 0; i < level; ++i) {
        printf("    ");
    }
    char moveStr[10]; 
    moveIntToChar(node->move, moveStr);
    printf("%p: stat:%d col:%c rat:%d par:%p chil[%d]@%p mov:%s\r\n",
    node, node->status, node->color, node->rating, node->parent,
    node->childrenCount, node->children, moveStr);
    printBoard(node->board);
    for (int i = 0; i < node->childrenCount; ++i) {
        printNode(node->children[i], level + 1, depth - 1);
    }
}

void printTree(void) {
    printf("Tree Structure:\n");
    printNode(root, 0, 100);
}

void printCurNode(void){
    printNode(cur, 0, 1);
}

#endif
