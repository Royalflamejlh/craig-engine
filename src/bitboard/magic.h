#ifndef magic_h
#define magic_h

#include <stdint.h>
#include "../types.h"

i32 generateMagics(void);
u64 bishopAttacks(u64 occ, i32 sq);
u64 rookAttacks(u64 occ, i32 sq);
#endif