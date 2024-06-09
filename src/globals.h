#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
//Flags
extern volatile i32 run_program;
extern volatile i32 run_get_best_move;
extern volatile i32 best_move_found;
extern volatile i32 print_pv_info;

void init_globals();
void free_globals();

void update_global_pv(u32 depth, Move* pvArray, i32 eval, SearchStats stats);

void set_global_position(Position pos);
Position get_global_position();

Move get_global_best_move();

SearchData get_global_pv_data();

#endif // GLOBALS_H