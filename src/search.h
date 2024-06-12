#ifndef SEARCH_H
#define SEARCH_H
#include "types.h"
void startSearch(u32 time, u32 depth);
void stopSearch();
void exit_search(Position* pos);
i32 searchLoop();
#endif