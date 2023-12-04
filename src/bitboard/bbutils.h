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
#include "../types.h"
void printBB(uint64_t BB);
Position fenToPosition(char* FEN);
void printPosition(Position position);

#endif /* bbutils_h */


