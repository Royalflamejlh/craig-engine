#ifndef evaluate_h
#define evaluate_h

#include "types.h"

#define MAX_EVAL      9999999

#define KING_VALUE     100000
#define QUEEN_VALUE     10000
#define ROOK_VALUE       5000
#define BISHOP_VALUE     3550
#define KNIGHT_VALUE     3500
#define PAWN_VALUE       1000

i32 evaluate(Position pos);
i32 quickEval(Position pos);
void initPST(void);
void evalMoves(Move* moveList, i32* moveVals, i32 size, Position pos);


#endif