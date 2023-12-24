#include "tree.h"
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "movement.h"
#include "types.h"
#include "util.h"
#include "evaluator.h"
#include "transposition.h"
#include "globals.h"


#if defined(__unix__) || defined(__APPLE__)
#include "pthread.h"
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#ifdef __COMPILE_DEBUG
#define DEBUG
#endif


#define ID_STEP 1 //Changing this may break Aspiration windows (it will)

#define CHECKMATE_VALUE -MAX_EVAL + 1000

#define KMV_CNT 3 //How many killer moves are stored for a pos 


#ifdef __FAST_AS_POOP

#define MAX_QUIESCE_PLY 4
#define MAX_PLY 255 //How far the total search can go

#define LMR_DEPTH 2

#define MAX_ASP_START 100
#define ASP_EDGE 1

#define FUTIL_DEPTH 1 //Depth to start futility pruning
#define FUTIL_MARGIN 100 //Score difference for a node to be futility pruned

#define RAZOR_DEPTH 3 //Depth to start razoring
#define RAZOR_MARGIN PAWN_VALUE*2 //Margin to start razoring

#define NULL_PRUNE_R 3 //How much Null prunin' takes off

#else

#define MAX_QUIESCE_PLY 5 //How far q search can go 
#define MAX_PLY 255 //How far the total search can go

#define LMR_DEPTH 3 //LMR not performed if depth < LMR_DEPTH

#define MAX_ASP_START PAWN_VALUE-100 //Maximum size of bounds for an aspiration window to start on
#define ASP_EDGE 10  //Buffer size of aspiration window (fast: 1)

#define FUTIL_DEPTH 1 //Depth to start futility pruning
#define FUTIL_MARGIN PAWN_VALUE-100 //Score difference for a node to be futility pruned

#define RAZOR_DEPTH 3 //Depth to start razoring
#define RAZOR_MARGIN PAWN_VALUE*3 //Margin to start razoring

#define NULL_PRUNE_R 2 //How much Null prunin' takes off

#endif

//Set up the Depth sizes depending on mode
#ifdef DEBUG
#define MAX_DEPTH 12
#elif defined(__PROFILE)
#define MAX_DEPTH 6
#endif

//static void selectSort(int i, Move *moveList, int *moveVals, int size);

typedef enum searchs{
   PVS,
   ZWS,
   QS,
   SEARCH_TYPE_COUNT
} SearchType;

#ifdef DEBUG
#include <assert.h>
#if defined(__unix__) || defined(__APPLE__)
#define DEBUG_TIME
#include <time.h>
static struct timespec start_time, end_time;
#endif


typedef enum stats{
   NODE_COUNT,
   NODE_LOOP_CHILDREN,

   NODE_TT_HIT,
   NODE_TT_PVS_RET,
   NODE_TT_BETA_RET,
   NODE_TT_ALPHA_RET,
   
   NODE_PRUNED_NULL,
   NODE_PRUNED_RAZOR,
   NODE_PRUNED_FUTIL,
   NODE_LMR_REDUCTIONS,


   NODE_BETA_CUT,
   NODE_ALPHA_RET,
   DEBUG_STATS_COUNT
} DebugStats;

static int64_t debug[SEARCH_TYPE_COUNT][DEBUG_STATS_COUNT] = {0};

