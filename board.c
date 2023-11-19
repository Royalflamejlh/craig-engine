#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "board.h"

//BOARD[y][x]
// 0,0 = A1
// 1,0 = A2

//UPPERCASE CHARACTERS ARE WHITE
//lowercase characters are black

void initializeBoard(char board[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = ' ';
        }
    }

    char initialPieces[8] = {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'};
    for (int i = 0; i < 8; i++) {
        board[0][i] = initialPieces[i];
        board[7][i] = tolower(initialPieces[i]);
        board[1][i] = 'P';
        board[6][i] = 'p';
    }
}


void printBoard(char board[8][8]){
    for (int i = 7; i >= 0; i--) {
        for (int j = 0; j < 8; j++) {
            printf("%c ", board[i][j]);
        }
        printf("\n");
    }
}

void receiveMove(char board[8][8], int64_t move) {
    char to_x   = (move >> 8) & 0xFF;
    char to_y   = move & 0xFF;
    char from_x = (move >> 24) & 0xFF;
    char from_y = (move >> 16) & 0xFF;
    char promotion = (move >> 32) & 0xFF; 

    board[to_y][to_x] = board[from_y][from_x];
    board[from_y][from_x] = ' ';

    if (promotion != 0) {
        board[to_y][to_x] = promotion;
    }
}


