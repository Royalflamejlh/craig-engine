#ifndef tree_h
#define tree_h
#include "types.h"

i32 search_tree(ThreadData *td);
i32 helper_search_tree(ThreadData *td);

i32 pv_search(ThreadData *td, i32 alpha, i32 beta, i8 depth, u8 ply);
i32 helper_pv_search(ThreadData *td, i32 alpha, i32 beta, i8 depth, u8 ply);
i32 zw_search(ThreadData *td,  i32 beta, i8 depth, u8 ply, u8 isNull);
i32 q_search(ThreadData *td,  i32 alpha, i32 beta, u8 ply, u8 q_ply);

#ifdef DEBUG
extern u8 debug_print_search;
#endif

#endif