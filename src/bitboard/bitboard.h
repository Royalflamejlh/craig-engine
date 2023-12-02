//
//  bitboard.h
//  godengine
//
//  Created by John Howard on 11/22/23.
//

#ifndef bitboard_h
#define bitboard_h
#include <stdint.h>
#include <stdio.h>
#include "bbutils.h"
#include "../types.h"
void generateMasks(void);
void testBB(void);
void printBB(uint64_t BB);
position getInitialBB(void);
uint64_t getKnightMoves(uint64_t knights, uint64_t ownPieces);

#endif /* bitboard_h */
