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
#include <ctype.h>
#include "mempool.h"



static size_t root_idx;
static size_t cur_idx;


static void propagateRating(size_t node);
static void buildTreeMovesHelper(size_t node, int depth);
static void deepSearchHelper(size_t node, int depth);
static void parseBoardFromFEN(const char* boardFEN, char board[8][8]);
static void parseCastlingFromFEN(const char* castlingFEN, unsigned char* castle);
static void freeTree(size_t node);

//Initialize the move tree at the start of the game
void initializeTree(void){
    freeTree(root_idx);
    root_idx = allocateNode();
    struct Node* root = getNode(root_idx);
    struct Move emptyMove = {0, 0, 0, 0, 0};
    root->move = emptyMove;
    root->status = STATUS_ROOT;
    root->castle = 0xF0;
    root->color = 'B';
    initializeBoard(root->board);
    root->rating = INT_MIN;
    root->parent = NULL_NODE;
    root->children = NULL;
    root->childrenCount = 0;
    cur_idx = root_idx;
}

void initializeTreeFEN(char* FEN) {
    freeTree(root_idx);
    root_idx = allocateNode();
    struct Node* root = getNode(root_idx);
    struct Move emptyMove = {0, 0, 0, 0, 0};
    root->move = emptyMove;
    root->status = STATUS_ROOT;
    root->castle = 0;
    root->rating = INT_MIN;
    root->parent = NULL_NODE;
    root->children = NULL;
    root->childrenCount = 0;
    cur_idx = root_idx;

    char* token;
    char* rest = FEN;

    token = strtok_r(rest, " ", &rest);
    parseBoardFromFEN(token, root->board);

    token = strtok_r(NULL, " ", &rest);
    root->color = (token[0] == 'w') ? 'W' : 'B';

    token = strtok_r(NULL, " ", &rest);
    parseCastlingFromFEN(token, &(root->castle));
}

static void parseBoardFromFEN(const char* boardFEN, char board[8][8]) {
    int rank = 7, file = 0;

    for (int i = 0; boardFEN[i] != '\0'; ++i) {
        if (isdigit(boardFEN[i])) {
            file += boardFEN[i] - '0';
        } else if (boardFEN[i] == '/') {
            rank--;
            file = 0;
        } else {
            board[rank][file] = boardFEN[i];
            file++;
        }
    }
}

static void parseCastlingFromFEN(const char* castlingFEN, unsigned char* castle) {
    for (int i = 0; castlingFEN[i] != '\0'; ++i) {
        switch (castlingFEN[i]) {
            case 'K': *castle |= WHITE_CASTLE_SHORT; break;
            case 'Q': *castle |= WHITE_CASTLE_LONG; break;
            case 'k': *castle |= BLACK_CASTLE_SHORT; break;
            case 'q': *castle |= BLACK_CASTLE_LONG; break;
        }
    }
}

/*
* Get the root node of a tree
*/
size_t getTreeRoot(void){
    return root_idx;
}

/*
* Add a node to the tree
*/
size_t addTreeNode(size_t parent, struct Move move, char status) {
    size_t node = allocateNode();
    struct Node* newNode = getNode(node);
    if (node == NULL_NODE || newNode == NULL) {
        return NULL_NODE;
    }
    
    newNode->move = move;
    newNode->status = status;
    newNode->parent = parent;
    newNode->children = NULL;
    newNode->childrenCount = 0;

    memcpy(&newNode->board[0][0], &getNode(parent)->board[0][0], 8*8*sizeof(char));
    receiveMove(newNode->board, move);

    newNode->castle = updateCastling(getNode(parent)->castle, move);

    newNode->color = 'W';
    newNode->rating = INT_MAX;
    if (getNode(parent)->color == 'W') {
        newNode->color = 'B';
        newNode->rating = INT_MIN;
    }

    struct Node* parentNode = getNode(parent);
    if (parentNode == NULL) {
        return NULL_NODE;
    }

    if (parentNode->children == NULL) {
        parentNode->children = malloc(sizeof(size_t) * (parentNode->childrenCount + 1));
    } else {
        size_t* tempChildren = realloc(parentNode->children, sizeof(size_t) * (parentNode->childrenCount + 1));
        if (tempChildren == NULL) {
            printf("info string Error in realloc\n\r");
            fflush(stdout);
            freeNode(node);
            return NULL_NODE;
        }
        parentNode->children = tempChildren;
    }

    
    if (parentNode->children == NULL) {
        printf("info string Error in malloc/realloc\n\r");
        fflush(stdout);
        freeNode(node);
        return NULL_NODE;
    }

    parentNode->children[parentNode->childrenCount] = node;
    parentNode->childrenCount++;

    if(status == STATUS_CURRENT){
        pruneAbove(node);
        cur_idx = node;
    }

    return node;
}