void startTreeDebug(void){
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

   int64_t total_count = 0;
   int64_t pvs_count = 0, zws_count = 0, q_count = 0;

   for (int i = 0; i < SEARCH_TYPE_COUNT; i++) {
      int64_t node_count = debug[i][NODE_COUNT];
      total_count += node_count;
      char typestr[5];
      if (i == PVS){
         pvs_count = node_count;
         strcpy(typestr, "PVS");
      }
      else if (i == ZWS){
         zws_count = node_count;
         strcpy(typestr, "ZWS");
      }
      else if (i == QS){
         q_count = node_count;
         strcpy(typestr, "Q");
      }
      printf("%s: Called: %" PRIu64 ", Entered Move Loop: %" PRIu64 ", Beta Cuts: %" PRIu64 ", Alpha Returns: %" PRIu64 "\n",
            typestr, debug[i][NODE_COUNT], debug[i][NODE_LOOP_CHILDREN], debug[i][NODE_BETA_CUT], debug[i][NODE_ALPHA_RET]);
      printf("     Null Prunes: %" PRIu64 ", Razor Prunes: %" PRIu64 ", Futil Prunes: %" PRIu64 ", LMR: %" PRIu64 " \n",
            debug[i][NODE_PRUNED_NULL], debug[i][NODE_PRUNED_RAZOR], debug[i][NODE_PRUNED_FUTIL], debug[i][NODE_LMR_REDUCTIONS]);
      printf("     TT Hits: %" PRIu64 ", TT PVS Returns: %" PRIu64 ", TT Beta Returns: %" PRIu64 ", TT Alpha Returns: %" PRIu64 " \n\n",
            debug[i][NODE_TT_HIT], debug[i][NODE_TT_PVS_RET], debug[i][NODE_TT_BETA_RET], debug[i][NODE_TT_ALPHA_RET]);
   }

   printf("\nTree searched %" PRIu64 " evals (pvs: %" PRIu64 ", zws: %" PRIu64 ", q: %" PRIu64 ")\n", total_count, pvs_count, zws_count, q_count);

   #ifdef DEBUG_TIME
   printf("\nTook %lf seconds, with %lf eval/sec\n", elap_time, ((double)total_count) / elap_time);
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

//exit thread function
static void exit_search(Move* pvArray){
   free(pvArray);
   #if defined(__unix__) || defined(__APPLE__)
   pthread_exit(NULL);
   #elif defined(_WIN32) || defined(_WIN64)
   ExitThread(0);
   #endif
}

/*
* Get that move son.
*/
int getBestMove(Position pos){

   clearKillerMoves(); //TODO: make thread safe!
   int i = 1;
   int eval = 0, eval_prev = 0, asp_upper = 0, asp_lower = 0;
   while(run_get_best_move 
         #ifdef MAX_DEPTH
         && i <= MAX_DEPTH
         #endif
         ){
      #ifdef DEBUG
      startTreeDebug();
      #endif
      #ifdef TT_DEBUG
      startTTDebug();
      #endif

      Move *pvArray = malloc(sizeof(Move) * (i*i + i)/2);
      memset(pvArray, 0, sizeof(Move) * (i*i + i)/2);
      if(!pvArray){
         printf("info Warning: failed to allocate space for pvArray");
         return -1;
      }

      //printf("Running pv search at depth %d\n", i);
      if(i <= 2){
         eval = pvSearch(&pos, INT_MIN+1, INT_MAX, i, 0, pvArray, 0);
      } else {
         //Calculate the Aspiration Window
         int asp_dif = (abs(eval_prev - eval) / 2) + ASP_EDGE;
         asp_upper = asp_lower = MIN(asp_dif, MAX_ASP_START);
         int q = (eval_prev + eval + eval + eval) / 4; //Weighted towards the most recent eval

         int eval_tmp = pvSearch(&pos, q-asp_lower, q+asp_upper, i, 0, pvArray, 0);
         while(eval_tmp <= q-asp_lower || eval_tmp >= q+asp_upper || pvArray[0] == NO_MOVE){
            if(eval_tmp <= q-asp_lower){ asp_lower *= 2;}
            if(eval_tmp >= q+asp_upper){ asp_upper *= 2;}
            if(pvArray[0] == NO_MOVE){
               asp_upper *= 2;
               asp_lower *= 2;
            }
            #ifdef DEBUG
            printf("Running again with window: %d, %d\n", q-asp_lower, q+asp_upper);
            #endif
            eval_tmp = pvSearch(&pos, q-asp_lower, q+asp_upper, i, 0, pvArray, 0);
         }
         eval_prev = eval;
         eval = eval_tmp;
      }
      
      #ifdef DEBUG
      printPV(pvArray, i);
      printf("found with score %d\n", eval);
      #endif
      #ifdef DEBUG
      printTreeDebug();
      #ifdef TT_DEBUG
      printTTDebug();
      #endif
      printf("\n-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
      #endif

      global_best_move = pvArray[0];
      free(pvArray);
      i+=ID_STEP;
   }

   

   #ifdef MAX_DEPTH
   return 1;
   #endif

   return 0;
}

/*
* Pruning Methods
*/

//Null Move Search
static int pruneNullMoves(Position* pos, int beta, int depth, int ply, Move* pvArray){
   if(!(pos->flags & IN_CHECK) && pos->stage != END_GAME){
      Position prevPos = *pos;
      makeNullMove(pos);
      int score = -zwSearch(pos, 1-beta, depth - NULL_PRUNE_R - 1, ply + 1, pvArray);
      if( score >= beta ){
         return beta;
      }
      *pos = prevPos;
   }
   return beta-1;
}

//Late move reduction
static char getSearchDepth(char curDepth, int moveNum, char flags, int allowLMR, char searchType){
   if(flags & IN_CHECK) return curDepth - 1;
   if(curDepth < LMR_DEPTH) return curDepth - 1;
   if(!allowLMR) return curDepth - 1;
   if(searchType == PVS){
      if(moveNum >= 3) return curDepth - (moveNum / 2);
   }
   else{
      if(moveNum >= 2) return curDepth - moveNum;
   }
   return curDepth - 1;
}

/*
*
*  PRINCIPAL VARIATION SEARCH
*
*/

int pvSearch( Position* pos, int alpha, int beta, char depth, char ply, Move* pvArray, int pvIndex) {
   //printf("Depth = %d, Ply = %d, Depth+ply = %d\n", depth, ply, depth+ply);
   if(!run_get_best_move) exit_search(pvArray);

   if( depth <= 0 ) {
      int q_eval = quiesce(pos, alpha, beta, ply, 0, pvArray);
      //printf("Returning q_eval: %d\n", q_eval);
      return q_eval;
   }
   #ifdef DEBUG
   debug[PVS][NODE_COUNT]++;
   #endif

   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);
   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE - ply;
      else return 0;
   }
   if(pos->halfmove_clock >= 50) return 0;

   char bSearchPv = 1;  //Flag for if we are 
   int pvNextIndex = pvIndex + depth;

   //Test the TT table
   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      #ifdef DEBUG
      debug[PVS][NODE_TT_HIT]++;
      #endif
      ttMove = ttEntry->move;
      if(ttEntry->depth >= depth){
         switch (ttEntry->nodeType) {
            case PV_NODE: // Exact value
               pvArray[pvIndex] = ttEntry->move;
               #ifdef DEBUG
               debug[PVS][NODE_TT_PVS_RET]++;
               #endif
               return ttEntry->eval;
            case CUT_NODE: // Lower bound
               if (ttEntry->eval >= beta){
                  #ifdef DEBUG
                  debug[PVS][NODE_TT_BETA_RET]++;
                  #endif
                  return beta;
               }
               break;
            case ALL_NODE: // Upper bound
               if (ttEntry->eval < alpha){
                  #ifdef DEBUG
                  debug[PVS][NODE_TT_ALPHA_RET]++;
                  #endif
                  return alpha;
               }
               break;
            default: //Q Node
               ttMove = NO_MOVE;
               break;
         }
      }
   }

   //Null move prunin'
   if(pruneNullMoves(pos, beta, depth, ply, pvArray) >= beta){
      #ifdef DEBUG
      debug[PVS][NODE_PRUNED_NULL]++;
      #endif
      return beta;
   }

   evalMoves(moveList, moveVals, size, ttMove, killerMoves[(int)ply], KMV_CNT, *pos);

   //Set up prunability
   char prunable = !(pos->flags & IN_CHECK);
   if(abs(beta-1) >= (-CHECKMATE_VALUE) - 2*MAX_MOVES) prunable = FALSE;

   //Set up late move reduction rules
   char LMR_allowed = TRUE;
   if(pos->flags & IN_CHECK) LMR_allowed = FALSE;

   #ifdef DEBUG
   if(size > 0) debug[PVS][NODE_LOOP_CHILDREN]++;
   #endif

   Move bestMove = NO_MOVE;
   int bestScore = INT_MIN;
   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      selectSort(i, moveList, moveVals, size);
      makeMove(pos, moveList[i]);

      //Update prunability
      if(pos->flags & IN_CHECK) prunable = 0;
      if(GET_FLAGS(moveList[i]) & CAPTURE) prunable = 0;

      if( prunable   && 
          depth == 1 && 
          quickEval(*pos) + moveVals[i] < beta-1 + FUTIL_MARGIN){ //Futility Pruning (Just prune the nodes that are expect to be poop)
         #ifdef DEBUG
         debug[PVS][NODE_PRUNED_FUTIL]++;
         #endif
         goto next_pvs_move;
      }

      int score;
      if ( bSearchPv ) {
         score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, pvNextIndex);
      } else {
         int search_depth = getSearchDepth(depth, i, pos->flags, LMR_allowed, PVS);
         #ifdef DEBUG
         debug[PVS][NODE_LMR_REDUCTIONS] += MAX(((depth - 1) - search_depth), 0);
         #endif
         score = -zwSearch(pos, -alpha, search_depth, ply + 1, pvArray);
         if ( score > alpha && score < beta){
            score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, pvNextIndex);
         }
      }
      if( score >= beta ) { //Beta cutoff
         storeTTEntry(prevPos.hash, depth, score, CUT_NODE, moveList[i]);
         storeKillerMove(ply, moveList[i]);
         storeHistoryMove(prevPos.flags, moveList[i], depth);
         //printf("Returning beta cutoff: %d >= %d\n", score, beta);
         #ifdef DEBUG
         debug[PVS][NODE_BETA_CUT]++;
         #endif
         return beta;
      }
      if( score > alpha ) {  //Improved alpha
         alpha = score;
         pvArray[pvIndex] = moveList[i];
         movcpy (pvArray + pvIndex + 1, pvArray + pvNextIndex, depth - 1);
         bSearchPv = 0; 
      }
      if( score > bestScore ){ //Improved best move
         bestMove = moveList[i];
         bestScore = moveVals[i];
      }
