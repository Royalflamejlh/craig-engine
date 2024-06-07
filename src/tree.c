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

#define ID_STEP 1 //Changing this may break Aspiration windows (it will)

#define PV_PRUNE_MOVE_IDX     5 // Move to start pruning on in pv
#define PRUNE_MOVE_IDX        2 // Move to start pruning on otherwise

#define CHECKMATE_VALUE -MAX_EVAL + 1000

#define KMV_CNT 3 //How many killer moves are stored for a pos 

#define MAX_QUIESCE_PLY 10 //How far q search can go 
#define MAX_PLY 255 //How far the total search can go

#define LMR_DEPTH 3 //LMR not performed if depth < LMR_DEPTH

#define MAX_ASP_START PAWN_VALUE-100 //Maximum size of bounds for an aspiration window to start on
#define ASP_EDGE PAWN_VALUE/4  //Buffer size of aspiration window

#define FUTIL_DEPTH 2 //Depth to start futility pruning
#define FUTIL_MARGIN PAWN_VALUE-100 //Score difference for a node to be futility pruned

#define NULL_PRUNE_R 3 //How much Null prunin' takes off

#define DELTA_VALUE 2000


//static void selectSort(i32 i, Move *moveList, i32 *moveVals, i32 size);

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
   NODE_PRUNED_FUTIL,
   NODE_LMR_REDUCTIONS,

   NODE_BETA_CUT,
   NODE_ALPHA_RET,
   DEBUG_STATS_COUNT
} DebugStats;

static i64 debug[SEARCH_TYPE_COUNT][DEBUG_STATS_COUNT] = {0};

static Move debug_moveList[5][MAX_MOVES] = {0};
static i32 debug_moveVals[5][MAX_MOVES] = {0};
static i32 debug_size[5] = {0};

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

   i64 total_count = 0;
   i64 pvs_count = 0, zws_count = 0, q_count = 0;

   for (i32 i = 0; i < SEARCH_TYPE_COUNT; i++) {
      i64 node_count = debug[i][NODE_COUNT];
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
      printf("     Null Prunes: %" PRIu64 ", Futil Prunes: %" PRIu64 ", LMR: %" PRIu64 " \n",
            debug[i][NODE_PRUNED_NULL], debug[i][NODE_PRUNED_FUTIL], debug[i][NODE_LMR_REDUCTIONS]);
      printf("     TT Hits: %" PRIu64 ", TT PVS Returns: %" PRIu64 ", TT Beta Returns: %" PRIu64 ", TT Alpha Returns: %" PRIu64 " \n\n",
            debug[i][NODE_TT_HIT], debug[i][NODE_TT_PVS_RET], debug[i][NODE_TT_BETA_RET], debug[i][NODE_TT_ALPHA_RET]);
   }

   printf("Move List:\n");
   i32 max_list_size = 0;
   for(i32 i = 0; i < 5; i++){
      if(debug_size[i] > max_list_size){
         max_list_size = debug_size[i];
      }
   }
   for(i32 i = 0; i < max_list_size; i++){
      for(i32 j = 0; j < 5; j++){
         printMoveSpaced(debug_moveList[j][i]);
         printf(" - %8d", debug_moveVals[j][i]);
         printf(" | ");
      }
      printf("\n");
   }

   printf("\nTree searched %" PRIu64 " evals (pvs: %" PRIu64 ", zws: %" PRIu64 ", q: %" PRIu64 ")\n", total_count, pvs_count, zws_count, q_count);

   #ifdef DEBUG_TIME
   printf("\nTook %lf seconds, with %lf eval/sec\n", elap_time, ((double)total_count) / elap_time);
   #endif
}
#endif

/*
* Returns whether the position is a Repetition
*/
static inline i32 isRepetition(Position* pos){
   for(i32 i = pos->hashStack.last_reset_idx; i < pos->hashStack.current_idx; i++){
      if(pos->hashStack.ptr[i] == pos->hashStack.ptr[pos->hashStack.current_idx]) return 1;
   }
   return 0;
}

/*
* Function pertaining to storage of killer moves 
*/
static Move killerMoves[MAX_PLY][KMV_CNT] = {0};
static u32 kmvIdx = 0;