/*
* Update the status of a node
*/
void updateNodeStatus(size_t node, char status) {
    
    if(status == STATUS_CURRENT){
        pruneAbove(node);
        cur_idx = node;
    }
    getNode(node)->status = status;
}

/*
* Iterate through a tree by putting in a move, 
*/
size_t iterateTree(size_t it, struct Move move) {
    
    for (int i = 0; i < getNode(it)->childrenCount; i++) {
        if (getNode(getNode(it)->children[i])->move.from_x == move.from_x &&
            getNode(getNode(it)->children[i])->move.from_y == move.from_y &&
            getNode(getNode(it)->children[i])->move.to_x == move.to_x &&
            getNode(getNode(it)->children[i])->move.to_y == move.to_y &&
            getNode(getNode(it)->children[i])->move.promotion == move.promotion
        ) {
            return getNode(it)->children[i];
        }
    }

    return NULL_NODE;
}


 
/*
* Frees a tree from a node
*/
static void freeTree(size_t node_idx) {
    if(node_idx == NULL_NODE){
        return;
    }
    struct Node* node = getNode(node_idx);
    if(node == NULL){
        return;
    }
    for (int i = 0; i < node->childrenCount; i++) {
        freeTree(node->children[i]);
    }

    free(node->children);
    freeNode(node_idx);
}


