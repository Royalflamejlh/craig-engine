#include "tree.h"
#include "movement.h"
#include "board.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static void getLegalPawnMoves(struct Node *node, char x, char y);
static void getLegalBishopMoves(struct Node *node, char x, char y);
static void getLegalKnightMoves(struct Node *node, char x, char y);
static void getLegalRookMoves(struct Node *node, char x, char y);
static void getLegalKingMoves(struct Node *node, char x, char y);
static char inCheck(char board[8][8], char color);

/*
* Takes in a node and generates all legal moves for that node
*/
void buildLegalMoves(struct Node *node){
    for (char y = 0; y < 8; y++) {
        for (char x = 0; x < 8; x++) {
            char piece = node->board[y][x];
            if (piece == ' ' || (node->color == 'B' && islower(piece)) || (node->color == 'W' && isupper(piece))) {
                continue;
            }
            switch (piece) {
                case 'P':
                case 'p':
                    getLegalPawnMoves(node, x, y);
                    break;
                case 'B': 
                case 'b':
                    getLegalBishopMoves(node, x, y);
                    break;
                case 'N': 
                case 'n':
                    getLegalKnightMoves(node, x, y);
                    break;
                case 'R': 
                case 'r':
                    getLegalRookMoves(node, x, y);
                    break;
                case 'Q': 
                case 'q':
                    getLegalBishopMoves(node, x, y);
                    getLegalRookMoves(node, x, y);
                    break;
                case 'K': 
                case 'k':
                    getLegalKingMoves(node, x, y);
                    break;
                default:
                    break;
            }
        }
    }
}

static int64_t makeMove(char from_x, char from_y, char to_x, char to_y, char promotion){
    int64_t result = 0;
    result = from_x;
    result = (result << 8) | from_y;
    result = (result << 8) | to_x;
    result = (result << 8) | to_y;
    if (promotion != ' ') {
        result = (result << 8) | promotion;
    }
    return result;
}


static void addMoveIfValid(struct Node *node, char from_x, char from_y, char to_x, char to_y, char promotion) {
    //Make sure move in on the board
    if( (from_x >= 8 || from_x < 0) ||
        (from_y >= 8 || from_y < 0) ||
          (to_x >= 8 || to_x < 0)   ||
          (to_y >= 8 || to_y < 0))  return;
    
    
    int64_t move = makeMove(from_x, from_y, to_x, to_y, promotion);

    char board[8][8];
    memcpy(&board[0][0], &node->board[0][0], 8*8*sizeof(char));
    receiveMove(board, move);
    if(inCheck(board, node->color)){
        return;
    }

    //Create a child node
    addTreeNode(node, move, STATUS_PREDICTED, 0);
}

/**
* Returns 1 if the color is in check, 0 if not, -1 if king not found
* board[y][x]
* 0,0 = A1
* 1,0 = A2
*/
static char inCheck(char board[8][8], char color){
    int kingRow, kingCol, found;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((color == 'W' && board[row][col] == 'K') || (color == 'B' && board[row][col] == 'k')) {
                kingRow = row;
                kingCol = col;
                found = 1;
                goto checkKing;
            }
        }
    }

