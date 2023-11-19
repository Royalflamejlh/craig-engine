#include "tree.h"
#include "movement.h"
#include <ctype.h>

static void getLegalPawnMoves(struct Node *node, char x, char y);
/*
* Takes in a node and generates all legal moves for that node
*/
void buildLegalMoves(struct Node *node){
    for (char y = 0; y < 8; y++) {
        for (char x = 0; x < 8; x++) {
            char piece = node->board[y][x];
            if (piece == ' ' || (node->color == 0 && islower(piece)) || (node->color == 1 && isupper(piece))) {
                continue;
            }
            switch (piece) {
                case 'P':
                case 'p':
                    getLegalPawnMoves(node, x, y);
                    break;
                case 'B': 
                case 'b':
                    //getLegalBishopMoves(node, x, y);
                    break;
                default:
                    break;
            }
        }
    }
}

static int64_t makeMove(char from_x, char from_y, char to_x, char to_y){
    int64_t result = 0;
    result = from_x;
    result = (result << 8) | from_y;
    result = (result << 8) | to_x;
    result = (result << 8) | to_y;
    return result;
}


static void addMoveIfValid(struct Node *node, char from_x, char from_y, char to_x, char to_y) {
    //Make sure the not mate function here
    int64_t move = makeMove(from_x, from_y, to_x, to_y);
    addTreeNode(node, move, STATUS_PREDICTED, 0);
}


static void getLegalPawnMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];
    char direction = isupper(piece) ? 1 : -1;

    char forward_y = y + direction;
    if (node->board[forward_y][x] == ' ') {
        addMoveIfValid(node, x, y, x, forward_y);

        if ((isupper(piece) && y == 1) || (islower(piece) && y == 6)) {
            char twoForward_y = y + 2 * direction;
            if (node->board[twoForward_y][x] == ' ') {
                addMoveIfValid(node, x, y, x, twoForward_y);
            }
        }
    }

    for (int dx = -1; dx <= 1; dx += 2) { 
        char capture_x = x + dx;
        char capture_y = y + direction;

        if (capture_x >= 0 && capture_x < 8 && capture_y >= 0 && capture_y < 8) {
            char target = node->board[capture_y][capture_x];
            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                addMoveIfValid(node, x, y, capture_x, capture_y);
            }
        }
    }
}
