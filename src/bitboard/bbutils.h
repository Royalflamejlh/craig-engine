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

extern u64 betweenMask[64][64];
extern u64 rankMask[64], fileMask[64], NESWMask[64], NWSEMask[64];

void generateBetweenMasks();
void generateRankMasks();
void generateFileMasks();
void generateDiagonalMasks();

void printBB(u64 BB);
Position fen_to_position(char* FEN);
i32 PositionToFen(Position pos, char* FEN);
void printPosition(Position position, char verbose);
void printDebug(Position position);
u64 northOne(u64 bb);
u64 northTwo(u64 bb);
u64 noEaOne(u64 bb);
u64 noWeOne(u64 bb);

u64 southOne(u64 bb);
u64 southTwo(u64 bb);
u64 soEaOne(u64 bb);
u64 soWeOne(u64 bb);

static inline u64 setBit(u64 bb, i32 square) {
    return bb | (1ULL << square);
}

static inline u64 clearBit(u64 bb, i32 square) {
    return bb & ~(1ULL << square);
}

/**
 * Flip a bitboard vertically about the centre ranks.
 * Rank 1 is mapped to rank 8 and vice versa.
 * @param board any bitboard
 * @return bitboard x flipped vertically
 */
static inline u64 flipVertical(u64 board) {
   return bswap_64(board);
}

#endif /* bbutils_h */


