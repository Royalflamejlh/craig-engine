#ifndef BOARD_H
#define BOARD_H

#include <stdint.h>
#include "util.h"
void initializeBoard(char board[8][8]);
void printBoard(char board[8][8]);
void receiveMove(char board[8][8], struct Move move);

#endif