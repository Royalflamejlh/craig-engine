#include "movement.h"
#include "board.h"
#include "mempool.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

static void getLegalPawnMoves(size_t node_idx, unsigned char x, unsigned char y);
static void getLegalBishopMoves(size_t node_idx, unsigned char x, unsigned char y);
static void getLegalKnightMoves(size_t node_idx, unsigned char x, unsigned char y);
static void getLegalRookMoves(size_t node_idx, unsigned char x, unsigned char y);
static void getLegalKingMoves(size_t node_idx, unsigned char x, unsigned char y);

/*
* Takes in a node and generates all legal moves for that node
*/
void buildLegalMoves(size_t node_idx){
    for (unsigned char y = 0; y < 8; y++) {
        for (unsigned char x = 0; x < 8; x++) {
            char piece = getNode(node_idx)->board[y][x];
            if (piece == ' ' || (getNode(node_idx)->color == 'B' && islower(piece)) || (getNode(node_idx)->color == 'W' && isupper(piece))) {
                continue;
            }
            switch (piece) {
                case 'P':
                case 'p':
                    getLegalPawnMoves(node_idx, x, y);
                    break;
                case 'B': 
                case 'b':
                    getLegalBishopMoves(node_idx, x, y);
                    break;
                case 'N': 
                case 'n':
                    getLegalKnightMoves(node_idx, x, y);
                    break;
                case 'R': 
                case 'r':
                    getLegalRookMoves(node_idx, x, y);
                    break;
                case 'Q': 
                case 'q':
                    getLegalBishopMoves(node_idx, x, y);
                    getLegalRookMoves(node_idx, x, y);
                    break;
                case 'K': 
                case 'k':
                    getLegalKingMoves(node_idx, x, y);
                    break;
                default:
                    break;
            }
        }
    }
}


static void addMoveIfValid(size_t node_idx, unsigned char from_x, unsigned char from_y, unsigned char to_x, unsigned char to_y, char promotion) {
    //Make sure move in on the board
    if( (from_x >= 8 || from_x < 0) ||
        (from_y >= 8 || from_y < 0) ||
          (to_x >= 8 || to_x < 0)   ||
          (to_y >= 8 || to_y < 0))  return;
    
    
    struct Move move = {from_x, from_y, to_x, to_y, promotion};
    char board[8][8];
    memcpy(&board[0][0], &getNode(node_idx)->board[0][0], 8*8*sizeof(char));
    char piece = board[move.from_y][move.from_x];


    receiveMove(board, move);
    if(inCheck(board, getColor(piece))){
        return;
    }
    if(piece == ' '){
        printf("info string Warning tried to move empty piece");
        return;
    }

    //Create a child node
    addTreeNode(node_idx, move, STATUS_PREDICTED);
}

/**
* Returns 1 if the color is in check, 0 if not, -1 if king not found
* board[y][x]
* 0,0 = A1
* 1,0 = A2
*/
char inCheck(char board[8][8], char color){
    int kingRow = 0;
    int kingCol = 0;
    int found = 0;

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

    if(color == 'B' && (kingRow-1) >= 0){
        if((kingCol+1) < 8  && board[kingRow-1][kingCol+1] == 'P'){
            return 1;
        }
        if((kingCol-1) >= 0 && board[kingRow-1][kingCol-1] == 'P'){
            return 1;
        }
    }

    if((color == 'W') && (kingRow+1) < 8){
       if((kingCol+1) < 8  && board[kingRow+1][kingCol+1] == 'p'){
            return 1;
       }
       if((kingCol-1) >= 0 && board[kingRow+1][kingCol-1] == 'p'){
            return 1;
       }
    }

    return 0;
}



static void getLegalPawnMoves(size_t node_idx, unsigned char x, unsigned char y) {
    char piece = getNode(node_idx)->board[y][x];
    int direction = isupper(piece) ? 1 : -1;

    int forward_y = y + direction;
    if (getNode(node_idx)->board[forward_y][x] == ' ') {
        if(forward_y == 7 || forward_y == 0){
            addMoveIfValid(node_idx, x, y, x, forward_y, 'q');
            addMoveIfValid(node_idx, x, y, x, forward_y, 'n');
            addMoveIfValid(node_idx, x, y, x, forward_y, 'b');
            addMoveIfValid(node_idx, x, y, x, forward_y, 'r');
        }
        else{
            addMoveIfValid(node_idx, x, y, x, forward_y, ' ');
        }

        if ((isupper(piece) && y == 1) || (islower(piece) && y == 6)) {
            int twoForward_y = y + 2 * direction;
            if (getNode(node_idx)->board[twoForward_y][x] == ' ') {
                addMoveIfValid(node_idx, x, y, x, twoForward_y, ' ');
            }
        }
    }

    for (int dx = -1; dx <= 1; dx += 2) { 
        int capture_x = x + dx;
        int capture_y = y + direction;

        if (capture_x >= 0 && capture_x < 8 && capture_y >= 0 && capture_y < 8) {
            int target = getNode(node_idx)->board[capture_y][capture_x];
            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                if(capture_y == 7 || capture_y == 0){
                    addMoveIfValid(node_idx, x, y, capture_x, capture_y, 'q');
                    addMoveIfValid(node_idx, x, y, capture_x, capture_y, 'n');
                    addMoveIfValid(node_idx, x, y, capture_x, capture_y, 'b');
                    addMoveIfValid(node_idx, x, y, capture_x, capture_y, 'r');
                }
                else{
                    addMoveIfValid(node_idx, x, y, capture_x, capture_y, ' ');
                }
            }
        }
    }

    // En-Passant
    struct Move lastMove = getNode(node_idx)->move;
    char pieceMoved = getNode(node_idx)->board[lastMove.to_y][lastMove.to_x];

    if (tolower(pieceMoved) == 'p' && abs(lastMove.to_y - lastMove.from_y) == 2) {
        if (y == lastMove.to_y && (x == lastMove.to_x - 1 || x == lastMove.to_x + 1)) {
            int capture_y = (piece == 'P') ? lastMove.to_y - 1 : lastMove.to_y + 1;
            addMoveIfValid(node_idx, x, y, lastMove.to_x, capture_y, ' ');
        }
    }

}

