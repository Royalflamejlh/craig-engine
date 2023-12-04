#include "magic.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>


static uint64_t find_magic(int sq, int m, int bishop);
static uint64_t rmask(int sq);
static uint64_t bmask(int sq);
static uint64_t ratt(int sq, uint64_t block);
static uint64_t batt(int sq, uint64_t block);
static uint64_t index_to_uint64(int index, int bits, uint64_t m);
static int count_1s(uint64_t b);
static int verifyMagic(int square, int isBishop);
static int initAttackTable(void);
static void calculateAttackTableOffsets();
static int transform(uint64_t b, uint64_t magic, int bits);

#ifdef DEBUG
static uint64_t debug_bishopAttacks(uint64_t occ, int sq);
static uint64_t debug_rookAttacks(uint64_t occ, int sq);
#endif

static uint64_t attack_table[108000];
static int attack_table_offsets[128];

typedef struct {
    uint64_t* ptr;
    uint64_t mask;
    uint64_t magic;
    int shift;
} SMagic;

SMagic mBishopTbl[64];
SMagic mRookTbl[64];

const int BitTable[64] = {
  63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
  51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
  26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
  58, 20, 37, 17, 36, 8
};

int RBits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

int BBits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

int generateMagics(void) {
    time_t t;
    srand((unsigned) time(&t));

    printf("Generating Magics!\n");
    calculateAttackTableOffsets();
    int square;

    for(square = 0; square < 64; square++){
        mRookTbl[square].magic = find_magic(square, RBits[square], 0);
        mRookTbl[square].mask  = rmask(square);
        mRookTbl[square].shift = 64 - RBits[square];
        mRookTbl[square].ptr = &attack_table[attack_table_offsets[square]];
        
        #ifdef DEBUG
        printf("Rook Square %d: Magic = 0x%" PRIx64 ", Mask = 0x%" PRIx64 ", Shift = %d, AttackTableIdx = %d, Ptr = %p\n",
            square,
            mRookTbl[square].magic,
            mRookTbl[square].mask,
            mRookTbl[square].shift,
            attack_table_offsets[square],
            (void*)mRookTbl[square].ptr);
        #endif
    }

    printf("Finished generating rook magics, generating bishop magics.\n");

    for(square = 0; square < 64; square++){
        mBishopTbl[square].magic = find_magic(square, BBits[square], 1);
        mBishopTbl[square].mask  = bmask(square);
        mBishopTbl[square].shift = 64 - BBits[square];
        mBishopTbl[square].ptr   = &attack_table[attack_table_offsets[square + 64]];
        
        #ifdef DEBUG
        printf("Bishop Square %d: Magic = 0x%" PRIx64 ", Mask = 0x%" PRIx64 ", Shift = %d, AttackTableIdx = %d, Ptr = %p\n",
            square,
            mBishopTbl[square].magic,
            mBishopTbl[square].mask,
            mBishopTbl[square].shift,
            attack_table_offsets[square + 64],
            (void*)mBishopTbl[square].ptr);
        #endif
    }

    
    initAttackTable();


    for (int square = 0; square < 64; ++square) {
        if (verifyMagic(square, 0)) {
            printf("Verification failed for rook magic at square %d\n", square);
            return -1;
        }
    }

    printf("All rook magics verified successfully\n");

    for (int square = 0; square < 64; ++square) {
        if (verifyMagic(square, 1)) {
            printf("Verification failed for bishop magic at square %d\n", square);
            return -1;
        }
    }

    printf("All bishop magics verified successfully\n");

    return 0;
}

static int verifyMagic(int square, int isBishop) {
    uint64_t mask = isBishop ? bmask(square) : rmask(square);
    int num_blocker_configs = 1 << count_1s(mask);

    for (int blockers = 0; blockers < num_blocker_configs; ++blockers) {
        uint64_t blocker_bits = index_to_uint64(blockers, count_1s(mask), mask);
        uint64_t expected_attack = isBishop ? batt(square, blocker_bits) : ratt(square, blocker_bits);
        #ifdef DEBUG
        uint64_t actual_attack = isBishop ? debug_bishopAttacks(blocker_bits, square) : debug_rookAttacks(blocker_bits, square);
        #else
        uint64_t actual_attack = isBishop ? bishopAttacks(blocker_bits, square) : rookAttacks(blocker_bits, square);
        #endif

        if (actual_attack != expected_attack) {
            printf("Failed with expected attack: 0x%llx, actual attack was: 0x%llx\n",
                (unsigned long long)expected_attack,
                (unsigned long long)actual_attack);
            return -1;
        }
    }

    return 0;
}


