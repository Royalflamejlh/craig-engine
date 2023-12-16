#ifndef tree_h
#define tree_h
#include "types.h"

Move getBestMove(Position pos);
int pvSearch( Position* pos, int alpha, int beta, int depth );
int zwSearch( Position* pos, int beta, int depth );
int quiesce( Position* pos, int alpha, int beta );

#endif