static inline void storeKillerMove(i32 ply, Move move){
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
static u32 historyTable[PLAYER_COUNT][BOARD_SIZE][BOARD_SIZE] = {0};

static inline void storeHistoryMove(char pos_flags, Move move, char depth){
   if(GET_FLAGS(move) & CAPTURE) return;
   historyTable[pos_flags & WHITE_TURN][GET_FROM(move)][GET_TO(move)] += 1 << depth;
}

u32 getHistoryScore(char pos_flags, Move move){
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
i32 getBestMove(Position pos
#ifdef MAX_DEPTH
, u32 depth
#endif
){

   clearKillerMoves(); //TODO: make thread safe!
   u32 i = 1;
   i32 eval = 0, eval_prev = 0, asp_upper = 0, asp_lower = 0;
   while(run_get_best_move 
         #ifdef MAX_DEPTH
         && i <= depth
         #endif
         ){
      #ifdef DEBUG
      startTreeDebug();
      startTTDebug();
      #endif

      Move *pvArray = malloc(sizeof(Move) * (i*i + i)/2);
      memset(pvArray, 0, sizeof(Move) * (i*i + i)/2);
      if(!pvArray){
         printf("info Warning: failed to allocate space for pvArray");
         return -1;
      }
      Position searchPos = pos;
      //printf("Running pv search at depth %d\n", i);
      if(i <= 2){
         eval = pvSearch(&searchPos, MIN_EVAL+1, MAX_EVAL, i, 0, pvArray, 0);
         searchPos = pos;
         eval_prev = eval;
         #ifdef DEBUG
         printf("Result from depth window: %d, %d i: %d eval: %d\n", MIN_EVAL+1, MAX_EVAL, i, eval);
         #endif
      } else {
         //Calculate the Aspiration Window
         i32 asp_dif = (abs(eval_prev - eval) / 2) + ASP_EDGE;
         asp_upper = asp_lower = MIN(asp_dif, MAX_ASP_START);
         i32 q = (eval_prev + eval + eval + eval) / 4; //Weighted towards the most recent eval
         #ifdef DEBUG
         printf("Running with window: %d, %d (eval: %d, eval_prev: %d, i: %d)\n", q-asp_lower, q+asp_upper, eval, eval_prev, i);
         #endif

         i32 eval_tmp = pvSearch(&searchPos, q-asp_lower, q+asp_upper, i, 0, pvArray, 0);
         searchPos = pos;
         while(eval_tmp <= q-asp_lower || eval_tmp >= q+asp_upper || pvArray[0] == NO_MOVE){
            if(eval_tmp <= q-asp_lower){ asp_lower += PAWN_VALUE/2;}
            if(eval_tmp >= q+asp_upper){ asp_upper += PAWN_VALUE/2;}
            if(pvArray[0] == NO_MOVE){
               asp_upper *= 2;
               asp_lower *= 2;
            }
            #ifdef DEBUG
            printf("Running again with window: %d, %d (eval: %d, eval_prev: %d, move: ", q-asp_lower, q+asp_upper, eval, eval_prev);
            printMove(pvArray[0]);
            printf(", i: %d", i);
            printf(")\n");
            #endif
            
            eval_tmp = pvSearch(&searchPos, q-asp_lower, q+asp_upper, i, 0, pvArray, 0);
            searchPos = pos;
         }
         eval_prev = eval;
         eval = eval_tmp;
      }
      

      #ifdef DEBUG
      printf("Principal Variation at depth %d: ", i);
      printPV(pvArray, i);
      printf("found with score %d\n", eval);
      printTreeDebug();
      printTTDebug();
      printf("\n-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
      #endif // DEBUG

      //TODO: move to a different thread
      #ifndef DEBUG
      printPVInfo(i, eval, pvArray);
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
static i32 pruneNullMoves(Position* pos, i32 beta, i32 depth, i32 ply, Move* pvArray){
   Position prevPos = *pos;
   makeNullMove(pos);
   i32 score = -zwSearch(pos, 1-beta, depth - NULL_PRUNE_R - 1, ply + 1, pvArray);
   *pos = prevPos;
   return score;
}

//Late move reduction
static u8 getSearchDepth(char curDepth, Move move, i32 allowLMR, char searchType){
   if(curDepth < LMR_DEPTH) return curDepth - 1;
   if(!allowLMR) return curDepth - 1;
   if(GET_FLAGS(move) & ~DOUBLE_PAWN_PUSH) return curDepth - 1; // If anything but double pawn push
   if(searchType == PVS){
      return curDepth/3;
   }
   else{
      return curDepth/3;
   }
   return curDepth - 1;
}

/*
*
*  PRINCIPAL VARIATION SEARCH
*
*/

i32 pvSearch( Position* pos, i32 alpha, i32 beta, char depth, char ply, Move* pvArray, i32 pvIndex) {
   //printf("Depth = %d, Ply = %d, Depth+ply = %d\n", depth, ply, depth+ply);
   if(!run_get_best_move) exit_search(pvArray);

   if( depth <= 0 ) {
      i32 q_eval = quiesce(pos, alpha, beta, ply, 0, pvArray);
      //printf("Returning q_eval: %d\n", q_eval);
      return q_eval;
   }

   #ifdef DEBUG
   debug[PVS][NODE_COUNT]++;
   #endif

   if(ply != 0 && isRepetition(pos)){
      //printf("is Repetition\n");
      return 0;
   }

   Move moveList[MAX_MOVES];
   i32 moveVals[MAX_MOVES];
   i32 size = generateLegalMoves(*pos, moveList);
   for(i32 i = 0; i < size; i++){
      moveVals[i] = 0;
   }

   //Store the list of moves and their evaluations at the start
   #ifdef DEBUG
   //Store the values at the starting time
   if(ply == 0){
      debug_size[0] = size;
      memcpy(debug_moveList[0], moveList, size*sizeof(i32));
      memcpy(debug_moveVals[0], moveVals, size*sizeof(i32));
   }
   #endif


   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE - ply;
      else return 0;
   }
   if(pos->halfmove_clock >= 100) return 0;

   i32 pvNextIndex = pvIndex + depth;

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
            default:
               break;
         }
      }
   }

   //Set up prunability
   char prunable = !(pos->flags & IN_CHECK);
   if(abs(beta-1) >= (-CHECKMATE_VALUE) - 2*MAX_MOVES) prunable = FALSE;
   if(pos->stage == END_GAME) prunable = FALSE;

   evalMoves(moveList, moveVals, size, *pos);

   //Store the list of moves and their evaluations at the start
   #ifdef DEBUG
   //Store the values at the starting time
   if(ply == 0){
      debug_size[1] = size;
      memcpy(debug_moveList[1], moveList, size*sizeof(i32));
      memcpy(debug_moveVals[1], moveVals, size*sizeof(i32));
   }
   #endif

   #ifdef DEBUG
   if(size > 0) debug[PVS][NODE_LOOP_CHILDREN]++;
   #endif

   Move bestMove = NO_MOVE;
   i32 bestScore = MIN_EVAL;
   u8 exact = FALSE;
   Position prevPos = *pos;
   for (i32 i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      selectSort(i, moveList, moveVals, size, ttMove, killerMoves[(i32)ply], KMV_CNT);

      makeMove(pos, moveList[i]);

      // Update prunability PVS
      u8 prunable_move = prunable;
      if(i <= PV_PRUNE_MOVE_IDX) prunable_move = FALSE;
      if(pos->flags & IN_CHECK) prunable_move = FALSE; // If in check
      if(GET_FLAGS(moveList[i]) & ~DOUBLE_PAWN_PUSH) prunable_move = FALSE; // If the move is anything but dpp / queit
      if(pos->stage == END_GAME) prunable_move = FALSE; // If its the endgame

      if( prunable_move && 
          depth == 1 && 
          pos->quick_eval + moveVals[i] < alpha - FUTIL_MARGIN){ //Futility Pruning (Just prune the nodes that are expect to be poop)
         #ifdef DEBUG
         //printf("PVS Futil Prune (qe=%d) + (moveVal=%d) < %d\n", quickEval(*pos), moveVals[i], beta-1 + FUTIL_MARGIN);
         debug[PVS][NODE_PRUNED_FUTIL]++;
         #endif
         *pos = prevPos; //Unmake Move
         continue;
      }

      i32 score;
      if ( i == 0 ) { // Only do full PV on the first move
         score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, pvNextIndex);
         //printf("PV b search pv score = %d\n", score);
      } else {
         i32 search_depth = getSearchDepth(depth, moveList[i], prunable_move, PVS);
         #ifdef DEBUG
         debug[PVS][NODE_LMR_REDUCTIONS] += MAX(((depth - 1) - search_depth), 0);
         #endif
         score = -zwSearch(pos, -alpha, search_depth, ply + 1, pvArray);
         if ( score > alpha ){
            score = -pvSearch(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, pvNextIndex);
         }
      }

      *pos = prevPos; //Unmake Move

      if( score >= beta ) { //Beta cutoff
         storeTTEntry(pos->hash, depth, score, CUT_NODE, moveList[i]);
         storeKillerMove(ply, moveList[i]);
         storeHistoryMove(pos->flags, moveList[i], depth);
      
         #ifdef DEBUG
         //printf("Returning beta cutoff: %d >= %d\n", score, beta);
         debug[PVS][NODE_BETA_CUT]++;
         if(ply == 0){
            debug_size[3] = size;
            memcpy(debug_moveList[3], moveList, size*sizeof(i32));
            memcpy(debug_moveVals[3], moveVals, size*sizeof(i32));
         }
         #endif
         return beta;
      }
      if( score > alpha ) {  //Improved alpha
         alpha = score;
         exact = TRUE;
         pvArray[pvIndex] = moveList[i];
         movcpy(pvArray + pvIndex + 1, pvArray + pvNextIndex, depth - 1);
      }
      if( score > bestScore ){ //Improved best move
         bestMove = moveList[i];
         bestScore = moveVals[i];
      }
   }
   if (exact) {
      // PV Node (exact value)
      storeTTEntry(pos->hash, depth, alpha, PV_NODE, pvArray[pvIndex]);
   } else {
      // ALL Node (upper bound)
      storeTTEntry(pos->hash, depth, bestScore, ALL_NODE, bestMove);
   }
   #ifdef DEBUG
   debug[PVS][NODE_ALPHA_RET]++;

   if(ply == 0){
      debug_size[4] = size;
      memcpy(debug_moveList[4], moveList, size*sizeof(i32));
      memcpy(debug_moveVals[4], moveVals, size*sizeof(i32));
   }
   #endif
   return alpha;
}



