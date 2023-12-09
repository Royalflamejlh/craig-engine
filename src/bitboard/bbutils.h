//
//  bbutils.h
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#ifndef bbutils_h
#define bbutils_h

#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "../types.h"


extern uint64_t betweenMask[64][64];
extern uint64_t rankMask[64], fileMask[64], NESWMask[64], NWSEMask[64];

void generateBetweenMasks();
void generateRankMasks();
void generateFileMasks();
void generateDiagonalMasks();

void printBB(uint64_t BB);
Position fenToPosition(char* FEN);
uint64_t flipVertical(uint64_t board);
void printPosition(Position position);
uint64_t setBit(uint64_t bb, int square);
uint64_t northOne(uint64_t bb);
uint64_t northTwo(uint64_t bb);
uint64_t noEaOne(uint64_t bb);
uint64_t noWeOne(uint64_t bb);


#endif /* bbutils_h */


