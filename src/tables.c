#include "tree.h"
#include "tables.h"
#include <string.h>

/*
* Killer Moves
*/
static Move killerMoves[MAX_DEPTH][KMV_CNT] = {0}; // TODO: Figure out if need to change / have two max depth values
static u32 kmvIdx = 0;

void storeKillerMove(int ply, Move move){ //TODO: Make sure cant have two of same moves in killer moves 
   if(killerMoves[ply][kmvIdx] != move){
      killerMoves[ply][kmvIdx] = move;
      kmvIdx = (kmvIdx + 1) % KMV_CNT;
   }
}

Move* getKillerMoves(int ply){
    return killerMoves[ply];
}

void clearKillerMoves(void){
   memset(killerMoves, 0, sizeof(killerMoves));
}

/*
* History Tables
*/
static u32 historyTable[PLAYER_COUNT][BOARD_SIZE][BOARD_SIZE] = {0};

void storeHistoryMove(char pos_flags, Move move, char depth){
   if(GET_FLAGS(move) & CAPTURE) return;
   historyTable[pos_flags & WHITE_TURN][GET_FROM(move)][GET_TO(move)] += 1 << depth;
}

u32 getHistoryScore(char pos_flags, Move move){
   if(GET_FLAGS(move) & CAPTURE) return 0;
   return historyTable[pos_flags & WHITE_TURN][GET_FROM(move)][GET_TO(move)];
}
