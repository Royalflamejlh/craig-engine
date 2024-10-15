#include "tree.h"
#include "types.h"
#include "tables.h"
//TODO: Both of these tables need to be modified to support threading.

/*
* Killer Moves
*/

void storeKillerMove(KillerMoves* km, int ply, Move move){ 
   if(GET_FLAGS(move) > DOUBLE_PAWN_PUSH) return; // Killer moves are quiet
   for(int i = 0; i < KMV_CNT; i++){
      if(km->table[ply][km->kmvIdx] == move) return;
   }
   km->table[ply][km->kmvIdx] = move;
   km->kmvIdx = (km->kmvIdx + 1) % KMV_CNT;
}

u8 isKillerMove(KillerMoves* km, Move move, int ply){ // TODO: Just get all three killer moves and on each move iteration check if they match
   for(int i = 0; i < KMV_CNT; i++){
      if(move == km->table[ply][i]) return TRUE;
   }
   return FALSE;
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