void pruneNodeExceptFor(size_t node_idx, size_t exceptNode) {
    struct Node* node = getNode(node_idx);
    for (int i = 0; i < node->childrenCount; ++i) {
        if (node->children[i] != exceptNode) {
            freeTree(node->children[i]);
        }
    }

    if (exceptNode != NULL_NODE) {
        node->children[0] = exceptNode;
        node->childrenCount = 1;

        size_t* temp = realloc(node->children, sizeof(size_t));
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

void pruneAbove(size_t current) {
    size_t child = current;
    size_t parent = getNode(current)->parent;

    while (parent != NULL_NODE) {
        pruneNodeExceptFor(parent, child);

        child = parent;
        parent = getNode(parent)->parent;
    }
}

size_t getBestChild(size_t node_idx){
    if (node_idx == NULL_NODE || getNode(node_idx)->childrenCount == 0) {
        return NULL_NODE;
    }
    struct Node* node = getNode(node_idx);

    size_t best = node->children[0];
    int bestRating = getNode(node->children[0])->rating;

    for (int i = 1; i < node->childrenCount; ++i) {
        int childRating = getNode(node->children[i])->rating;

        //Note if node->color='B' you are looking for the best move for 'W'
        if (((node->color == 'W') && childRating < bestRating) ||
            ((node->color == 'B') && childRating > bestRating)) {
            best = node->children[i];
            bestRating = childRating;
        }
    }

    return best;
}


size_t getBestCurChild(void){
    size_t child = getBestChild(cur_idx);
    if(child == NULL_NODE) buildTreeMoves(1);
    return getBestChild(cur_idx);
}


void buildTreeMoves(int depth){
    if(cur_idx == NULL_NODE){
        return;
    }
    buildTreeMovesHelper(cur_idx, depth);
}

static void buildTreeMovesHelper(size_t node_idx, int depth){
    if(depth <= 0){
        updateRating(node_idx);
        propagateRating(node_idx);
        return;
    }
    struct Node* node = getNode(node_idx);
    if(node->childrenCount == 0){
        buildLegalMoves(node_idx);
        node = getNode(node_idx);
        if(node == NULL) return;
    }
    for (int i = 0; i < node->childrenCount; i++) {
        buildTreeMovesHelper(node->children[i], depth - 1);
        node = getNode(node_idx);
        if(node == NULL) return;
    }
}

static void propagateRating(size_t node_idx) {
    if (node_idx == NULL_NODE || getNode(node_idx)->parent == NULL_NODE) {
        return;
    }
    
    struct Node* node = getNode(node_idx);
    size_t parent_idx = node->parent;
    struct Node* parent = getNode(parent_idx);

    if (((node->color == 'W') && parent->rating < node->rating) ||
        ((node->color == 'B') && parent->rating > node->rating)) {
        parent->rating = node->rating;
        propagateRating(parent_idx);
    }
}

/*
* Below here is the ever so feared DEEP SEARCH logic...
*/
void deepSearchTree(int starting_depth, int depth){
    size_t node = cur_idx;
    size_t prev = cur_idx;
    for(int i = 0; i < starting_depth; i++){
        node = getBestChild(node);
        if(node == NULL_NODE){
            node = prev;
            break;
        }
        prev = node;
    }
    deepSearchHelper(node, depth);
}

static void deepSearchHelper(size_t node_idx, int depth){
    struct Node* node = getNode(node_idx);
    if(node == NULL) return;
    if(depth <= 0){
        return;
    }
    if(node->children == 0){
        buildTreeMovesHelper(node_idx, 1);
        node = getNode(node_idx);
        if(node == NULL) return;
        for (int i = 0; i < node->childrenCount; i++) {
            if(depth == 0) {
                updateRating(node->children[i]);
                propagateRating(node->children[i]);
            }
            else {
                updateRatingFast(node->children[i]);
            }
        }
    }

    size_t best[DEEP_SEARCH_WIDTH];

    int maxWidth = MIN(DEEP_SEARCH_WIDTH, node->childrenCount);

    for(int i = 0; i < maxWidth; i++){
        best[i] = node->children[0];
    }


    for (int i = 1; i < node->childrenCount; ++i) {
        int childRating = getNode(node->children[i])->rating;
        for(int j = 0; j < maxWidth; j++){
            if ((((node->color == 'W') && childRating < getNode(best[j])->rating) ||
                ((node->color == 'B') && childRating > getNode(best[j])->rating))){
                best[j] = node->children[i];
                break;
            }
        }
    }


    for(int i = 0; i < maxWidth; i++){
        if(best[i] != NULL_NODE) deepSearchHelper(best[i], depth-1);
    }
}




#ifdef DEBUG
void printNode(size_t node_idx, int level, int depth) {
    if (node_idx == NULL_NODE) {
        return;
    }
    struct Node* node = getNode(node_idx);
    if(depth == 0){
        return;
    }
    for (int i = 0; i < level; ++i) {
        printf("    ");
    }
    char moveStr[6]; 
    moveStructToStr((&node->move), moveStr);
    printf("%lu: stat:%d col:%c cast:%x rat:%d par:%lu chil[%d]@%p mov:%s\r\n",
    node_idx, node->status, node->color, node->castle, node->rating, node->parent,
    node->childrenCount, node->children, moveStr);
    printBoard(node->board);
    for (int i = 0; i < node->childrenCount; ++i) {
        printNode(node->children[i], level + 1, depth - 1);
    }
}

void printTree(void) {
    printf("Tree Structure:\n");
    printNode(root_idx, 0, 100);
}

void printCurNode(void){
    printNode(cur_idx, 0, 1);
}

#endif
