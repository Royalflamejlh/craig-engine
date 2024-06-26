#ifndef SEARCH_H
#define SEARCH_H
#include "types.h"
void start_search(SearchParameters search);
void search_timed_out(void);
void stopSearch(void);
void exit_search(Position* pos);
i32 search_loop(u32 thread_num);
#endif