checkKing:
    if(found != 1){
        return -1;
    }

    //Iterate N
    for(int i = kingRow; i < 8; i++){
        char piece = board[i][kingCol];

        if(piece == ' ') continue;

        if(piece == 'N' || piece == 'n' || 
           piece == 'B' || piece == 'b' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'R' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'r' && color == 'W') return 1;

    }
    //Iterate W
    for(int i = kingCol; i < 8; i++){
        char piece = board[kingRow][i];

        if(piece == ' ') continue;

        if(piece == 'N' || piece == 'n' || 
           piece == 'B' || piece == 'b' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'R' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'r' && color == 'W') return 1;

    }
    //Iterate S
    for(int i = kingRow; i >= 0; i--){
        char piece = board[i][kingCol];

        if(piece == ' ') continue;

        if(piece == 'N' || piece == 'n' || 
           piece == 'B' || piece == 'b' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'R' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'r' && color == 'W') return 1;

    }
    //Iterate E
    for(int i = kingCol; i >= 0; i--){
        char piece = board[kingRow][i];

        if(piece == ' ') continue;

        if(piece == 'N' || piece == 'n' || 
           piece == 'B' || piece == 'b' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'R' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'r' && color == 'W') return 1;

    }


    // Iterate NW
    for(int i = 1; (kingRow - i >= 0) && (kingCol - i >= 0); i++){
        char piece = board[kingRow - i][kingCol - i];

        if(piece == ' ') continue;
        if(piece == 'N' || piece == 'n' || 
           piece == 'R' || piece == 'r' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'B' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'b' && color == 'W') return 1;
    }

    // Iterate SW
    for(int i = 1; (kingRow + i < 8) && (kingCol - i >= 0); i++){
        char piece = board[kingRow + i][kingCol - i];

        if(piece == ' ') continue;
        if(piece == 'N' || piece == 'n' || 
           piece == 'R' || piece == 'r' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'B' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'b' && color == 'W') return 1;
    }

    // Iterate SE
    for(int i = 1; (kingRow + i < 8) && (kingCol + i < 8); i++){
        char piece = board[kingRow + i][kingCol + i];

        if(piece == ' ') continue;
        if(piece == 'N' || piece == 'n' || 
           piece == 'R' || piece == 'r' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'B' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'b' && color == 'W') return 1;
    }

    // Iterate NE
    for(int i = 1; (kingRow - i >= 0) && (kingCol + i < 8); i++){
        char piece = board[kingRow - i][kingCol + i];

        if(piece == ' ') continue;
        if(piece == 'N' || piece == 'n' || 
           piece == 'R' || piece == 'r' ||
           piece == 'P' || piece == 'p') break;

        if(piece == 'K' && color == 'B') return 1;
        if(piece == 'Q' && color == 'B') return 1;
        if(piece == 'B' && color == 'B') return 1;

        if(piece == 'k' && color == 'W') return 1;
        if(piece == 'q' && color == 'W') return 1;
        if(piece == 'b' && color == 'W') return 1;
    }



    //Check for Knights
    int knightMoves[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
    for (int i = 0; i < 8; i++) {
        int newRow = kingRow + knightMoves[i][0];
        int newCol = kingCol + knightMoves[i][1];

        if (newRow >= 0 && newRow < 8 && newCol >= 0 && newCol < 8) {
            char piece = board[newRow][newCol];
            if ((piece == 'N' && color == 'B') || (piece == 'n' && color == 'W')) {
                return 1;
            }
        }
    }
}



static void getLegalPawnMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];
    char direction = isupper(piece) ? 1 : -1;

    char forward_y = y + direction;
    if (node->board[forward_y][x] == ' ') {
        addMoveIfValid(node, x, y, x, forward_y, ' ');

        if ((isupper(piece) && y == 1) || (islower(piece) && y == 6)) {
            char twoForward_y = y + 2 * direction;
            if (node->board[twoForward_y][x] == ' ') {
                addMoveIfValid(node, x, y, x, twoForward_y, ' ');
            }
        }
    }

    for (int dx = -1; dx <= 1; dx += 2) { 
        char capture_x = x + dx;
        char capture_y = y + direction;

        if (capture_x >= 0 && capture_x < 8 && capture_y >= 0 && capture_y < 8) {
            char target = node->board[capture_y][capture_x];
            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                addMoveIfValid(node, x, y, capture_x, capture_y, ' ');
            }
        }
    }
}

static void getLegalRookMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];

    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; 

    for (int i = 0; i < 4; i++) {
        char dx = directions[i][1];
        char dy = directions[i][0];

        char new_x = x + dx;
        char new_y = y + dy;

        while (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if ((isupper(piece) && isupper(target)) || (islower(piece) && islower(target))) {
                break;
            }

            addMoveIfValid(node, x, y, new_x, new_y, ' ');

            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                break;
            }

            new_x += dx;
            new_y += dy;
        }
    }
}


static void getLegalBishopMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];

    int directions[4][2] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}}; 

    for (int i = 0; i < 4; i++) {
        char dx = directions[i][1];
        char dy = directions[i][0];

        char new_x = x + dx;
        char new_y = y + dy;

        while (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if ((isupper(piece) && isupper(target)) || (islower(piece) && islower(target))) {
                break;
            }

            addMoveIfValid(node, x, y, new_x, new_y, ' ');

            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                break;
            }

            new_x += dx;
            new_y += dy;
        }
    }
}


static void getLegalKnightMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];
    int moves[8][2] = {{-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}};

    for (int i = 0; i < 8; i++) {
        char new_x = x + moves[i][1];
        char new_y = y + moves[i][0];

        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if (!((isupper(piece) && islower(target)) || (islower(piece) && isupper(target)) || target == ' ')) {
                continue;
            }

            addMoveIfValid(node, x, y, new_x, new_y, ' ');
        }
    }
}


static void getLegalKingMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];

    int moves[8][2] = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1},
                       {1, 0}, {1, -1}, {0, -1}, {-1, -1}};

    for (int i = 0; i < 8; i++) {
        char new_x = x + moves[i][1];
        char new_y = y + moves[i][0];

        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if (!((isupper(piece) && isupper(target)) || (islower(piece) && islower(target)))) {
                addMoveIfValid(node, x, y, new_x, new_y, ' ');
            }
        }
    }
}
