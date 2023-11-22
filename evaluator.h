#ifndef EVAL_H
#define EVAL_H
#include "util.h"
#include "tree.h"
void updateRating(struct Node* node);
void updateRatingFast(struct Node* node);
int getBoardRating(char board[8][8]);
int getBoardRatingFast(char board[8][8]);

#endif