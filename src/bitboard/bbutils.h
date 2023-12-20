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

#ifdef _MSC_VER

#include <stdlib.h>
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)

#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

#include <sys/byteorder.h>
#define bswap_32(x) BSWAP_32(x)
#define bswap_64(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

#include <sys/endian.h>
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)

#elif defined(__OpenBSD__)

#include <sys/types.h>
#define bswap_32(x) swap32(x)
#define bswap_64(x) swap64(x)

#elif defined(__NetBSD__)

#include <sys/types.h>
#include <machine/bswap.h>

#elif defined(_WIN32) || defined(_WIN64)

#include <stdlib.h>
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)

#if defined(__BSWAP_RENAME) && !defined(__bswap_32)
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)
#endif

#else

#include <byteswap.h>

#endif

extern uint64_t betweenMask[64][64];
extern uint64_t rankMask[64], fileMask[64], NESWMask[64], NWSEMask[64];

void generateBetweenMasks();
void generateRankMasks();
void generateFileMasks();
void generateDiagonalMasks();

void printBB(uint64_t BB);
Position fenToPosition(char* FEN);
int PositionToFen(Position pos, char* FEN);
void printPosition(Position position, char verbose);
uint64_t northOne(uint64_t bb);
uint64_t northTwo(uint64_t bb);
uint64_t noEaOne(uint64_t bb);
uint64_t noWeOne(uint64_t bb);

uint64_t southOne(uint64_t bb);
uint64_t southTwo(uint64_t bb);
uint64_t soEaOne(uint64_t bb);
uint64_t soWeOne(uint64_t bb);

static inline uint64_t setBit(uint64_t bb, int square) {
    return bb | (1ULL << square);
}

static inline uint64_t clearBit(uint64_t bb, int square) {
    return bb & ~(1ULL << square);
}

/**
 * Flip a bitboard vertically about the centre ranks.
 * Rank 1 is mapped to rank 8 and vice versa.
 * @param board any bitboard
 * @return bitboard x flipped vertically
 */
static inline uint64_t flipVertical(uint64_t board) {
   return bswap_64(board);
}

#endif /* bbutils_h */


