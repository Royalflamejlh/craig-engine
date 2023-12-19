#include "tree.h"
#include <stdio.h>
#include <limits.h>
#include "movement.h"
#include "util.h"
#include "evaluator.h"
#include "transposition.h"

#define DEBUG
#define MAX_DEPTH 7
#define ID_STEP 1


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
   Move bestMove;
   int moveScore;
   
   #ifdef DEBUG
   startTTDebug();
   startTreeDebug();
   #endif

   for(int i = 1; i <= MAX_DEPTH; i+=ID_STEP){
      moveScore = -pvSearch(&pos, -10000, 10000, i, &bestMove);
      printf("Move found with score %d at depth %d\n", moveScore, i);
      printMove(bestMove);
      fflush(stdout);
   }

   #ifdef DEBUG
   printTreeDebug();
   printTTDebug();
   #endif


   printf("Move found with score %d\n", moveScore);
   printMove(bestMove);

   return bestMove;
}

int pvSearch( Position* pos, int alpha, int beta, char depth, Move* returnMove ) {
   // printf("Searching at depth %d, a: %d, b: %d, pos:%llu\n", (int)depth, alpha, beta, pos->hash);
   if( depth == 0 ) {
      int q_eval = quiesce(pos, alpha, beta);
      // printf("Q search returned score: %d\n", q_eval);
      return q_eval;
   }
   #ifdef DEBUG
   pvs_count++;
   #endif

   // int pv_node_found = 0;
   // int stored_eval = 0;
   // char stored_depth = 0;
   // Move stored_move = NO_MOVE;

   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      if(ttEntry->depth >= depth){
         switch (ttEntry->nodeType) {
            case PV_NODE: // Exact value
               // pv_node_found = 1;
               // stored_eval = ttEntry->eval;
               // stored_depth = ttEntry->depth;
               // stored_move = ttEntry->move;
               if(returnMove) *returnMove = ttEntry->move;
               return ttEntry->eval;
               break;
            case CUT_NODE: // Lower bound
               if(returnMove) *returnMove = ttEntry->move;
               if (ttEntry->eval >= beta) return beta;
               break;
            case ALL_NODE: // Upper bound
               if (ttEntry->eval > alpha) alpha = ttEntry->eval;
               break;
         }
      }
      else{
         ttMove = ttEntry->move;
      }
   }

   char bSearchPv = 1;
   int size;
   
   uint64_t cur_hash = pos->hash;
   Move bestMove = NO_MOVE;
   Move moveList[MAX_MOVES + 1];
   if (ttMove != NO_MOVE) {
      moveList[0] = ttMove;
      size = generateLegalMoves(*pos, moveList + 1) + 1; // Including ttMove
   } else {
      size = generateLegalMoves(*pos, moveList);
   }
   // printf("Generated %d nodes\n", size);
   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
      makeMove(pos, moveList[i]);
      int score;
      if ( bSearchPv ) {
         score = -pvSearch(pos, -beta, -alpha, depth - 1, NULL);
      } else {
         score = -zwSearch(pos, -alpha, depth - 1);
         if ( score > alpha && score < beta) // && score < beta
            score = -pvSearch(pos, -beta, -alpha, depth - 1, NULL); // re-search
      }
      *pos = prevPos;
      //unmakeMove(currentPosition)
      if( score >= beta ) {
         //printf("Storing CUT_NODE for hash %llu with elo %d at depth %d\n", cur_hash, beta, (int)depth);
         storeTTEntry(cur_hash, depth, beta, CUT_NODE, moveList[i]);
         // if(pv_node_found) printf(" and it had a real (beta) value of %d\n", beta);
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
      // printf("Storing PV_NODE for hash %llu with elo %d at depth %d (move: %d)\n", pos->hash, alpha, (int)depth, bestMove);
      storeTTEntry(pos->hash, depth, alpha, PV_NODE, bestMove);
   } else {
      // ALL Node (upper bound)
      // printf("Storing ALL_NODE for hash %llu with elo %d at depth %d\n", pos->hash, alpha, (int)depth);
      storeTTEntry(pos->hash, depth, alpha, ALL_NODE, NO_MOVE);
   }
   // if(pv_node_found){
   //    printf("Found a PV_NODE (%llu) in TT with eval: %d", ttEntry->hash, stored_eval);
   //    printf(" at depth (TT:%d , R:%d)", (int)stored_depth, (int)depth);
   //    printf(" with move (TT:%d, R:%d)", stored_move, bestMove);
   //    printf(" and it had a real value of %d\n", alpha);
   // }
   if(returnMove) *returnMove = bestMove;
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

   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      if(ttEntry->depth >= depth){
         switch (ttEntry->nodeType) {
            case PV_NODE: // Exact value
               return ttEntry->eval;
               break;
            case CUT_NODE: // Lower bound
               if (ttEntry->eval >= beta) return beta;
               break;
            case ALL_NODE: // Upper bound
               if (ttEntry->eval <= beta-1) return beta-1;
               break;
         }
      }
      else{
         ttMove = ttEntry->move;
      }
   }

   int size;
   Move moveList[MAX_MOVES + 1];
   if (ttMove != NO_MOVE) {
      moveList[0] = ttMove;
      size = generateLegalMoves(*pos, moveList + 1) + 1; // Including ttMove
   } else {
      size = generateLegalMoves(*pos, moveList);
   }

   uint64_t cur_hash = pos->hash;
   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
     makeMove(pos, moveList[i]);
     int score = -zwSearch(pos, 1-beta, depth - 1);
     *pos = prevPos;
     //unmakeMove(pos)
     if( score >= beta ){
        storeTTEntry(cur_hash, depth, beta, CUT_NODE, moveList[i]);
        return beta;   // fail-hard beta-cutoff
     }
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
