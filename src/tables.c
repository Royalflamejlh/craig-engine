#include "tree.h"
#include "types.h"
#include "tables.h"
#include <string.h>

//TODO: Both of these tables need to be modified to support threading.

/*
* Killer Moves
*/
static Move killerMoves[MAX_DEPTH][KMV_CNT] = {0}; // TODO: Figure out if need to change / have two max depth values
static u32 kmvIdx = 0;

void storeKillerMove(int ply, Move move){ 
   if(GET_FLAGS(move) <= DOUBLE_PAWN_PUSH) return; // Killer moves arent quiet
   for(int i = 0; i < KMV_CNT; i++){
      if(killerMoves[ply][kmvIdx] == move) return;
   }
   killerMoves[ply][kmvIdx] = move;
   kmvIdx = (kmvIdx + 1) % KMV_CNT;
}

u8 isKillerMove(Move move, int ply){
   for(int i = 0; i < KMV_CNT; i++){
      if(move == killerMoves[ply][i]) return TRUE;
   }
   return FALSE;
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
