#ifndef magic_h
#define magic_h

#include <stdint.h>

int generateMagics(void);
uint64_t bishopAttacks(uint64_t occ, int sq);
uint64_t rookAttacks(uint64_t occ, int sq);
#endif