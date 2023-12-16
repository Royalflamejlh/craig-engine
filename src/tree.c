#include "tree.h"
#include <stdio.h>
#include <limits.h>
#include "movement.h"
#include "util.h"
#include "evaluator.h"

Move getBestMove(Position pos){
   int size;
   Move moveList[MAX_MOVES];
   int moveScores[MAX_MOVES];
   int turn = pos.flags & WHITE_TURN;
   
   Position prevPos = pos;
   size = generateLegalMoves(pos, moveList);
   if(size == 0) return 0;

   for(int j = 0; j < 3; j += 1){
      for(int i = 0; i < size; i++)  {
         makeMove(&pos, moveList[i]);
         moveScores[i] = pvSearch(&pos, -10000, 10000, j);
         pos = prevPos;
      }
   }

   int best_move_val = turn ? INT_MIN : INT_MAX;
   int best_move_idx = 0;

   for (int i = 0; i < size; i++)  {
      if(turn ? moveScores[i] > best_move_val : moveScores[i] < best_move_val){
         best_move_val = moveScores[i];
         best_move_idx = i;
      }
   }

   return moveList[best_move_idx];
}

int pvSearch( Position* pos, int alpha, int beta, int depth ) {
   if( depth == 0 ) return quiesce(pos, alpha, beta);
   char bSearchPv = 1;
   int size;
   Move moveList[MAX_MOVES];
   size = generateLegalMoves(*pos, moveList);
   for (int i = 0; i < size; i++)  {
      Position prevPos = *pos;
      makeMove(pos, moveList[i]);
      int score;
      if ( bSearchPv ) {
         score = -pvSearch(pos, -beta, -alpha, depth - 1);
      } else {
         score = -zwSearch(pos, -alpha, depth - 1);
         if ( score > alpha ) // in fail-soft ... && score < beta ) is common
            score = -pvSearch(pos, -beta, -alpha, depth - 1); // re-search
      }
      *pos = prevPos;
      //unmakeMove(currentPosition)
      if( score >= beta )
         return beta;   // fail-hard beta-cutoff
      if( score > alpha ) {
         alpha = score; // alpha acts like max in MiniMax
         bSearchPv = 0;   // it is recommend to set bSearchPv outside the score > alpha condition.
      }
   }
   return alpha;
}

// fail-hard zero window search, returns either beta-1 or beta
int zwSearch( Position* pos, int beta, int depth ) {
   // alpha == beta - 1
   // this is either a cut- or all-node
   if( depth == 0 ) return quiesce(pos, beta-1, beta);

   int size;
   Move moveList[MAX_MOVES];
   size = generateLegalMoves(*pos, moveList);
   for (int i = 0; i < size; i++)  {
     Position prevPos = *pos;
     makeMove(pos, moveList[i]);
     int score = -zwSearch(pos, 1-beta, depth - 1);
     *pos = prevPos;
     //unmakeMove(pos)
     if( score >= beta )
        return beta;   // fail-hard beta-cutoff
   }
   return beta-1; // fail-hard, return alpha
}

//quisce search
int quiesce( Position* pos, int alpha, int beta ) {
    int stand_pat = evaluate(*pos);
    if( stand_pat >= beta )
        return beta;
    if( alpha < stand_pat )
        alpha = stand_pat;

    int size;
    Move moveList[MAX_MOVES];
    size = generateLegalMoves(*pos, moveList);
    for(int i = 0; i < size; i++)  {
        if(!(GET_FLAGS(moveList[i]) & CAPTURE)) continue;
        Position prevPos = *pos;
        makeMove(pos, moveList[i]);
        int score = -quiesce(pos,  -beta, -alpha );
        *pos = prevPos;
        //unmakeMove(pos)
        if( score >= beta )
            return beta;
        if( score > alpha )
           alpha = score;
    }
    return alpha;
}
