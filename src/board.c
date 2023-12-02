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



void receiveMove(char board[8][8], struct Move move) {
    char promotion = move.promotion;
    unsigned char to_y   = move.to_y;
    unsigned char to_x   = move.to_x;
    unsigned char from_y = move.from_y;
    unsigned char from_x = move.from_x;

    //Castling
    if((board[from_y][from_x] == 'K' || board[from_y][from_x] == 'k') && (abs(from_x - to_x) == 2)){
        if(to_y == 0){
            //White Short Castle
            if(to_x == 6){
                board[0][6] = board[0][4]; //Set the King
                board[0][5] = board[0][7]; //Set the rook
                board[0][4] = ' '; //Remove the king
                board[0][7] = ' '; //Remove the rook
                
            }
            //White Long Castle
            else{
                board[0][2] = board[0][4]; //Set the King
                board[0][3] = board[0][0]; //Set the rook
                board[0][4] = ' '; //Remove the king
                board[0][0] = ' '; //Remove the rook
            }
        }
        else{
            //Black Short Castle
            if(to_x == 6){
                board[7][6] = board[7][4]; //Set the King
                board[7][5] = board[7][7]; //Set the rook
                board[7][4] = ' '; //Remove the king
                board[7][7] = ' '; //Remove the rook
            }
            //Black Long Castle
            else{
                board[7][2] = board[7][4]; //Set the King
                board[7][3] = board[7][0]; //Set the rook
                board[7][4] = ' '; //Remove the king
                board[7][0] = ' '; //Remove the rook
            }
        } 
        return;
    }

    //En-passant -- check if moving diag, and if not capturing, then en-passant
    if((board[from_y][from_x] == 'P' || board[from_y][from_x] == 'p') &&
        (to_x != from_x) && (board[to_x][to_y] == ' ')){
            board[from_y][to_x] = ' ';
    }


    board[to_y][to_x] = board[from_y][from_x];
    board[from_y][from_x] = ' ';

    //Promotions
    if (promotion == 'q' || 
        promotion == 'b' ||
        promotion == 'r' ||
        promotion == 'n') {
        if(to_y == 7) promotion = toupper(promotion);
        board[to_y][to_x] = promotion;
    } 
}


