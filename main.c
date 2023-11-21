#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include "util.h"
#include "tree.h"
#include "board.h"

#define DEPTH_SCAN 4
#define DEPTH_DEEP 10

static void processUCI(void);
static void processIsReady(void);
static int processInput(char* input);
static void processMoves(char* str);
static void sendBestMove();

int main(void) {
    char input[1024];
    input[1023] = '\0';

    initializeTree();

    while (true) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }
        if(processInput(input)){
            break;
        }
    }

    return 0;
}

static int processInput(char* input){
    if (strncmp(input, "uci", 3) == 0) {
        processUCI();
        return 0;
    } 
    else if (strncmp(input, "isready", 7) == 0) {
        processIsReady();
        return 0;
    }
    else if (strncmp(input, "position", 8) == 0) {
        input += 9;
        if (strncmp(input, "startpos", 8) == 0) {
            input += 9;
            if (strncmp(input, "moves", 5) == 0) {
                input += 6;
                processMoves(input);
                return 0;
            }
        }
        else if (strncmp(input, "fen", 3) == 0) {
            printf("info string Loading FEN not supported\r\n");
            fflush(stdout);
            //sendBestMove();
            return 0;
        }
    }
    else if (strncmp(input, "quit", 4) == 0) {
        return -1; 
    }
    return 0;
}

/*
* Processes a list of moves such as e2e3 a3a4 b2b3
* makes sure these nodes are in the tree and marks them as played
* also will prune the branches of all already played moves
* except for the children of the last node
*/
void processMoves(char* str){

    struct Node* it = getTreeRoot();
    struct Node* prevIt = it;

    char* pch = strtok(str, " ");
    while (pch != NULL) {
        char* moveChar = trimWhitespace(pch);
        int64_t moveInt = moveCharToInt(moveChar); 

        struct Node* nextIt = iterateTree(it, moveInt);
        if (nextIt == NULL) {
            prevIt = it;
            it = addTreeNode(prevIt, moveInt, 0);
        } else {
            updateNodeStatus(it, 0);
            prevIt = it;
            it = nextIt;
        }
        pch = strtok(NULL, " ");
    }

    updateNodeStatus(it, 2);
    buildTreeMoves(DEPTH_SCAN);

    printCurNode();

    deepSearchTree(DEPTH_SCAN-2, DEPTH_DEEP);
    

    sendBestMove();

}


static void processUCI(void) {
    printf("id name CraigEngine\r\n");
    printf("id author John\r\n");
    printf("uciok\r\n");
    fflush(stdout);
}

static void processIsReady(void) {
    printf("readyok\r\n");
    fflush(stdout);
}


 static void sendBestMove(){
    struct Node* node = getBestCurChild();
    char move[6];
    moveIntToChar(node->move, move);
    printf("bestmove %s\r\n", move);
    printf("Move %s found for %c with a board score of %d\n\r", move, node->color, node->rating);
    fflush(stdout);
 }
 
