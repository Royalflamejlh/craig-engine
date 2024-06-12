#include "tree.h"
#include <stdio.h>
#include <string.h>
#include "bitboard/bbutils.h"
#include "search.h"
#include "movement.h"
#include "types.h"
#include "util.h"
#include "evaluator.h"
#include "transposition.h"
#include "globals.h"
#include "tables.h"

#if defined(__unix__) || defined(__APPLE__)
#include "pthread.h"
#include <time.h>
#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

#define ID_STEP 1 //Changing this may break Aspiration windows (it will)

#define PV_PRUNE_MOVE_IDX     5 // Move to start pruning on in pv
#define PRUNE_MOVE_IDX        2 // Move to start pruning on otherwise

#define MAX_QUIESCE_PLY 5 //How far q search can go 

#define LMR_DEPTH 3       // LMR not performed if depth < LMR_DEPTH

#define ASP_EDGE (PAWN_VALUE/4)         // Buffer size of aspiration window
#define HELPER_ASP_EDGE (PAWN_VALUE/2)  // Buffer size of aspiration window in helper search

#define PV_FUTIL_MARGIN 300 // Score difference for a node to be futility pruned
#define ZW_FUTIL_MARGIN 200

#define NULL_PRUNE_R 3  // How much Null prunin' takes off
#define NMR_MARGIN 2000 // The higher this is, the more likely a null move search is to be taken

#define DELTA_VALUE 750 // Min move value to be tried in Q search

#define HELPER_MOVE_DISORDER 3 // Increasing this changes how out of order helper searches look at moves
#define HELPER_THREAD_DISORDER 3 // How different each helper thread searches from one another


typedef enum searchs{
   PVS,
   ZWS,
   QS,
   SEARCH_TYPE_COUNT
} SearchType;

// UNIX Functions
#if defined(__unix__) || defined(__APPLE__) // UNIX

void startStats(SearchStats* stats){
   clock_gettime(CLOCK_MONOTONIC, &stats->start_time);
   stats->node_count = 0;
   stats->elap_time = 0;
}

void stopStats(SearchStats* stats){
   clock_gettime(CLOCK_MONOTONIC, &stats->end_time);
   stats->elap_time = (stats->end_time.tv_sec - stats->start_time.tv_sec) +
                     (stats->end_time.tv_nsec - stats->start_time.tv_nsec) / 1e9;
}

#elif defined(_WIN32) || defined(_WIN64)

void startStats(SearchStats* stats){
   LARGE_INTEGER freq;
   QueryPerformanceFrequency(&freq);
   stats->freq_inv = 1.0 / freq.QuadPart;
   QueryPerformanceCounter(&stats->start_time);
   stats->node_count = 0;
   stats->elap_time = 0;
}

void stopStats(SearchStats* stats){
   QueryPerformanceCounter(&stats->end_time);
   stats->elap_time = (stats->end_time.QuadPart - stats->start_time.QuadPart) * stats->freq_inv;
}
#endif // Windows

#ifdef DEBUG
#include <math.h>
#include <assert.h>
#if defined(__unix__) || defined(__APPLE__)
#define DEBUG_TIME
#include <time.h>
static struct timespec start_time, end_time;
#endif

static double calculateEBF();

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

static u64 debug_node_counts[MAX_SEARCH_DEPTH] = {0};
static u32 debug_current_depth = 0;

static Move debug_moveList[5][MAX_MOVES] = {0};
static i32 debug_moveVals[5][MAX_MOVES] = {0};
static i32 debug_size[5] = {0};

void clearDebug() {
    memset(debug, 0, sizeof(debug));
    memset(debug_node_counts, 0, sizeof(debug_node_counts));
    debug_current_depth = 0;
    for (int i = 0; i < 5; i++) {
        memset(debug_moveList[i], 0, sizeof(debug_moveList[i]));
        memset(debug_moveVals[i], 0, sizeof(debug_moveVals[i]));
    }
    memset(debug_size, 0, sizeof(debug_size));
}

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

   // EBF Calculating Code
   debug_node_counts[debug_current_depth] = total_count;
   double ebf = calculateEBF();
   printf("EBF found to be: %f\n", ebf);
   debug_current_depth++;
}

