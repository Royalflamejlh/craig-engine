#include "tree.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "movement.h"
#include "util.h"
#include "evaluator.h"
#include "transposition.h"
#include "globals.h"


#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#define DEPTH 8
#define ID_STEP 1

#define CHECKMATE_VALUE (INT_MIN + 10)

#define KMV_CNT 4 //How many killer moves are stored for a pos

#define MAX_QUIESCE_PLY 4 //How far q search can go
#define MAX_PLY 255 //How far the total search can go

#define LMR_DEPTH 3 //The min depth to start performing lmr
#define LMR_MIN_MOVE 2 //the move number to start performing lmr on

//static void selectSort(int i, Move *moveList, int *moveVals, int size);


#ifdef DEBUG

#if defined(__unix__) || defined(__APPLE__)
#define DEBUG_TIME
#include <time.h>
#include "pthread.h"
static struct timespec start_time, end_time;
#endif

static int64_t pvs_count;
static int64_t zws_count;
static int64_t q_count;

void startTreeDebug(void){
   pvs_count = 0;
   zws_count = 0;
   q_count = 0;
   #ifdef DEBUG_TIME
   clock_gettime(CLOCK_MONOTONIC, &start_time);
   #endif
}
void printTreeDebug(void){
   #ifdef DEBUG_TIME
   clock_gettime(CLOCK_MONOTONIC, &end_time);
   double elap_time = (end_time.tv_sec - start_time.tv_sec) +
                     (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
   #endif

   uint64_t total_count = pvs_count + zws_count + q_count;

   printf("Tree searched %" PRIu64 " evals (pvs: %" PRIu64 ", zws: %" PRIu64 ", q: %" PRIu64 ")\n", total_count, pvs_count, zws_count, q_count);
   #ifdef DEBUG_TIME
   printf("Took %lf seconds, with %lf eval/sec\n", elap_time, ( (double)total_count) / elap_time);
   #endif
}
#endif


/*
* Function pertaining to storage of killer moves 
*/
static Move killerMoves[MAX_PLY][KMV_CNT] = {0};
static unsigned int kmvIdx = 0;

static inline void storeKillerMove(int ply, Move move){
   if(killerMoves[ply][kmvIdx] != move){
      killerMoves[ply][kmvIdx] = move;
      kmvIdx = (kmvIdx + 1) % KMV_CNT;
   }
}

static inline void clearKillerMoves(void){
   memset(killerMoves, 0, sizeof(killerMoves));
}

/*
* Functions pertaining to storage in the history table
*/
static uint32_t historyTable[PLAYER_COUNT][BOARD_SIZE][BOARD_SIZE] = {0};

static inline void storeHistoryMove(char pos_flags, Move move, char depth){
   if(GET_FLAGS(move) & CAPTURE) return;
   historyTable[pos_flags & WHITE_TURN][GET_FROM(move)][GET_TO(move)] += 1 << depth;
}

uint32_t getHistoryScore(char pos_flags, Move move){
   if(GET_FLAGS(move) & CAPTURE) return 0;
   return historyTable[pos_flags & WHITE_TURN][GET_FROM(move)][GET_TO(move)];
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
   int i = 1;
   while(run_get_best_move){
      i+=ID_STEP;
      moveScore = -pvSearch(&pos, INT_MIN+1, INT_MAX, i, 0, &bestMove);
      //printf("Move found with score %d at depth %d\n", moveScore, i);
      //printMove(bestMove);
      global_best_move = bestMove;
   }

   #ifdef DEBUG
   printTreeDebug();
   printTTDebug();
   #endif


   //printf("Move found with score %d\n", moveScore);
   printMove(bestMove);

   return bestMove;
}


int pvSearch( Position* pos, int alpha, int beta, char depth, char ply, Move* returnMove) {
   #if defined(__unix__) || defined(__APPLE__)
   if(!run_get_best_move) pthread_exit(NULL);
   #elif defined(_WIN32) || defined(_WIN64)
   if(!run_get_best_move) ExitThread(0);
   #endif
   //printf("pvSearch (%llu) a: %d, b: %d, d: %d, ply: %d, retMove %p\n", pos->hash, alpha, beta, (int)depth, (int)ply, returnMove);
   if( depth <= 0 ) {
      int q_eval = quiesce(pos, alpha, beta, ply, 0);
      //printf("Returning q_eval: %d\n", q_eval);
      return q_eval;
   }
   #ifdef DEBUG
   pvs_count++;
   #endif

   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);
   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE;
      else return 0;
   }
   if(pos->halfmove_clock >= 50) return 0;


   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      ttMove = ttEntry->move;
      if(ttEntry->depth >= depth){
         switch (ttEntry->nodeType) {
            case PV_NODE: // Exact value
               if(returnMove) *returnMove = ttEntry->move;
               //printf("Returning PV_NODE: %d\n", ttEntry->eval);
               return ttEntry->eval;
            case CUT_NODE: // Lower bound
               if(returnMove) *returnMove = ttEntry->move;
               if (ttEntry->eval >= beta){
                  //printf("Returning CUT_NODE beta: %d\n", beta);
                  return beta;
               }
               break;
            case ALL_NODE: // Upper bound
               if (ttEntry->eval > alpha) alpha = ttEntry->eval;
               break;
            default:
               break;
         }
      }
   }

   char bSearchPv = 1;
   
   evalMoves(moveList, moveVals, size, ttMove, killerMoves[(int)ply], KMV_CNT, *pos);

   //Set up late move reduction rules
   char LMR = TRUE;
   if(pos->flags & IN_CHECK) LMR = FALSE;
   if(depth < LMR_DEPTH) LMR = FALSE;

   Position prevPos = *pos;
   Move bestMove = NO_MOVE;
   for (int i = 0; i < size; i++)  {
      selectSort(i, moveList, moveVals, size);
      makeMove(pos, moveList[i]);

      //Update current late move reduction rules
      char LMR_cur = LMR;
      if(pos->flags & IN_CHECK) LMR_cur = FALSE;
      if(i < LMR_MIN_MOVE) LMR_cur = FALSE;


      int score;
      if ( bSearchPv ) {
         score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, NULL);
      } else {
         if(LMR_cur) score = -zwSearch(pos, -alpha, LMR_DEPTH, ply + 1);
         else score = -zwSearch(pos, -alpha, depth - 1, ply + 1);
         if ( score > alpha && score < beta)
            score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, NULL);
      }
      *pos = prevPos; //Unmake move
      if( score >= beta ) {
         storeTTEntry(pos->hash, depth, beta, CUT_NODE, moveList[i]);
         storeKillerMove(ply, moveList[i]);
         storeHistoryMove(pos->flags, moveList[i], depth);
         //printf("Returning beta cutoff: %d >= %d\n", score, beta);
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
   //printf("Returning alpha: %d\n", alpha);
   return alpha;
}

