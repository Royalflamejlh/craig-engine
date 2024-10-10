#ifndef SEARCH_H
#define SEARCH_H
#include "types.h"
void start_search(SearchParameters search);
void search_timed_out(void);
void stopSearch(void);
void exit_search();
i32 enter_loop(ThreadData *td);
#endif