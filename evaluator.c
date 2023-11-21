#include "board.h"
#include "evaluator.h"
#include "movement.h"
#include <ctype.h>
#include <stdlib.h>


#define PAWN_VALUE 1000
#define PAWN_POSITION_MULT 1
#define PAWN_PROTECT_BONUS 5
#define PAWN_CENTER_BONUS_GOOD 100
#define PAWN_CENTER_BONUS_GREAT 150
#define PAWN_CHAIN_BONUS 10

#define BISHOP_VALUE 3000
#define BISHOP_PROTECT_BONUS 40
#define BISHOP_OPEN_BONUS 10
#define BISHOP_UNDEVELOP_PUNISH 100

#define KNIGHT_VALUE 2900
#define KNIGHT_EDGE_PUNISH_HARD 200
#define KNIGHT_EDGE_PUNISH_SOFT 20
#define KNIGHT_PROTECT_BONUS 40
#define KNIGHT_MOVE_BONUS 10

#define ROOK_VALUE 5000
#define ROOK_PROTECT_BONUS 100
#define ROOK_OPEN_BONUS 10

#define QUEEN_VALUE 10000
#define QUEEN_MOVE_BONUS 100
#define QUEEN_PROTECT_BONUS 1000

#define KING_VALUE 1000000



static char isChained(char board[8][8], int i, int j);
static char isBishopOpen(char board[8][8], int x, int y);
static char isRookOpen(char board[8][8], int x, int y);
static char isProtected(char board[8][8], int x, int y);
static char canKnightMove(char board[8][8], int x, int y);
static char canQueenMove(char board[8][8], int x, int y);

// 0,0 = A1
// 1,0 = A2

int getRating(char board[8][8]){
    int score = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            switch (piece) {
                case 'P':
                    score += PAWN_VALUE;
                    score += ((i) * (PAWN_POSITION_MULT + abs(j-4))); //Value moving center pawns more
                    if(isChained(board, i, j)) score += PAWN_CHAIN_BONUS;
                    if(isProtected(board, i, j)) score += PAWN_PROTECT_BONUS;
                    if(i >= 2 && i <= 5 && j >= 2 && j <= 5) score += PAWN_CENTER_BONUS_GOOD;
                    if(i >= 3 && i <= 4 && j >= 3 && j <= 4) score += PAWN_CENTER_BONUS_GREAT;
                    break;
                case 'p':
                    score -= ((7-i) * (PAWN_POSITION_MULT + abs(j-4)));
                    score -= PAWN_VALUE;
                    if(isChained(board, i, j)) score -= PAWN_CHAIN_BONUS;
                    if(isProtected(board, i, j)) score -= PAWN_PROTECT_BONUS;
                    if(i >= 2 && i <= 5 && i >= 2 && i <= 5) score -= PAWN_CENTER_BONUS_GOOD;
                    if(i >= 3 && i <= 4 && j >= 3 && j <= 4) score += PAWN_CENTER_BONUS_GREAT;
                    break;

                case 'B': 
                    score += BISHOP_VALUE;
                    if(isProtected(board, i, j)) score += BISHOP_PROTECT_BONUS;
                    if(isBishopOpen(board, i, j)) score += BISHOP_OPEN_BONUS;
                    if(i == 0 && (j == 2 || j == 5)) score -= BISHOP_UNDEVELOP_PUNISH;
                    break;
                case 'b':
                    score -= BISHOP_VALUE;
                    if(isProtected(board, i, j)) score -= BISHOP_PROTECT_BONUS;
                    if(isBishopOpen(board, i, j)) score -= BISHOP_OPEN_BONUS;
                    if(i == 8 && (j == 2 || j == 5)) score += BISHOP_UNDEVELOP_PUNISH;
                    break;

                case 'N':
                    score += KNIGHT_VALUE;
                    if(i == 7 || j == 7 || i == 0 || j == 0) score -= KNIGHT_EDGE_PUNISH_HARD;
                    if(i == 6 || j == 6 || i == 1 || j == 1) score -= KNIGHT_EDGE_PUNISH_SOFT;
                    if(isProtected(board, i, j)) score += KNIGHT_PROTECT_BONUS;
                    if(canKnightMove(board, i, j)) score += KNIGHT_MOVE_BONUS;
                    break;
                case 'n':
                    score -= KNIGHT_VALUE;
                    if(i == 7 || j == 7 || i == 0 || j == 0) score += KNIGHT_EDGE_PUNISH_HARD;
                    if(i == 6 || j == 6 || i == 1 || j == 1) score += KNIGHT_EDGE_PUNISH_SOFT;
                    if(isProtected(board, i, j)) score -= KNIGHT_PROTECT_BONUS;
                    if(canKnightMove(board, i, j)) score -= KNIGHT_MOVE_BONUS;
                    break;

                case 'R': 
                    score += ROOK_VALUE;
                    if(isRookOpen(board, i, j)) score += ROOK_OPEN_BONUS;
                    if(isProtected(board, i, j)) score += ROOK_PROTECT_BONUS;
                    break;
                case 'r':
                    score -= ROOK_VALUE;
                    if(isRookOpen(board, i, j)) score -= ROOK_OPEN_BONUS;
                    if(isProtected(board, i, j)) score -= ROOK_PROTECT_BONUS;
                    break;

                case 'Q':
                    score += QUEEN_VALUE;
                    if(isRookOpen(board, i, j)) score += ROOK_OPEN_BONUS;
                    if(isProtected(board, i, j)) score += QUEEN_PROTECT_BONUS;
                    if(isBishopOpen(board, i, j)) score += BISHOP_OPEN_BONUS;
                    if(canQueenMove(board, i, j)) score += QUEEN_MOVE_BONUS;
                    break;
                case 'q':
                    score -= QUEEN_VALUE;
                    if(isRookOpen(board, i, j)) score -= ROOK_OPEN_BONUS;
                    if(isProtected(board, i, j)) score -= QUEEN_PROTECT_BONUS;
                    if(isBishopOpen(board, i, j)) score -= BISHOP_OPEN_BONUS;
                    if(canQueenMove(board, i, j)) score -= QUEEN_MOVE_BONUS;
                    break;

                case 'K': 
                    score += KING_VALUE;
                    break;
                case 'k':
                    score -= KING_VALUE;
                    break;

                default:
                    break;
            }
        }
    }
    return score;
}