next_pvs_move:
      *pos = prevPos;
   }
   if (!bSearchPv) {
      // PV Node (exact value)
      storeTTEntry(pos->hash, depth, alpha, PV_NODE, pvArray[pvIndex]);
   } else {
      // ALL Node (upper bound)
      storeTTEntry(pos->hash, depth, bestScore, ALL_NODE, bestMove);
   }
   #ifdef DEBUG
   debug[PVS][NODE_ALPHA_RET]++;
   #endif
   return alpha;
}



/*
*
*  ZERO WINDOW SEARCH
*
*/
int zwSearch( Position* pos, int beta, char depth, char ply, Move* pvArray ) {
   if(!run_get_best_move) exit_search(pvArray);
   // alpha == beta - 1
   // this is either a cut- or all-node
   if( depth <= 0 ) return quiesce(pos, beta-1, beta, ply, 0, pvArray);
   #ifdef DEBUG
   debug[ZWS][NODE_COUNT]++;
   #endif

   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);
   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE + ply;
      else return 0;
   }
   if(pos->halfmove_clock >= 50) return 0;

   TTEntry* ttEntry = getTTEntry(pos->hash);
   Move ttMove = NO_MOVE;
   if (ttEntry) {
      #ifdef DEBUG
      debug[ZWS][NODE_TT_HIT]++;
      #endif
      ttMove = ttEntry->move;
      if(ttEntry->depth >= depth){
         switch (ttEntry->nodeType) {
            case PV_NODE: // Exact value
               #ifdef DEBUG
               debug[ZWS][NODE_TT_PVS_RET]++;
               #endif
               return ttEntry->eval;
               break;
            case CUT_NODE: // Lower bound
               if (ttEntry->eval >= beta){
                  #ifdef DEBUG
                  debug[ZWS][NODE_TT_BETA_RET]++;
                  #endif
                  return beta;
               }
               break;
            case ALL_NODE:
               if (ttEntry->eval < beta-1){
                  #ifdef DEBUG
                  debug[ZWS][NODE_TT_ALPHA_RET]++;
                  #endif
                  return beta-1;
               }
               break;
            default:
               ttMove = NO_MOVE;
               break;
         }
      }
   }

   //Null move prunin'
   if(pruneNullMoves(pos, beta, depth, ply, pvArray) >= beta){
      #ifdef DEBUG
      debug[ZWS][NODE_PRUNED_NULL]++;
      #endif
      return beta;
   }

   evalMoves(moveList, moveVals, size, ttMove, killerMoves[(int)ply], KMV_CNT, *pos);

   //Set up prunability
   char prunable = !(pos->flags & IN_CHECK);
   if(abs(beta-1) >= (-CHECKMATE_VALUE) - 2*MAX_MOVES) prunable = FALSE;

   //Set up late move reduction rules
   char LMR_allowed = TRUE;
   if(pos->flags & IN_CHECK) LMR_allowed = FALSE;

   #ifdef DEBUG
   if(size > 0) debug[ZWS][NODE_LOOP_CHILDREN]++;
   #endif

   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      selectSort(i, moveList, moveVals, size);
      makeMove(pos, moveList[i]);

      //Update prunability
      if(pos->flags & IN_CHECK) prunable = 0;
      if(GET_FLAGS(moveList[i]) & CAPTURE) prunable = 0;

      if( prunable   && 
          depth == 1 && 
          quickEval(*pos) + moveVals[i] < beta-1 + FUTIL_MARGIN){ //Futility Pruning
         #ifdef DEBUG
         debug[ZWS][NODE_PRUNED_FUTIL]++;
         #endif
         goto next_zws_move;
      }
      
      char search_depth = getSearchDepth(depth, i, pos->flags, LMR_allowed, ZWS);
      #ifdef DEBUG
      debug[ZWS][NODE_LMR_REDUCTIONS] += MAX(((depth - 1) - search_depth), 0);
      #endif
      int score = -zwSearch(pos, 1-beta, search_depth, ply + 1, pvArray);

      if( score >= beta ){ //Beta Cutoff
         storeTTEntry(prevPos.hash, depth, score, CUT_NODE, moveList[i]);
         storeKillerMove(ply, moveList[i]);
         storeHistoryMove(prevPos.flags, moveList[i], depth);
         #ifdef DEBUG
         debug[ZWS][NODE_BETA_CUT]++;
         #endif
         return beta;   // fail-hard beta-cutoff
      }

      if(prunable && (depth <= RAZOR_DEPTH) && (score + RAZOR_MARGIN < beta)){ //Razoring
         int razor_score = quiesce(pos, beta-1, beta, ply, 0, pvArray);
         if(razor_score >= beta){
            #ifdef DEBUG
            debug[ZWS][NODE_PRUNED_RAZOR]++;
            #endif
            return beta;
         }
      }