/*
*
*  ZERO WINDOW SEARCH
*
*/
i32 zwSearch( Position* pos, i32 beta, char depth, char ply, Move* pvArray ) {
   if(!run_get_best_move) exit_search(pvArray);
   // alpha == beta - 1
   // this is either a cut- or all-node
   if( depth <= 0 ) return quiesce(pos, beta-1, beta, ply, 0, pvArray);
   #ifdef DEBUG
   debug[ZWS][NODE_COUNT]++;
   #endif

   if(isRepetition(pos)) return 0;

   Move moveList[MAX_MOVES];
   i32 moveVals[MAX_MOVES];
   i32 size = generateLegalMoves(*pos, moveList);
   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return CHECKMATE_VALUE + ply;
      else return 0;
   }
   if(pos->halfmove_clock >= 100) return 0;

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

   //Set up prunability
   char prunable = !(pos->flags & IN_CHECK);
   if(abs(beta-1) >= (-CHECKMATE_VALUE) - 2*MAX_MOVES) prunable = FALSE;
   if(pos->stage == END_GAME) prunable = FALSE;

   //Null move prunin'
   if(prunable && depth > (NULL_PRUNE_R + 1) && pruneNullMoves(pos, beta, depth, ply, pvArray) >= beta){
      #ifdef DEBUG
      debug[ZWS][NODE_PRUNED_NULL]++;
      #endif
      return beta;
   }

   evalMoves(moveList, moveVals, size, *pos);

   #ifdef DEBUG
   if(size > 0) debug[ZWS][NODE_LOOP_CHILDREN]++;
   #endif

   Position prevPos = *pos;
   for (i32 i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      selectSort(i, moveList, moveVals, size, ttMove, killerMoves[(i32)ply], KMV_CNT);

      makeMove(pos, moveList[i]);

      // Set Move prunability prunability ZWS
      u8 prunable_move = prunable;
      if(i <= PRUNE_MOVE_IDX) prunable_move = FALSE;
      if(pos->flags & IN_CHECK) prunable_move = FALSE; // If in check
      if(GET_FLAGS(moveList[i]) & ~DOUBLE_PAWN_PUSH) prunable_move = FALSE; // If the move is anything but dpp
      if(pos->stage == END_GAME) prunable_move = FALSE; // If its the endgame

      if( prunable_move && depth == 1){
         if(pos->quick_eval + moveVals[i] < beta-1 - FUTIL_MARGIN){ // Futility Pruning
            #ifdef DEBUG
            debug[ZWS][NODE_PRUNED_FUTIL]++;
            #endif
            *pos = prevPos; // Unmake Move
            continue;
         }
      }
      
      char search_depth = getSearchDepth(depth, moveList[i], prunable_move, ZWS);
      #ifdef DEBUG
      debug[ZWS][NODE_LMR_REDUCTIONS] += MAX(((depth - 1) - search_depth), 0);
      //printf("zws further search score %d\n", score);
      #endif
      i32 score = -zwSearch(pos, 1-beta, search_depth, ply + 1, pvArray);
      
      *pos = prevPos; // Unmake Move

      if( score >= beta ){ // Beta Cutoff
         storeTTEntry(pos->hash, depth, score, CUT_NODE, moveList[i]);
         storeKillerMove(ply, moveList[i]);
         storeHistoryMove(pos->flags, moveList[i], depth);
         #ifdef DEBUG
         debug[ZWS][NODE_BETA_CUT]++;
         //printf("zws fail hard beta cut %d\n", beta);
         #endif
         return beta;   // fail-hard beta-cutoff
      }
   }

   //printf("zws fail %d\n", beta-1);
   #ifdef DEBUG
   debug[ZWS][NODE_ALPHA_RET]++;
   #endif
   return beta-1; // fail-hard, return alpha
}

