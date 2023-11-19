#ifndef GAY
#include "tree.h"


/*
* Takes in a node and generates all legal moves for that node
*/
void getLegalMoves(struct Node *node){
    char board[8][8] = node->board;
    *moveCount = 0;
    int64_t* moves;

    for (char y = 0; y < 8; y++) {
        for (char x = 0; x < 8; x++) {
            char piece = board[y][x];
            if (piece == ' ' || (turn == 0 && islower(piece)) || (turn == 1 && isupper(piece))) {
                continue;
            }
            switch (piece) {
                case 'P':
                case 'p':
                    getLegalPawnMoves(board, x, y, moves, moveCount);
                    break;
                case 'B': 
                case 'b':
                    getLegalBishopMoves(board, x, y, moves, moveCount);
                    break;
                default:
                    break;
            }
        }
    }
}


void addMoveIfValid(char board[8][8], int64_t* moves, int* moveCount, char from_x, char from_y, char to_x, char to_y) {
    if (to_x < 0 || to_x >= 8 || to_y < 0 || to_y >= 8) {
        return;
    }
    moves[*moveCount].from_x = from_x;
    moves[*moveCount].from_y = from_y;
    moves[*moveCount].to_x = to_x;
    moves[*moveCount].to_y = to_y;
    (*moveCount)++;
}


void getLegalPawnMoves(char board[8][8], char x, char y, int64_t* moves, int* moveCount) {
    char piece = board[y][x];
    char direction = isupper(piece) ? 1 : -1;

    char forward_y = y + direction;
    if (board[forward_y][x] == ' ') {
        addMoveIfValid(board, moves, moveCount, x, y, x, forward_y);

        if ((isupper(piece) && y == 1) || (islower(piece) && y == 6)) {
            char twoForward_y = y + 2 * direction;
            if (board[twoForward_y][x] == ' ') {
                addMoveIfValid(board, moves, moveCount, x, y, x, twoForward_y);
            }
        }
    }

    for (int dx = -1; dx <= 1; dx += 2) { 
        char capture_x = x + dx;
        char capture_y = y + direction;

        if (capture_x >= 0 && capture_x < 8 && capture_y >= 0 && capture_y < 8) {
            char target = board[capture_y][capture_x];
            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                addMoveIfValid(board, moves, moveCount, x, y, capture_x, capture_y);
            }
        }
    }
}
#endif
