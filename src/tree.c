#include "tree.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "movement.h"
#include "util.h"
#include "evaluator.h"
#include "transposition.h"

#define DEBUG

#define DEPTH 7
#define ID_STEP 1

#define MAX_QUIESCE_PLY 10
#define MAX_PLY 255



//static void selectSort(int i, Move *moveList, int *moveVals, int size);


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


/*
* Function pertaining to storage of killer moves 
*/
#define KMV_CNT 10
static Move killerMoves[MAX_PLY][KMV_CNT] = {0};

static inline void storeKillerMove(int ply, Move move){
   for(int i = 0; i < KMV_CNT; i++){
      if(killerMoves[ply][i] != move){
         killerMoves[ply][i] = move;
         return;
      }
   }
}

static inline void clearKillerMoves(void){
   memset(killerMoves, 0, sizeof(killerMoves));
}

/*
* Get that move son.
*/

Move getBestMove(Position pos){
   Move bestMove;
   int moveScore;
   
   #ifdef DEBUG
   startTTDebug();
   startTreeDebug();
   #endif

   clearKillerMoves();
   for(int i = 1; i <= DEPTH; i+=ID_STEP){
      moveScore = -pvSearch(&pos, -10000, 10000, i, 0, &bestMove);
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


int pvSearch( Position* pos, int alpha, int beta, char depth, char ply, Move* returnMove) {
   if( depth == 0 ) {
      int q_eval = quiesce(pos, alpha, beta, ply + 1, 0);
      return q_eval;
   }
   #ifdef DEBUG
   pvs_count++;
   #endif


   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      if(ttEntry->depth >= depth){
         switch (ttEntry->nodeType) {
            case PV_NODE: // Exact value
               if(returnMove) *returnMove = ttEntry->move;
               return ttEntry->eval;
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
   
   //TTmove
   //KillerMoves[ply][kmove_size]
   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   size = generateLegalMoves(*pos, moveList);
   evalMoves(moveList, moveVals, size, ttMove, killerMoves[(int)ply], KMV_CNT, *pos);
   Position prevPos = *pos;
   Move bestMove;
   for (int i = 0; i < size; i++)  {
      selectSort(i, moveList, moveVals, size);
      makeMove(pos, moveList[i]);
      int score;
      if ( bSearchPv ) {
         score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, NULL);
      } else {
         score = -zwSearch(pos, -alpha, depth - 1, ply + 1);
         if ( score > alpha && score < beta)
            score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, NULL);
      }
      *pos = prevPos; //Unmake move
      if( score >= beta ) {
         storeTTEntry(pos->hash, depth, beta, CUT_NODE, moveList[i]);
         storeKillerMove(ply, moveList[i]);
         return beta;
      }
      if( score > alpha ) {
         alpha = score;
         bestMove = moveList[i];
         bSearchPv = 0; 
      }
   }
   if (bestMove != NO_MOVE) {
      // PV Node (exact value)
      storeTTEntry(pos->hash, depth, alpha, PV_NODE, bestMove);
   } else {
      // ALL Node (upper bound)
      storeTTEntry(pos->hash, depth, alpha, ALL_NODE, NO_MOVE);
   }
   if(returnMove) *returnMove = bestMove;
   return alpha;
}

// fail-hard zero window search, returns either beta-1 or beta
int zwSearch( Position* pos, int beta, char depth, char ply ) {
   // alpha == beta - 1
   // this is either a cut- or all-node
   if( depth == 0 ) return quiesce(pos, beta-1, beta, ply + 1, 0);
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

   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);
   evalMoves(moveList, moveVals, size, ttMove, killerMoves[(int)ply], KMV_CNT, *pos);

   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
     selectSort(i, moveList, moveVals, size);
     makeMove(pos, moveList[i]);
     int score = -zwSearch(pos, 1-beta, depth - 1, ply + 1);
     *pos = prevPos;
     //unmakeMove(pos)
     if( score >= beta ){
        storeTTEntry(pos->hash, depth, beta, CUT_NODE, moveList[i]);
        storeKillerMove(ply, moveList[i]);
        return beta;   // fail-hard beta-cutoff
     }
   }
   return beta-1; // fail-hard, return alpha
}

//quisce search
int quiesce( Position* pos, int alpha, int beta, char ply, char q_ply) {
    #ifdef DEBUG
    q_count++;
    #endif
    int stand_pat = evaluate(*pos);
    if( stand_pat >= beta )
        return beta;
    if( alpha < stand_pat )
        alpha = stand_pat;
   
    if(q_ply >= MAX_QUIESCE_PLY) return alpha;

   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);
   evalMoves(moveList, moveVals, size, NO_MOVE, killerMoves[(int)ply], KMV_CNT, *pos);
   
   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
      selectSort(i, moveList, moveVals, size);
      if(!(GET_FLAGS(moveList[i]) & CAPTURE)) continue;
      makeMove(pos, moveList[i]);
      int score = -quiesce(pos,  -beta, -alpha, ply + 1, q_ply + 1);
      *pos = prevPos;

      if( score >= beta ){
         storeKillerMove(ply, moveList[i]);
         return beta;
      }
      if( score > alpha ){
         alpha = score;
      }
    }
    return alpha;
}


void selectSort(int i, Move *moveList, int *moveVals, int size) {
    int maxIdx = i;

    for (int j = i + 1; j < size; j++) {
        if (moveVals[j] > moveVals[maxIdx]) {
            maxIdx = j;
        }
    }

    if (maxIdx != i) {
        int tempVal = moveVals[i];
        moveVals[i] = moveVals[maxIdx];
        moveVals[maxIdx] = tempVal;

        Move tempMove = moveList[i];
        moveList[i] = moveList[maxIdx];
        moveList[maxIdx] = tempMove;
    }
}

