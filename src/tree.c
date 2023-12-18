#include "tree.h"
#include <stdio.h>
#include <limits.h>
#include "movement.h"
#include "util.h"
#include "evaluator.h"
#include "transposition.h"

#define DEBUG
#define MAX_DEPTH 3


#ifdef DEBUG
#include <time.h>
static int64_t pvs_count;
static int64_t zws_count;
static int64_t q_count;
static struct timespec start_time, end_time;

void startTreeDebug(void){
   pvs_count = 0;
   zws_count = 0;
   q_count = 0;
   clock_gettime(CLOCK_MONOTONIC, &start_time);
}
void printTreeDebug(void){
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elap_time = (end_time.tv_sec - start_time.tv_sec) +
                       (end_time.tv_nsec - start_time.tv_nsec) / 1e9;

   uint64_t total_count = pvs_count + zws_count + q_count;

   printf("Tree searched %lld evals (pvs: %lld, zws: %lld, q: %lld)\n", total_count, pvs_count, zws_count, q_count);
   printf("Took %lf seconds, with %lf eval/sec\n", elap_time, ( (double)total_count) / elap_time);
}
#endif

Move getBestMove(Position pos){
   int size;
   Move moveList[MAX_MOVES];
   int moveScores[MAX_MOVES];
   
   Position prevPos = pos;
   size = generateLegalMoves(pos, moveList);
   if(size == 0) return 0;

   //for(int j = 0; j < 3; j += 1){
   #ifdef DEBUG
   startTTDebug();
   startTreeDebug();
   #endif
      for(int i = 0; i < size; i++)  {
         makeMove(&pos, moveList[i]);
         moveScores[i] = -pvSearch(&pos, -10000, 10000, MAX_DEPTH);
         pos = prevPos;
      }
   #ifdef DEBUG
   printTreeDebug();
   printTTDebug();
   #endif
   //}

   int best_move_val = INT_MIN;
   int best_move_idx = 0;

   for (int i = 0; i < size; i++)  {
      if(moveScores[i] > best_move_val){
         best_move_val = moveScores[i];
         best_move_idx = i;
      }
   }

   return moveList[best_move_idx];
}

int pvSearch( Position* pos, int alpha, int beta, char depth ) {
   if( depth == 0 ) return quiesce(pos, alpha, beta);
   #ifdef DEBUG
   pvs_count++;
   #endif

   TTEntry* ttEntry = getTTEntry(pos->hash);
   if (ttEntry && ttEntry->depth >= depth) {
      switch (ttEntry->nodeType) {
         case PV_NODE: // Exact value
            return ttEntry->eval;
         case CUT_NODE: // Lower bound
            if (ttEntry->eval >= beta) return ttEntry->eval;
            break;
         case ALL_NODE: // Upper bound
            if (ttEntry->eval <= alpha) return ttEntry->eval;
            break;
      }
   }

   char bSearchPv = 1;
   int size;
   
   uint64_t cur_hash = pos->hash;
   Move bestMove = NO_MOVE;
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
         if ( score > alpha) // && score < beta
            score = -pvSearch(pos, -beta, -alpha, depth - 1); // re-search
      }
      *pos = prevPos;
      //unmakeMove(currentPosition)
      if( score >= beta ) {
         //storeTTEntry(cur_hash, depth, beta, CUT_NODE, moveList[i]);
         return beta;   // fail-hard beta-cutoff
      }
      if( score > alpha ) {
         alpha = score; // alpha acts like max in MiniMax
         bestMove = moveList[i];
         bSearchPv = 0;   // it is recommend to set bSearchPv outside the score > alpha condition.
      }
   }
   if (bestMove != NO_MOVE) {
      // PV Node (exact value)
      storeTTEntry(pos->hash, depth, alpha, PV_NODE, bestMove);
   } else {
      // ALL Node (upper bound)
      //storeTTEntry(pos->hash, depth, alpha, ALL_NODE, NO_MOVE);
   }
   return alpha;
}

// fail-hard zero window search, returns either beta-1 or beta
int zwSearch( Position* pos, int beta, char depth ) {
   // alpha == beta - 1
   // this is either a cut- or all-node
   if( depth == 0 ) return quiesce(pos, beta-1, beta);
   #ifdef DEBUG
   zws_count++;
   #endif
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
    #ifdef DEBUG
    q_count++;
    #endif
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