//quisce search
i32 quiesce( Position* pos, i32 alpha, i32 beta, char ply, char q_ply, Move* pvArray) {
   if(!run_get_best_move) exit_search(pvArray);
   #ifdef DEBUG
   debug[QS][NODE_COUNT]++;
   //printf("Pos->Eval in q search: %d\n", pos->eval);
   #endif
   // Handle Draw or Mate
   if(pos->halfmove_clock >= 100) return 0;
   if(isRepetition(pos)) return 0;
   
   Move moveList[MAX_MOVES];
   i32 moveVals[MAX_MOVES];
   i32 size = generateThreatMoves(*pos, moveList);
   if(size == 0){
      size = generateLegalMoves(*pos, moveList);
      if(size == 0){ // Check for end game
         if(pos->flags & IN_CHECK){
            return CHECKMATE_VALUE + ply; // Check Mate
         }
         else{
            return 0; // Stalemate
         }
      }
      else{
         return evaluate(*pos
         #ifdef DEBUG
         , FALSE
         #endif
         );
      }
   }

   i32 stand_pat = evaluate(*pos
                     #ifdef DEBUG
                     , FALSE
                     #endif 
                     );
   if( stand_pat >= beta )
      return beta;
   if( alpha < stand_pat ){
      alpha = stand_pat;
   }

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

   evalMoves(moveList, moveVals, size, *pos);
   
   Move bestMove = NO_MOVE;
   i32 bestScore = MIN_EVAL;
   #ifdef DEBUG
   if(size > 0) debug[QS][NODE_LOOP_CHILDREN]++;
   #endif
   i32 exact = 0;
   Position prevPos = *pos;
   for (i32 i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      selectSort(i, moveList, moveVals, size, ttMove, killerMoves[(i32)ply], KMV_CNT); 

      if(moveVals[i] < 0) break;

      makeMove(pos, moveList[i]);

      i32 score = -quiesce(pos,  -beta, -alpha, ply + 1, q_ply + 1, pvArray);

      *pos = prevPos; //Unmake Move

      if( score >= beta ){
         storeKillerMove(ply, moveList[i]);
         storeTTEntry(prevPos.hash, 0, score, CUT_NODE, moveList[i]);
         #ifdef DEBUG
         debug[QS][NODE_BETA_CUT]++;
         #endif
         return beta;
      }

      //Delta Pruning
      i32 delta = DELTA_VALUE;
      if (GET_FLAGS(moveList[i]) & PROMOTION) delta += (QUEEN_VALUE - PAWN_VALUE);
      if ( score < alpha - delta && pos->stage != END_GAME) {
         storeKillerMove(ply, moveList[i]);
         //storeTTEntry(pos->hash, 0, bestScore, Q_ALL_NODE, moveList[i]);
         #ifdef DEBUG
         debug[QS][NODE_PRUNED_FUTIL]++;
         #endif
         return alpha;
      }


      if( score > alpha ){
         alpha = score;
         exact = TRUE;
      }
      if( score > bestScore){
         bestScore = score;
         bestMove = moveList[i];
      }
   }

   if(exact) storeTTEntry(pos->hash, 0, alpha, Q_EXACT_NODE, bestMove);
   else if(bestScore != MIN_EVAL) storeTTEntry(pos->hash, 0, bestScore, Q_ALL_NODE, bestMove);

   #ifdef DEBUG
   debug[QS][NODE_ALPHA_RET]++;
   #endif
   return alpha;
}


void selectSort(i32 i, Move *moveList, i32 *moveVals, i32 size, Move ttMove, Move *killerMoves, i32 kmv_size) {
    i32 maxIdx = i;

    for (i32 j = i + 1; j < size; j++) {
        if (moveList[j] == ttMove){
            maxIdx = j;
            break;
        }

        i32 found = 0;
        for(i32 k = 0; k < kmv_size; k++){
            if(moveList[j] == killerMoves[k]){
                maxIdx = j;
                found = 1;
                break;
            }
        }
        if(found) break;

        if (moveVals[j] > moveVals[maxIdx]) {
            maxIdx = j;
        }
    }
   
    if (maxIdx != i) {
        i32 tempVal = moveVals[i];
        moveVals[i] = moveVals[maxIdx];
        moveVals[maxIdx] = tempVal;

        Move tempMove = moveList[i];
        moveList[i] = moveList[maxIdx];
        moveList[maxIdx] = tempMove;
    }
}




