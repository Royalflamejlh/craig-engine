#ifndef MOVEMENT_H
#define MOVEMENT_H
#include <stdlib.h>
#include "tree.h"
#include "util.h"
#include "types.h"

void buildLegalMoves(size_t node);
char inCheck(char board[8][8], char color);
#endif
