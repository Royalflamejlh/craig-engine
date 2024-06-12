#ifndef tree_h
#define tree_h
#include "types.h"


i32 search_tree(Position pos, u32 depth, Move *pvArray, KillerMoves* km, i32 eval_prev, SearchStats* stats);
i32 helper_search_tree(Position pos, u32 depth, Move *pvArray, KillerMoves* km, i32 eval, SearchStats* stats, u32 thread_num);

i32 pv_search( Position* pos, i32 alpha, i32 beta, i8 depth, u8 ply, Move* pvArray, KillerMoves* km, SearchStats* stats);
i32 helper_pv_search( Position* pos, i32 alpha, i32 beta, i8 depth, u8 ply, Move* pvArray, KillerMoves* km, SearchStats* stats, u32 thread_num);
i32 zw_search( Position* pos, i32 beta, i8 depth, u8 ply, KillerMoves* km, SearchStats* stats, u8 isNull);
i32 q_search( Position* pos, i32 alpha, i32 beta, u8 ply, u8 q_ply, SearchStats* stats);

#endif