int getRatingFast(char board[8][8]){
    int score = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            char piece = board[i][j];
            switch (piece) {
                case 'P':
                    score += PAWN_VALUE;
                    score += ((i) * (PAWN_POSITION_MULT + abs(j-4)));
                    if(i >= 2 && i <= 5 && j >= 2 && j <= 5) score += PAWN_CENTER_BONUS_GOOD;
                    if(i >= 3 && i <= 4 && j >= 3 && j <= 4) score += PAWN_CENTER_BONUS_GREAT;
                    break;
                case 'p':
                    score -= ((7-i) * (PAWN_POSITION_MULT + abs(j-4)));
                    score -= PAWN_VALUE;
                    if(i >= 2 && i <= 5 && i >= 2 && i <= 5) score -= PAWN_CENTER_BONUS_GOOD;
                    if(i >= 3 && i <= 4 && j >= 3 && j <= 4) score += PAWN_CENTER_BONUS_GREAT;
                    break;

                case 'B': 
                    score += BISHOP_VALUE;
                    if(isBishopOpen(board, i, j)) score += BISHOP_OPEN_BONUS;
                    if(i == 0 && (j == 2 || j == 5)) score -= BISHOP_UNDEVELOP_PUNISH;
                    break;
                case 'b':
                    score -= BISHOP_VALUE;
                    if(isBishopOpen(board, i, j)) score -= BISHOP_OPEN_BONUS;
                    if(i == 8 && (j == 2 || j == 5)) score += BISHOP_UNDEVELOP_PUNISH;
                    break;

                case 'N':
                    score += KNIGHT_VALUE;
                    if(i == 7 || j == 7 || i == 0 || j == 0) score -= KNIGHT_EDGE_PUNISH_HARD;
                    if(i == 6 || j == 6 || i == 1 || j == 1) score -= KNIGHT_EDGE_PUNISH_SOFT;
                    if(canKnightMove(board, i, j)) score += KNIGHT_MOVE_BONUS;
                    break;
                case 'n':
                    score -= KNIGHT_VALUE;
                    if(i == 7 || j == 7 || i == 0 || j == 0) score += KNIGHT_EDGE_PUNISH_HARD;
                    if(i == 6 || j == 6 || i == 1 || j == 1) score += KNIGHT_EDGE_PUNISH_SOFT;
                    if(canKnightMove(board, i, j)) score -= KNIGHT_MOVE_BONUS;
                    break;

                case 'R': 
                    score += ROOK_VALUE;
                    if(isRookOpen(board, i, j)) score += ROOK_OPEN_BONUS;
                    break;
                case 'r':
                    score -= ROOK_VALUE;
                    if(isRookOpen(board, i, j)) score -= ROOK_OPEN_BONUS;
                    break;

                case 'Q':
                    score += QUEEN_VALUE;
                    if(canQueenMove(board, i, j)) score += QUEEN_MOVE_BONUS;
                    break;
                case 'q':
                    score -= QUEEN_VALUE;
                    if(canQueenMove(board, i, j)) score -= QUEEN_MOVE_BONUS;
                    break;

                case 'K': 
                    score += KING_VALUE;
                    break;
                case 'k':
                    score -= KING_VALUE;
                    break;

                default:
                    break;
            }
        }
    }
    return score;
}