// fail-hard zero window search, returns either beta-1 or beta
int zwSearch( Position* pos, int beta, char depth, char ply ) {
   #if defined(__unix__) || defined(__APPLE__)
   if(!run_get_best_move) pthread_exit(NULL);
   #elif defined(_WIN32) || defined(_WIN64)
   if(!run_get_best_move) ExitThread(0);
   #endif
   // alpha == beta - 1
   // this is either a cut- or all-node
   if( depth <= 0 ) return quiesce(pos, beta-1, beta, ply, 0);
   #ifdef DEBUG
   zws_count++;
   #endif

   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);
   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE;
      else return 0;
   }
   if(pos->halfmove_clock >= 50) return 0;

   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      ttMove = ttEntry->move;
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
            default:
               break;
         }
      }
   }


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
        storeHistoryMove(pos->flags, moveList[i], depth);
        return beta;   // fail-hard beta-cutoff
     }
   }
   return beta-1; // fail-hard, return alpha
}

//quisce search
int quiesce( Position* pos, int alpha, int beta, char ply, char q_ply) {
   #if defined(__unix__) || defined(__APPLE__)
   if(!run_get_best_move) pthread_exit(NULL);
   #elif defined(_WIN32) || defined(_WIN64)
   if(!run_get_best_move) ExitThread(0);
   #endif
   #ifdef DEBUG
   q_count++;
   #endif
   
   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);

   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE;
      else return 0;
   }
   if(pos->halfmove_clock >= 50) return 0;

   int stand_pat = evaluate(*pos);
   if( stand_pat >= beta )
      return beta;
   if( alpha < stand_pat )
      alpha = stand_pat;
   if(q_ply >= MAX_QUIESCE_PLY) return alpha;

   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      ttMove = ttEntry->move;
      switch (ttEntry->nodeType) {
         case Q_NODE:
            return ttEntry->eval;
            break;
         case CUT_NODE:
            if (ttEntry->eval >= beta) return beta;
            break;
         case ALL_NODE: // Upper bound
            if (ttEntry->eval > alpha) alpha = ttEntry->eval;
            break;
         default:
            break;
      }
   }

   evalMoves(moveList, moveVals, size, ttMove, killerMoves[(int)ply], KMV_CNT, *pos);
   
   Move bestMove = NO_MOVE;
   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
      selectSort(i, moveList, moveVals, size);
      if(!(GET_FLAGS(moveList[i]) & CAPTURE)) continue;
      makeMove(pos, moveList[i]);
      int score = -quiesce(pos,  -beta, -alpha, ply + 1, q_ply + 1);
      *pos = prevPos;

      if( score >= beta ){
         storeKillerMove(ply, moveList[i]);
         storeTTEntry(pos->hash, 0, beta, CUT_NODE, moveList[i]);
         return beta;
      }
      if( score > alpha ){
         alpha = score;
         bestMove = moveList[i];
      }
   }
   storeTTEntry(pos->hash, 0, beta, Q_NODE, bestMove);
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