static double calculateEBF(){
   if(debug_current_depth < 2) return 0;
   double cntCurr = debug_node_counts[debug_current_depth];
   double cntPrev = debug_node_counts[debug_current_depth - 2];
   if(cntPrev == 0) return 0;
   if(cntCurr <= debug_current_depth + 1) return 0;
   if(cntPrev <= debug_current_depth + 1) return 0;
   return sqrt(cntCurr / cntPrev);

}
#endif

#define TT_MOVE_BONUS       3000000 // Bonus for move being in the TT
#define CAPTURE_MOVE_BONUS  2000000 // Bonus for move being a capture
#define KILLER_MOVE_BONUS   1000000 // Bonus for move being killer move

static inline u32 select_sort(u32 i, u32 evalIdx, Position* pos, Move *moveList, i32 *moveVals, u32 size, KillerMoves* km, Move ttMove, u32 ply) {
   u32 maxIdx = i;

   if(moveList[i] == ttMove){
      moveVals[i] = TT_MOVE_BONUS;
      return evalIdx; // Return if the next move is the TTMove
   }

   if(i <= evalIdx){
      moveVals[i] = evalMove(moveList[i], pos);
      if(GET_FLAGS(moveList[i]) > DOUBLE_PAWN_PUSH){
         moveVals[i] += CAPTURE_MOVE_BONUS;
      } else if(isKillerMove(km, moveList[i], ply)){
         moveVals[i] += KILLER_MOVE_BONUS;
      }
      evalIdx = i+1;
   }

   for (u32 j = i + 1; j < size; j++) {
      if(moveList[j] == ttMove){ // If the move is in the TT sort immediatly
         maxIdx = j;
         moveVals[j] = TT_MOVE_BONUS;
         break;
      }

      if(j <= evalIdx){ // If the move hasn't been evaluated yet calculate score
         moveVals[j] = evalMove(moveList[j], pos);
         if(GET_FLAGS(moveList[j]) > DOUBLE_PAWN_PUSH){
            moveVals[j] += CAPTURE_MOVE_BONUS;
         } else if(isKillerMove(km, moveList[j], ply)){
            moveVals[j] += KILLER_MOVE_BONUS;
         }
         evalIdx = j+1;
      }

      if (moveVals[j] > moveVals[maxIdx]) {
         maxIdx = j;
      }
   }

   if (maxIdx != i) { // Swap the moves
      i32 tempVal = moveVals[i];
      moveVals[i] = moveVals[maxIdx];
      moveVals[maxIdx] = tempVal;

      Move tempMove = moveList[i];
      moveList[i] = moveList[maxIdx];
      moveList[maxIdx] = tempMove;
   }

   return evalIdx;
}

/*
 * Simple select sort for q search
 */