static int initAttackTable() {
    printf("Initializing the attack table!\n");

    // Rooks
    for (int sq = 0; sq < 64; ++sq) {
        uint64_t mask = rmask(sq);
        int num_blocker_configs = 1 << count_1s(mask);
        for (int blockers = 0; blockers < num_blocker_configs; ++blockers) {
            uint64_t blocker_bits = index_to_uint64(blockers, count_1s(mask), mask);
            uint64_t index = transform(blocker_bits, mRookTbl[sq].magic, RBits[sq]);
            attack_table[attack_table_offsets[sq] + index] = ratt(sq, blocker_bits);
        }
    }

    // Bishops
    for (int sq = 0; sq < 64; ++sq) {
        uint64_t mask = bmask(sq);
        int num_blocker_configs = 1 << count_1s(mask);
        for (int blockers = 0; blockers < num_blocker_configs; ++blockers) {
            uint64_t blocker_bits = index_to_uint64(blockers, count_1s(mask), mask);
            uint64_t index = transform(blocker_bits, mBishopTbl[sq].magic, BBits[sq]);
            attack_table[attack_table_offsets[sq + 64] + index] = batt(sq, blocker_bits);
        }
    }


    return 0;
}

static void calculateAttackTableOffsets() {
    int offset = 0;

    for (int sq = 0; sq < 64; ++sq) {
        uint64_t mask = rmask(sq);
        int num_blocker_configs = 1 << count_1s(mask);
        attack_table_offsets[sq] = offset;
        offset += num_blocker_configs;

        mask = bmask(sq);
        num_blocker_configs = 1 << count_1s(mask);
        attack_table_offsets[sq + 64] = offset;
        offset += num_blocker_configs;
    }
    printf("final offset is: %d", offset);
}


uint64_t bishopAttacks(uint64_t occ, int sq) {
   uint64_t* aptr = mBishopTbl[sq].ptr;
   occ           &= mBishopTbl[sq].mask;
   occ           *= mBishopTbl[sq].magic;
   occ          >>= mBishopTbl[sq].shift;
   return aptr[occ];
}

uint64_t rookAttacks(uint64_t occ, int sq) {
   uint64_t* aptr = mRookTbl[sq].ptr;
   occ           &= mRookTbl[sq].mask;
   occ           *= mRookTbl[sq].magic;
   occ          >>= mRookTbl[sq].shift;
   return aptr[occ];
}

#ifdef DEBUG
static uint64_t debug_bishopAttacks(uint64_t occ, int sq) {
    uint64_t* aptr = mBishopTbl[sq].ptr;
    uint64_t original_occ = occ;
    occ &= mBishopTbl[sq].mask;
    uint64_t masked_occ = occ;
    occ *= mBishopTbl[sq].magic;
    uint64_t multiplied_occ = occ;
    occ >>= mBishopTbl[sq].shift;
    uint64_t index = occ;

    printf("Bishop Debug: %d: Magic = 0x%" PRIx64 ", Mask = 0x%" PRIx64 ", Shift = %d, Ptr = %p, Original Occupancy: 0x%" PRIx64 ", Masked Occupancy: 0x%" PRIx64 ", Multiplied Occupancy: 0x%" PRIx64 ", Index: 0x%" PRIx64 "\n",
            sq,
            mBishopTbl[sq].magic,
            mBishopTbl[sq].mask,
            mBishopTbl[sq].shift,
            (void*)mBishopTbl[sq].ptr,
            original_occ,
            masked_occ,
            multiplied_occ,
            index);
    return aptr[index];
}

static uint64_t debug_rookAttacks(uint64_t occ, int sq) {
    uint64_t* aptr = mRookTbl[sq].ptr;
    uint64_t original_occ = occ;
    occ &= mRookTbl[sq].mask;
    uint64_t masked_occ = occ;
    occ *= mRookTbl[sq].magic;
    uint64_t multiplied_occ = occ;
    occ >>= mRookTbl[sq].shift;
    uint64_t index = occ;

    printf("Rook Debug: %d: Magic = 0x%" PRIx64 ", Mask = 0x%" PRIx64 ", Shift = %d, Ptr = %p, Original Occupancy: 0x%" PRIx64 ", Masked Occupancy: 0x%" PRIx64 ", Multiplied Occupancy: 0x%" PRIx64 ", Index: 0x%" PRIx64 "\n",
            sq,
            mRookTbl[sq].magic,
            mRookTbl[sq].mask,
            mRookTbl[sq].shift,
            (void*)mRookTbl[sq].ptr,
            original_occ,
            masked_occ,
            multiplied_occ,
            index);
    return aptr[index];
}
#endif

