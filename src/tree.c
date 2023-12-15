#include "tree.h"
#include "movement.h"
#include "evaluator.h"

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