static void getLegalRookMoves(size_t node_idx, unsigned char x, unsigned char y) {
    struct Node* node = getNode(node_idx);
    if(node == NULL) return;
    char piece = node->board[y][x];

    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; 

    for (int i = 0; i < 4; i++) {
        int dx = directions[i][1];
        int dy = directions[i][0];

        int new_x = x + dx;
        int new_y = y + dy;

        while (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if ((isupper(piece) && isupper(target)) || (islower(piece) && islower(target))) {
                break;
            }

            addMoveIfValid(node_idx, x, y, new_x, new_y, ' ');
            node = getNode(node_idx);
            if(node == NULL) return;

            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                break;
            }

            new_x += dx;
            new_y += dy;
        }
    }
}


static void getLegalBishopMoves(size_t node_idx, unsigned char x, unsigned char y) {
    struct Node* node = getNode(node_idx);
    if(node == NULL) return;
    char piece = node->board[y][x];

    int directions[4][2] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}}; 

    for (int i = 0; i < 4; i++) {
        int dx = directions[i][1];
        int dy = directions[i][0];

        int new_x = x + dx;
        int new_y = y + dy;

        while (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if ((isupper(piece) && isupper(target)) || (islower(piece) && islower(target))) {
                break;
            }

            addMoveIfValid(node_idx, x, y, new_x, new_y, ' ');
            node = getNode(node_idx);
            if(node == NULL) return;

            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                break;
            }

            new_x += dx;
            new_y += dy;
        }
    }
}


static void getLegalKnightMoves(size_t node_idx, unsigned char x, unsigned char y) {
    struct Node* node = getNode(node_idx);
    char piece = node->board[y][x];
    int moves[8][2] = {{-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}};

    for (int i = 0; i < 8; i++) {
        int new_x = x + moves[i][1];
        int new_y = y + moves[i][0];

        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if (!((isupper(piece) && islower(target)) || (islower(piece) && isupper(target)) || target == ' ')) {
                continue;
            }

            addMoveIfValid(node_idx, x, y, new_x, new_y, ' ');
            node = getNode(node_idx);
            if(node == NULL) return;
        }
    }
}


static void getLegalKingMoves(size_t node_idx, unsigned char x, unsigned char y) {
    struct Node* node = getNode(node_idx);
    char piece = node->board[y][x];

    int moves[8][2] = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1},
                       {1, 0}, {1, -1}, {0, -1}, {-1, -1}};

    for (int i = 0; i < 8; i++) {
        int new_x = x + moves[i][1];
        int new_y = y + moves[i][0];

        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if (!((isupper(piece) && isupper(target)) || (islower(piece) && islower(target)))) {
                addMoveIfValid(node_idx, x, y, new_x, new_y, ' ');
                node = getNode(node_idx);
                if(node == NULL) return;
            }
        }
    }

    if(inCheck(node->board, getColor(piece))){
        return;
    }

    if ((node->castle & WHITE_CASTLE_SHORT) && piece == 'K') {
        if (node->board[y][5] == ' ' && node->board[y][6] == ' ') {
            node->board[y][5] = piece;
            node->board[y][x] = ' ';
            if(inCheck(node->board, getColor(piece)) == 0){
                node->board[y][5] = ' ';
                node->board[y][x] = piece;
                
                addMoveIfValid(node_idx, x, y, 6, y, ' ');
                node = getNode(node_idx);
                if(node == NULL) return;
            }
        }
    }
    if ((node->castle & WHITE_CASTLE_LONG) && piece == 'K') {
        if (node->board[y][1] == ' ' && node->board[y][2] == ' ' && node->board[y][3] == ' ') {
            node->board[y][3] = piece;
            node->board[y][x] = ' ';
            if(inCheck(node->board, getColor(piece)) == 0){
                node->board[y][3] = ' ';
                node->board[y][x] = piece;
                
                addMoveIfValid(node_idx, x, y, 2, y, ' ');
                node = getNode(node_idx);
                if(node == NULL) return;
            }
        }
    }
    if ((node->castle & BLACK_CASTLE_SHORT) && piece == 'k') {
        if (node->board[y][5] == ' ' && node->board[y][6] == ' ') {
            node->board[y][5] = piece;
            node->board[y][x] = ' ';
            if(inCheck(node->board, getColor(piece)) == 0){
                node->board[y][5] = ' ';
                node->board[y][x] = piece;
                
                addMoveIfValid(node_idx, x, y, 6, y, ' ');
                node = getNode(node_idx);
                if(node == NULL) return;
            }
        }
    }
    if ((node->castle & BLACK_CASTLE_LONG) && piece == 'k') {
        if (node->board[y][1] == ' ' && node->board[y][2] == ' ' && node->board[y][3] == ' ') {
            node->board[y][3] = piece;
            node->board[y][x] = ' ';
            if(inCheck(node->board, getColor(piece)) == 0){
                node->board[y][3] = ' ';
                node->board[y][x] = piece;
                
                addMoveIfValid(node_idx, x, y, 2, y, ' ');
                node = getNode(node_idx);
                if(node == NULL) return;
            }
        }
    }
    
}