next_zws_move:
      *pos = prevPos;
   }
   return beta-1; // fail-hard, return alpha
}

//quisce search
//TODO : MAKE A MOVE GENERATOR FOR THIS AND USE IT U REGARD
int quiesce( Position* pos, int alpha, int beta, char ply, char q_ply, Move* pvArray) {
   if(!run_get_best_move) exit_search(pvArray);
   #ifdef DEBUG
   debug[QS][NODE_COUNT]++;
   #endif
   
   Move moveList[MAX_MOVES];
   int moveVals[MAX_MOVES];
   int size = generateLegalMoves(*pos, moveList);

   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE + ply;
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
      #ifdef DEBUG
      debug[QS][NODE_TT_HIT]++;
      #endif
      ttMove = ttEntry->move;
      switch (ttEntry->nodeType) {
         case Q_EXACT_NODE:
            #ifdef DEBUG
            debug[QS][NODE_TT_PVS_RET]++;
            #endif
            return ttEntry->eval;
            break;
         case CUT_NODE:
            if (ttEntry->eval >= beta){
               #ifdef DEBUG
               debug[QS][NODE_TT_BETA_RET]++;
               #endif
               return beta;
            }
            break;
         case Q_ALL_NODE:
         case ALL_NODE: // Upper bound
            if (ttEntry->eval < alpha){
               #ifdef DEBUG
               debug[QS][NODE_TT_ALPHA_RET]++;
               #endif
               return alpha;
            }
            break;
         default:
            break;
      }
   }


   evalMoves(moveList, moveVals, size, ttMove, killerMoves[(int)ply], KMV_CNT, *pos);
   
   Move bestMove = NO_MOVE;
   int bestScore = INT_MIN;
   #ifdef DEBUG
   if(size > 0) debug[QS][NODE_LOOP_CHILDREN]++;
   #endif
   int exact = 0;
   Position prevPos = *pos;
   for (int i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      selectSortQ(i, moveList, moveVals, size); 
      if(!(GET_FLAGS(moveList[i]) & CAPTURE)) continue;
      makeMove(pos, moveList[i]);
      int score = -quiesce(pos,  -beta, -alpha, ply + 1, q_ply + 1, pvArray);

      if( score >= beta ){
         storeKillerMove(ply, moveList[i]);
         //storeTTEntry(prevPos.hash, 0, score, CUT_NODE, moveList[i]);
         #ifdef DEBUG
         debug[QS][NODE_BETA_CUT]++;
         #endif
         return beta;
      }
      if( score > alpha ){
         alpha = score;
         exact = 1;
      }
      if( score > bestScore){
         bestScore = score;
         bestMove = moveList[i];
      }

      *pos = prevPos;
   }
   //Handle the case were there where no captures or checks
   if(bestScore == INT_MIN) bestScore = evaluate(*pos);

   if(exact) storeTTEntry(pos->hash, 0, alpha, Q_EXACT_NODE, bestMove);
   else      storeTTEntry(pos->hash, 0, bestScore, Q_ALL_NODE, bestMove);

   #ifdef DEBUG
   debug[QS][NODE_ALPHA_RET]++;
   #endif
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

void selectSortQ(int i, Move *moveList, int *moveVals, int size) {
    int maxIdx = i;

    for (int j = i + 1; j < size; j++) {
        if (moveVals[j] > moveVals[maxIdx] && (GET_FLAGS(moveList[j]) & CAPTURE)) {
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

