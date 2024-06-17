#pragma once

#include "types.h"

extern u64 PassedPawnMask[2][64];

extern u64 KnightOutpostMask[2];

extern u64 KingAreaMask[64];

extern u64 BishopOutpostMask[2];

void init_masks();