static char isChained(char board[8][8], int i, int j){
    char pawn = board[i][j];
    if(i-1 >= 0 && j-1 >= 0) return (board[i-1][j-1] == pawn);
    if(i-1 >= 0 && j+1 <  8) return (board[i-1][j+1] == pawn);
    if(i+1 <  8 && j-1 >= 0) return (board[i+1][j-1] == pawn);
    if(i+1 <  8 && j+1 <  8) return (board[i+1][j+1] == pawn);
    return 0;
}

//TODO: Fix to make sure it is actually
static char isBishopOpen(char board[8][8], int x, int y){
    int directions[4][2] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}}; 

    for (int i = 0; i < 4; i++) {
        int new_x = x + directions[i][1];
        int new_y  = y + directions[i][0];
        if(new_x >= 0 && new_y>= 0 && new_y < 8 && new_x < 8 && board[new_x][new_y] == ' ') return 1;
    }
    return 0;
}

//TODO: Fix to make sure it is actually
static char isRookOpen(char board[8][8], int x, int y){
    int directions[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}}; 

    for (int i = 0; i < 4; i++) {
        int new_x = x + directions[i][1];
        int new_y = y + directions[i][0];
        if(new_x >= 0 && new_y>= 0 && new_y < 8 && new_x < 8 && board[new_x][new_y] == ' ') return 1;
    }
    return 0;
}

/*
* See if a piece is protected, replace it with a king of the opposite color, replace the opposite colors king
* with a pawn of the current color, see if the newly placed king is in check, if so it is protected.
*/
static char isProtected(char board[8][8], int x, int y){
    char result;
    int kingRow;
    int kingCol;
    char tempPiece = board[x][y];
    char color = 'B'; // Color of us
    if(isupper(board[x][y])) color = 'W';

    //Get pos of other color's king
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((color == 'B' && board[row][col] == 'K') || (color == 'W' && board[row][col] == 'k')) {
                kingRow = row;
                kingCol = col;
            }
        }
    }

    //Set it to our color pawn
    board[kingRow][kingCol] = 'P';
    if(color == 'B') board[kingRow][kingCol] =  'p';

    //Set their king to checking piece
    board[x][y] = 'k';
    if(color == 'B') board[x][y] =  'K';


    if(color == 'W'){
        result = inCheck(board, 'B');
    }
    else{
        result =inCheck(board, 'W');
    }

    board[x][y] = tempPiece;
    board[kingRow][kingCol] = 'k';
    if(color == 'B') board[kingRow][kingCol] = 'K';

    
    return result;
}

static char canKnightMove(char board[8][8], int x, int y){

    int moves[8][2] = {{-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}};

    for (int i = 0; i < 8; i++) {
        int new_x = x + moves[i][1];
        int new_y = y + moves[i][0];
        if(new_x >= 0 && new_y>= 0 && new_y < 8 && new_x < 8 && board[new_x][new_y] == ' ') return 1;
    }
    return 0;
}

static char canQueenMove(char board[8][8], int x, int y){
    return (isRookOpen(board, x, y) || isBishopOpen(board, x, y));
}