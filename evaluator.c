#include "board.h"
#include "evaluator.h"

int getRating(char board[8][8]){
    int score = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            switch (board[i][j]) {
                case 'P':
                    score += 1;
                    break;
                case 'p':
                    score -= 1;
                    break;
                case 'B': 
                    score += 3;
                    break;
                case 'b':
                    score -= 3;
                    break;
                case 'N': 
                    score += 3;
                    break;
                case 'n':
                    score -= 3;
                    break;
                case 'R': 
                    score += 5;
                    break;
                case 'r':
                    score -= 5;
                    break;
                case 'Q': 
                    score += 10;
                    break;
                case 'q':
                    score -= 10;
                    break;
                default:
                    break;
            }
        }
    }
    return score;
}