#ifndef evaluate_h
#define evaluate_h

#include "types.h"
int evaluate(Position pos);
void initPST(void);
void evalMoves(Move* moveList, int* moveVals, int size, 
               Move ttMove, Move *killerMoves, int kmv_size, Position pos);


#endif