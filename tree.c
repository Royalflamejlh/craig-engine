#include "tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "board.h"

static struct Node* root;
static struct Node* cur;

//Initialize the move tree at the start of the game
void inititializeTree(void){
    root = malloc(sizeof(struct Node));
    root->move = 0;
    root->status = STATUS_ROOT;
    root->color = 'B';
    initializeBoard(root->board);
    root->rating = 0;
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
    newNode->rating = 0;
    newNode->parent = parent;
    newNode->children = NULL;
    newNode->childrenCount = 0;

    
    if (parent->color == 'W') {
        newNode->color = 'B';
    } else {
        newNode->color = 'W';
    }

    parent->children = realloc(parent->children, sizeof(struct Node*) * (parent->childrenCount + 1));
    if (parent->children == NULL) {
        free(newNode);
        return NULL;
    }
    parent->children[parent->childrenCount] = newNode;
    parent->childrenCount++;

    if(status == STATUS_CURRENT){
        pruneAbove(newNode);
        cur = newNode;
    }

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
struct Node* iterateTree(struct Node* cur, int64_t move) {
    if (cur == NULL) {
        return NULL;
    }

    for (int i = 0; i < cur->childrenCount; i++) {
        if (cur->children[i]->move == move) {
            return cur->children[i];
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
    
    freeBoard(node->board);

    free(node->children);
    free(node);
}


void pruneNodeExceptFor(struct Node* node, struct Node* exceptNode) {
    if (node == NULL || node->childrenCount == 0) {
        return;
    }

    for (int i = 0; i < node->childrenCount; ++i) {
        if (node->children[i] != exceptNode) {
            freeTree(node->children[i]);
        }
    }

    if (exceptNode != NULL) {
        node->children[0] = exceptNode;
        node->childrenCount = 1;
    } else {
        free(node->children);
        node->children = NULL;
        node->childrenCount = 0;
    }

    if (exceptNode != NULL) {
        node->children = realloc(node->children, sizeof(struct Node*));
        if (node->children == NULL) {
            //TODO: exception?
        }
    }
}

void pruneAbove(struct Node* current) {
    struct Node* child = current;
    struct Node* parent = current->parent;

    while (parent != NULL && parent->status != STATUS_ROOT) {
        pruneNodeExceptFor(parent, child);

        child = parent;
        parent = parent->parent;
    }
}

struct Node* getBestChild(struct Node* node){
    if(node == NULL){
        return NULL;
    }
    if(node->childrenCount == 0){
        return NULL;
    }
    struct Node *best = NULL;
    int max = INT_MIN;
    for (int i = 0; i < node->childrenCount; ++i) {
        if (node->children[i]->rating > max) {
            best = node->children[i];
            max = node->children[i]->rating;
        }
    }
    return best;
}


#ifdef DEBUG
void printNode(struct Node* node, int level) {
    if (node == NULL) {
        return;
    }
    for (int i = 0; i < level; ++i) {
        printf("    ");
    }
    char moveStr[10]; 
    moveIntToChar(node->move, moveStr);

    printf("Move: %s, Status: %d, Rating: %d\r\n", moveStr, node->status, node->rating);

    for (int i = 0; i < node->childrenCount; ++i) {
        printNode(node->children[i], level + 1);
    }
}

void printTree(void) {
    printf("Tree Structure:\n");
    printNode(root, 0);
}

void moveIntToChar(int64_t move, char* result) {
    for (int i = 7; i >= 0; --i) {
        result[7 - i] = (char)((move >> (i * 8)) & 0xFF);
    }
    result[8] = '\0'; // Null-terminate the string
}
#endif