static inline void q_select_sort(i32 i, Move *moveList, i32 *moveVals, i32 size) {
   i32 maxIdx = i;

   for (i32 j = i + 1; j < size; j++) {
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

/*
 * Select sort for the helper search with slighly different ordering
 */
static inline u32 helper_select_sort(u32 i, u32 evalIdx, Position* pos, Move *moveList, i32 *moveVals, u32 size, KillerMoves* km, Move ttMove, u32 ply, u32 thread_num) {
   i32 thread_dif = (thread_num % 2) ? -1 : 1;
   moveVals[i] += (i * HELPER_MOVE_DISORDER) + (thread_dif * (i32)thread_num * HELPER_THREAD_DISORDER);
   select_sort(i, evalIdx, pos, moveList, moveVals, size, km, ttMove, ply);
   return evalIdx;
}

/*
 * On a TT hit in the mainline fills the pv array with the PV for printing
 */
static inline void pvFill(Position pos, Move* pvArray, u8 depth){
   u8 ply = 0;
   TTEntry* ttEntry = getTTEntry(pos.hash);
   while(ttEntry && ttEntry->depth > 0 && ply < depth){
      pvArray[ply] = ttEntry->move;
      #ifdef DEBUG
      if(ttEntry->move == NO_MOVE) printf("NO MOVE FOUND IN PV");
      #endif
      makeMove(&pos, ttEntry->move);
      ply++;
      ttEntry = getTTEntry(pos.hash);
   }
}

/*
* Sets up the aspiration window, then searches the 
*/
i32 search_tree(Position pos, u32 depth, Move *pvArray, KillerMoves* km, i32 eval, SearchStats* stats){
   if(!pvArray){
      printf("info string Warning: No PV Array found!\n");
      return 0;
   }
   #ifdef DEBUG
   startTreeDebug();
   startTTDebug();
   #endif

   startStats(stats);

   Position searchPos = pos;

   //printf("Running pv search at depth %d\n", i);
   if(depth <= 2){
      eval = pv_search(&searchPos, MIN_EVAL+1, MAX_EVAL-1, depth, 0, pvArray, km, stats);
      searchPos = pos;
      #ifdef DEBUG
      printf("Result from depth window: %d, %d i: %d eval: %d\n", MIN_EVAL+1, MAX_EVAL, depth, eval);
      #endif
   } else {
      i32 asp_lower, asp_upper;
      //Calculate the Aspiration Window
      asp_upper = asp_lower = ASP_EDGE;
      i32 q = eval;
      #ifdef DEBUG
      printf("Running with window: %d, %d (eval_prev: %d, depth: %d)\n", q-asp_lower, q+asp_upper, eval, depth);
      #endif
      eval = pv_search(&searchPos, q-asp_lower, q+asp_upper, depth, 0, pvArray, km, stats);
      searchPos = pos;
      while(eval <= q-asp_lower || eval >= q+asp_upper || pvArray[0] == NO_MOVE){
         if(abs(eval) == CHECKMATE_VALUE) break;
         if(eval <= q-asp_lower){
            asp_upper = ASP_EDGE;
            asp_lower = (asp_lower + ASP_EDGE) * 2;
         }
         else if(eval >= q+asp_upper){
            asp_upper = (asp_upper + ASP_EDGE) * 2;
            asp_lower = ASP_EDGE;
         }
         else if(pvArray[0] == NO_MOVE){
            asp_upper = (asp_upper + ASP_EDGE) * 2;
            asp_lower = (asp_lower + ASP_EDGE) * 2;
         }
         #ifdef DEBUG
         printf("Running again with window: %d, %d (eval: %d, q: %d, move: ", q-asp_lower, q+asp_upper, eval, q);
         printMove(pvArray[0]);
         printf(", depth: %d", depth);
         printf(")\n");
         #endif

         q = eval;
         eval = pv_search(&searchPos, q-asp_lower, q+asp_upper, depth, 0, pvArray, km, stats);
         searchPos = pos;
      }
   }
   pvFill(searchPos, pvArray, depth);

   #ifdef DEBUG
   printf("Principal Variation at depth %d: ", depth);
   printPV(pvArray, depth);
   printf(" found with score %d\n", eval);
   printTreeDebug();
   printTTDebug();
   printf("\n-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n");
   #endif // DEBUG

   stopStats(stats);
   return eval;
}

/*
 * Search tree function called from a helper thread with slighly different bounds and move sorting
 */
i32 helper_search_tree(Position pos, u32 depth, Move *pvArray, KillerMoves* km, i32 eval, SearchStats* stats, u32 thread_num){
   Position searchPos = pos;
   i32 asp_lower, asp_upper;
   asp_upper = asp_lower = HELPER_ASP_EDGE;
   i32 q = eval;
   eval = helper_pv_search(&searchPos, q-asp_lower, q+asp_upper, depth, 0, pvArray, km, stats, thread_num);
   searchPos = pos;
   while(eval <= q-asp_lower || eval >= q+asp_upper || pvArray[0] == NO_MOVE){
      if(abs(eval) == CHECKMATE_VALUE) break;
      if(eval <= q-asp_lower){
         asp_upper = HELPER_ASP_EDGE;
         asp_lower = (asp_lower + HELPER_ASP_EDGE) * 2;
      }
      else if(eval >= q+asp_upper){
         asp_upper = (asp_upper + HELPER_ASP_EDGE) * 2;
         asp_lower = HELPER_ASP_EDGE;
      }
      else if(pvArray[0] == NO_MOVE){
         asp_upper = (asp_upper + HELPER_ASP_EDGE) * 2;
         asp_lower = (asp_lower + HELPER_ASP_EDGE) * 2;
      }
      q = eval;
      eval = helper_pv_search(&searchPos, q-asp_lower, q+asp_upper, depth, 0, pvArray, km, stats, thread_num);
      searchPos = pos;
   }
   return eval;
}


/*
* Pruning Methods
*/

// Null Move Search
static inline i32 pruneNullMoves(Position* pos, i32 beta, i32 depth, i32 ply, KillerMoves* km, SearchStats* stats){
   Position prevPos = *pos;
   makeNullMove(pos);
   i32 score = -zw_search(pos, 1-beta, depth - NULL_PRUNE_R - 1, ply + 1, km, stats, TRUE);
   *pos = prevPos;
   return score;
}

// Late move reduction
static inline u8 getLMRDepth(u8 curDepth, u8 moveIdx, u8 moveCount, Move move, i32 allowLMR){
   if(curDepth < LMR_DEPTH) return curDepth - 1;
   if(!allowLMR) return curDepth - 1;
   if(GET_FLAGS(move) > DOUBLE_PAWN_PUSH) return curDepth - 1; // If anything but double pawn push
   if(moveIdx < moveCount / 8) return curDepth - 1;
   if(moveIdx < moveCount / 4) return curDepth - 2;
   if(moveIdx < moveCount / 2) return curDepth / 3;
   return curDepth / 4;
}

/*
*
*  PRINCIPAL VARIATION SEARCH
*
*/
i32 pv_search( Position* pos, i32 alpha, i32 beta, i8 depth, u8 ply, Move* pvArray, KillerMoves* km, SearchStats* stats) {
   //printf("Depth = %d, Ply = %d, Depth+ply = %d\n", depth, ply, depth+ply);
   if(!run_get_best_move) exit_search(pos);

   stats->node_count++;
   #ifdef DEBUG
   debug[PVS][NODE_COUNT]++;
   #endif
   pvArray[ply] = NO_MOVE;

   if(ply != 0 && (pos->halfmove_clock >= 100 || isInsufficient(*pos) || isRepetition(pos))) return 0;

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
      if(pos->flags & IN_CHECK) return -(CHECKMATE_VALUE - ply);
      else return 0;
   }

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
               #ifdef DEBUG
               debug[PVS][NODE_TT_PVS_RET]++;
               #endif
               pvArray[ply] = ttEntry->move;
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

   if( depth <= 0 ) {
      i32 q_eval = q_search(pos, alpha, beta, ply, 0, stats);
      if     (q_eval < alpha) storeTTEntry(pos->hash, 0, q_eval, ALL_NODE, NO_MOVE);
      else if(q_eval >= beta) storeTTEntry(pos->hash, 0, q_eval, CUT_NODE, NO_MOVE);
      else                    storeTTEntry(pos->hash, 0, q_eval,  PV_NODE, NO_MOVE);
      return q_eval;
   }

   //Set up prunability
   char prunable = !(pos->flags & IN_CHECK);
   if(abs(beta-1) >= CHECKMATE_VALUE/2) prunable = FALSE;
   if(pos->stage == END_GAME) prunable = FALSE;

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
   u32 evalIdx = 0; // Used for select sort
   Position prevPos = *pos;
   for (i32 i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      evalIdx = select_sort(i, evalIdx, pos, moveList, moveVals, size, km, ttMove, ply);
      makeMove(pos, moveList[i]);
      // Update Prunability PVS
      u8 prunable_move = prunable;
      if(i <= PV_PRUNE_MOVE_IDX || pos->flags & IN_CHECK || (GET_FLAGS(moveList[i]) > DOUBLE_PAWN_PUSH) || pos->stage == END_GAME ) prunable_move = FALSE;

      if( prunable_move && depth == 1 && abs(alpha) < (CHECKMATE_VALUE/2) && abs(beta) < (CHECKMATE_VALUE/2)){ // Futility Pruning
         if(prevPos.quick_eval + moveVals[i] < alpha - PV_FUTIL_MARGIN){ 
            #ifdef DEBUG
            debug[PVS][NODE_PRUNED_FUTIL]++;
            #endif
            *pos = prevPos; //Unmake Move
            continue;
         }
      }

      i32 score;
      if ( i == 0 ) { // Only do full PV on the first move
         score = -pv_search(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, km, stats);
         //printf("PV b search pv score = %d\n", score);
      } else {
         score = -zw_search(pos, -alpha, depth - 1, ply + 1, km, stats, FALSE);
         if ( score > alpha ){
            score = -pv_search(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, km, stats);
         }
      }

      *pos = prevPos; //Unmake Move

      if( score >= beta ) { //Beta cutoff
         storeTTEntry(pos->hash, depth, score, CUT_NODE, moveList[i]);
         storeKillerMove(km, ply, moveList[i]);
         //storeHistoryMove(pos->flags, moveList[i], depth);
      
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
         pvArray[ply] = moveList[i];
      }
      if( score > bestScore ){ //Improved best move
         bestMove = moveList[i];
         bestScore = score;
      }
   }
   if (exact) {
      // PV Node (exact value)
      storeTTEntry(pos->hash, depth, alpha, PV_NODE, pvArray[ply]);
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

i32 helper_pv_search( Position* pos, i32 alpha, i32 beta, i8 depth, u8 ply, Move* pvArray, KillerMoves* km, SearchStats* stats, u32 thread_num) {
   if(!run_get_best_move) exit_search(pos);
   pvArray[ply] = NO_MOVE;
   if(ply != 0 && (pos->halfmove_clock >= 100 || isInsufficient(*pos) || isRepetition(pos))) return 0;

   Move moveList[MAX_MOVES];
   i32 moveVals[MAX_MOVES] = {0};

   i32 size = generateLegalMoves(*pos, moveList);

   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return -(CHECKMATE_VALUE - ply);
      else return 0;
   }

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
               pvArray[ply] = ttEntry->move;
               return ttEntry->eval;
            case CUT_NODE: // Lower bound
               if (ttEntry->eval >= beta){
                  return beta;
               }
               break;
            case ALL_NODE: // Upper bound
               if (ttEntry->eval < alpha){
                  return alpha;
               }
               break;
            default:
               break;
         }
      }
   }
   if( depth <= 0 ) {
      i32 q_eval = q_search(pos, alpha, beta, ply, 0, stats);
      if     (q_eval < alpha) storeTTEntry(pos->hash, 0, q_eval, ALL_NODE, NO_MOVE);
      else if(q_eval >= beta) storeTTEntry(pos->hash, 0, q_eval, CUT_NODE, NO_MOVE);
      else                    storeTTEntry(pos->hash, 0, q_eval,  PV_NODE, NO_MOVE);
      return q_eval;
   }
   
   Move bestMove = NO_MOVE;
   i32 bestScore = MIN_EVAL;
   u8 exact = FALSE;
   u32 evalIdx = 0; // Used for select sort
   Position prevPos = *pos;
   for (i32 i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      evalIdx = helper_select_sort(i, evalIdx, pos, moveList, moveVals, size, km, ttMove, ply, thread_num);
      makeMove(pos, moveList[i]);
      i32 score;
      if ( i == 0 ) {
         score = -pv_search(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, km, stats);
      } else {
         score = -zw_search(pos, -alpha, depth - 1, ply + 1, km, stats, FALSE);
         if ( score > alpha ){
            score = -pv_search(pos, -beta, -alpha, depth - 1, ply + 1, pvArray, km, stats);
         }
      }
      *pos = prevPos;
      if( score >= beta ) {
         storeTTEntry(pos->hash, depth, score, CUT_NODE, moveList[i]);
         storeKillerMove(km, ply, moveList[i]);
         return beta;
      }
      if( score > alpha ) {
         alpha = score;
         exact = TRUE;
         pvArray[ply] = moveList[i];
      }
      if( score > bestScore ){
         bestMove = moveList[i];
         bestScore = score;
      }
   }
   if (exact) {
      storeTTEntry(pos->hash, depth, alpha, PV_NODE, pvArray[ply]);
   } else {
      storeTTEntry(pos->hash, depth, bestScore, ALL_NODE, bestMove);
   }
   return alpha;
}


