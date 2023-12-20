#ifndef tree_h
#define tree_h
#include "types.h"

uint32_t getHistoryScore(char pos_flags, Move move);

Move getBestMove(Position pos);
int pvSearch( Position* pos, int alpha, int beta, char depth, char ply, Move* move);
int zwSearch( Position* pos, int beta, char depth, char ply );
int quiesce( Position* pos, int alpha, int beta, char ply, char q_ply);
void selectSort(int i, Move *moveList, int *moveVals, int size);
#endif