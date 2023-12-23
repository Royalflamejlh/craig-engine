#ifndef tree_h
#define tree_h
#include "types.h"

uint32_t getHistoryScore(char pos_flags, Move move);

int getBestMove(Position pos);
int pvSearch( Position* pos, int alpha, int beta, char depth, char ply, Move* pvArray, int pvIndex);
int zwSearch( Position* pos, int beta, char depth, char ply, Move* pvArray );
int quiesce( Position* pos, int alpha, int beta, char ply, char q_ply, Move* pvArray);
void selectSort(int i, Move *moveList, int *moveVals, int size);
void selectSortQ(int i, Move *moveList, int *moveVals, int size);
#endif