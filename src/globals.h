#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include <stdatomic.h>

//Flags
extern _Atomic volatile i32 run_program;
extern _Atomic volatile i32 run_get_best_move;
extern _Atomic volatile i32 best_move_found;

//Print Signals
extern _Atomic volatile i32 print_pv_info;
extern _Atomic volatile i32 print_best_move;

void init_globals();
void free_globals();

u8 update_global_pv(u32 depth, Move* pv_array, i32 eval, SearchStats stats);

void set_global_position(Position pos);
Position get_global_position();
Position copy_global_position();

Move get_global_best_move();

SearchData get_global_pv_data();

#endif // GLOBALS_H