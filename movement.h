#ifndef MOVEMENT_H
#define MOVEMENT_H
#include "tree.h"
void buildLegalMoves(struct Node *node);
char inCheck(char board[8][8], char color);
#endif