/*
*
*  ZERO WINDOW SEARCH
*
*/
i32 zw_search( Position* pos, i32 beta, i8 depth, u8 ply, KillerMoves* km, SearchStats* stats, u8 isNull) {
   if(!run_get_best_move) exit_search(pos);
   // alpha == beta - 1
   // this is either a cut- or all-node

   stats->node_count++;
   #ifdef DEBUG
   debug[ZWS][NODE_COUNT]++;
   #endif

   if(pos->halfmove_clock >= 100 || isInsufficient(*pos) || isRepetition(pos)) return 0;

   Move moveList[MAX_MOVES];
   i32 moveVals[MAX_MOVES];
   i32 size = generateLegalMoves(*pos, moveList);
   //Handle Draw or Mate
   if(size == 0){
      if(pos->flags & IN_CHECK) return -(CHECKMATE_VALUE - ply);
      else return 0;
   }

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

   if( depth <= 0 ){
      i32 q_eval = q_search(pos, beta-1, beta, ply, 0, stats);
      if     (q_eval < beta-1) storeTTEntry(pos->hash, 0, q_eval, ALL_NODE, NO_MOVE);
      else if(q_eval >= beta)  storeTTEntry(pos->hash, 0, q_eval, CUT_NODE, NO_MOVE);
      return q_eval;
   }

   //Set up prunability
   char prunable = !(pos->flags & IN_CHECK);
   if(pos->stage == END_GAME) prunable = FALSE;

   //Null move prunin'
   if(prunable && !isNull 
               && depth > NULL_PRUNE_R + 1 
               && pos->quick_eval >= (beta - NMR_MARGIN)){
      if(pruneNullMoves(pos, beta, depth, ply, km, stats) >= beta){
         #ifdef DEBUG
         debug[ZWS][NODE_PRUNED_NULL]++;
         #endif
         return beta;
      }
   }

   #ifdef DEBUG
   if(size > 0) debug[ZWS][NODE_LOOP_CHILDREN]++;
   #endif

   Position prevPos = *pos;
   u32 evalIdx = 0;
   for (i32 i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      
      evalIdx = select_sort(i, evalIdx, pos, moveList, moveVals, size, km, ttMove, ply);
      makeMove(pos, moveList[i]);

      // Set Move prunability prunability ZWS
      u8 prunable_move = prunable;
      if(i <= PRUNE_MOVE_IDX || pos->flags & IN_CHECK || (GET_FLAGS(moveList[i]) > DOUBLE_PAWN_PUSH) || pos->stage == END_GAME) prunable_move = FALSE;

      if( prunable_move && depth == 1 && abs(beta) < (CHECKMATE_VALUE/2) ){ // Futility Pruning
         if((prevPos.quick_eval + moveVals[i]) < ((beta-1) - ZW_FUTIL_MARGIN)){ 
            #ifdef DEBUG
            debug[ZWS][NODE_PRUNED_FUTIL]++;
            #endif // Unmake Move
            *pos = prevPos;
            continue;
         }
      }
      
      char search_depth = getLMRDepth(depth, i, size, moveList[i], prunable_move);
      #ifdef DEBUG
      debug[ZWS][NODE_LMR_REDUCTIONS] += MAX(((depth - 1) - search_depth), 0);
      //printf("zws further search score %d\n", score);
      #endif
      i32 score = -zw_search(pos, 1-beta, search_depth, ply + 1, km, stats, FALSE);
      *pos = prevPos; // Unmake Move

      if( score >= beta ){ // Beta Cutoff
         storeTTEntry(pos->hash, depth, score, CUT_NODE, moveList[i]);
         storeKillerMove(km, ply, moveList[i]);
         //storeHistoryMove(pos->flags, moveList[i], depth);
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
i32 q_search( Position* pos, i32 alpha, i32 beta, u8 ply, u8 q_ply, SearchStats* stats) {
   if(!run_get_best_move) exit_search(pos);
   stats->node_count++;
   #ifdef DEBUG
   debug[QS][NODE_COUNT]++;
   //printf("Pos->Eval in q search: %d\n", pos->eval);
   #endif
   // Check the bounds
   if(q_ply >= MAX_QUIESCE_PLY) return alpha;
   // Handle Draw or Mate
   if(pos->halfmove_clock >= 100 || isInsufficient(*pos) || isRepetition(pos)) return 0;

   #ifdef DEBUG
   i32 stand_pat = evaluate(*pos, FALSE); // Check to see if the player can opt to not move and be better
   #else
   i32 stand_pat = evaluate(*pos); // Check to see if the player can opt to not move and be better
   #endif

   if(!(pos->flags & IN_CHECK) && stand_pat >= beta){
      return beta;
   }
   if( alpha < stand_pat ){
      alpha = stand_pat;
   }

   // Early Delta Pruning (See if down more than Queen)
   i32 delta = QUEEN_VALUE;
   if (canPromotePawn(pos)) delta += (QUEEN_VALUE - PAWN_VALUE);
   if (!(pos->flags & IN_CHECK) && stand_pat + delta < alpha && pos->stage != END_GAME) {
      #ifdef DEBUG
      debug[QS][NODE_PRUNED_FUTIL]++;
      #endif
      return alpha;
   }
   
   Move moveList[MAX_MOVES];
   i32 moveVals[MAX_MOVES];
   i32 size = generateThreatMoves(*pos, moveList);
   if(size == 0){
      size = generateLegalMoves(*pos, moveList);
      if(size == 0){ // Check for end game
         if(pos->flags & IN_CHECK){
            return -(CHECKMATE_VALUE - ply); // Check Mate
         }
         else{
            return 0; // Stalemate
         }
      }
      else{
         return alpha;
      }
   }

   q_evalMoves(moveList, moveVals, size, *pos);
   
   #ifdef DEBUG
   if(size > 0) debug[QS][NODE_LOOP_CHILDREN]++;
   #endif
   Position prevPos = *pos;
   for (i32 i = 0; i < size; i++)  {
      #ifdef DEBUG
      assert(prevPos.hash == pos->hash);
      #endif
      q_select_sort(i, moveList, moveVals, size); 

      //Delta Pruning
      i32 delta = DELTA_VALUE;
      if (GET_FLAGS(moveList[i]) & PROMOTION) delta += (QUEEN_VALUE - PAWN_VALUE);
      if (!(pos->flags & IN_CHECK) && stand_pat + delta + moveVals[i] < alpha && pos->stage != END_GAME) {
         #ifdef DEBUG
         debug[QS][NODE_PRUNED_FUTIL]++;
         #endif
         continue;
      }

      makeMove(pos, moveList[i]);
      i32 score = -q_search(pos, -beta, -alpha, ply + 1, q_ply + 1, stats);
      *pos = prevPos; //Unmake Move

      if( score >= beta ){
         // storeKillerMove(ply, moveList[i]);
         #ifdef DEBUG
         debug[QS][NODE_BETA_CUT]++;
         #endif
         return beta;
      }
      if( score > alpha ){
         alpha = score;
      }
   }

   #ifdef DEBUG
   debug[QS][NODE_ALPHA_RET]++;
   #endif
   return alpha;
}