uint64_t random_uint64() {
  uint64_t u1, u2, u3, u4;
  u1 = (uint64_t)(rand()) & 0xFFFF; u2 = (uint64_t)(rand()) & 0xFFFF;
  u3 = (uint64_t)(rand()) & 0xFFFF; u4 = (uint64_t)(rand()) & 0xFFFF;
  return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64_t random_uint64_fewbits() {
  return random_uint64() & random_uint64() & random_uint64();
}

static int count_1s(uint64_t b) {
  int r;
  for(r = 0; b; r++, b &= b - 1);
  return r;
}

int pop_1st_bit(uint64_t *bb) {
  uint64_t b = *bb ^ (*bb - 1);
  unsigned int fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return BitTable[(fold * 0x783a9b23) >> 26];
}

static uint64_t index_to_uint64(int index, int bits, uint64_t m) {
  int i, j;
  uint64_t result = 0ULL;
  for(i = 0; i < bits; i++) {
    j = pop_1st_bit(&m);
    if(index & (1 << i)) result |= (1ULL << j);
  }
  return result;
}

static uint64_t rmask(int sq) {
  uint64_t result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r = rk+1; r <= 6; r++) result |= (1ULL << (fl + r*8));
  for(r = rk-1; r >= 1; r--) result |= (1ULL << (fl + r*8));
  for(f = fl+1; f <= 6; f++) result |= (1ULL << (f + rk*8));
  for(f = fl-1; f >= 1; f--) result |= (1ULL << (f + rk*8));
  return result;
}

static uint64_t bmask(int sq) {
  uint64_t result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
  for(r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
  for(r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
  for(r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
  return result;
}

static uint64_t ratt(int sq, uint64_t block) {
  uint64_t result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r = rk+1; r <= 7; r++) {
    result |= (1ULL << (fl + r*8));
    if(block & (1ULL << (fl + r*8))) break;
  }
  for(r = rk-1; r >= 0; r--) {
    result |= (1ULL << (fl + r*8));
    if(block & (1ULL << (fl + r*8))) break;
  }
  for(f = fl+1; f <= 7; f++) {
    result |= (1ULL << (f + rk*8));
    if(block & (1ULL << (f + rk*8))) break;
  }
  for(f = fl-1; f >= 0; f--) {
    result |= (1ULL << (f + rk*8));
    if(block & (1ULL << (f + rk*8))) break;
  }
  return result;
}

static uint64_t batt(int sq, uint64_t block) {
  uint64_t result = 0ULL;
  int rk = sq/8, fl = sq%8, r, f;
  for(r = rk+1, f = fl+1; r <= 7 && f <= 7; r++, f++) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  for(r = rk+1, f = fl-1; r <= 7 && f >= 0; r++, f--) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  for(r = rk-1, f = fl+1; r >= 0 && f <= 7; r--, f++) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  for(r = rk-1, f = fl-1; r >= 0 && f >= 0; r--, f--) {
    result |= (1ULL << (f + r*8));
    if(block & (1ULL << (f + r * 8))) break;
  }
  return result;
}


static int transform(uint64_t b, uint64_t magic, int bits) {
  return (int)((b * magic) >> (64 - bits));
}

static uint64_t find_magic(int sq, int m, int bishop) {
  uint64_t mask, b[4096], a[4096], used[4096], magic;
  int i, j, k, n, fail;

  mask = bishop? bmask(sq) : rmask(sq);
  n = count_1s(mask);

  for(i = 0; i < (1 << n); i++) {
    b[i] = index_to_uint64(i, n, mask);
    a[i] = bishop? batt(sq, b[i]) : ratt(sq, b[i]);
  }
  for(k = 0; k < 100000000; k++) {
    magic = random_uint64_fewbits();
    if(count_1s((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
    for(i = 0; i < 4096; i++) used[i] = 0ULL;
    for(i = 0, fail = 0; !fail && i < (1 << n); i++) {
      j = transform(b[i], magic, m);
      if(used[j] == 0ULL) used[j] = a[i];
      else if(used[j] != a[i]) fail = 1;
    }
    if(!fail) return magic;
  }
  printf("***Failed***\n");
  return 0ULL;
}



