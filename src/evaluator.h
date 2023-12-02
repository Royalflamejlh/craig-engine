#ifndef EVAL_H
#define EVAL_H
#include "util.h"
#include "tree.h"
#include "types.h"
void updateRating(size_t node);
void updateRatingFast(size_t node);
int getBoardRating(char board[8][8]);
int getBoardRatingFast(char board[8][8]);

#endif
