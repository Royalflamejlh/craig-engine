#pragma once
#include "types.h"
i32 helper_loop(ThreadData *td);
i32 search_loop(ThreadData *td);
void start_search(SearchParameters search);
void search_timed_out(void);
void stopSearch(void);
void exit_search();