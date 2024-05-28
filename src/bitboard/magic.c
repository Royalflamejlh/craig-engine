#include "magic.h"
#include "../util.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <time.h>
#include <string.h>


//static u64 find_magic(i32 sq, i32 m, i32 bishop);
static u64 rmask(i32 sq);
static u64 bmask(i32 sq);
static u64 ratt(i32 sq, u64 block);
static u64 batt(i32 sq, u64 block);
static u64 index_to_uint64(i32 index, i32 bits, u64 m);
static i32 count_1s(u64 b);
static i32 verifyMagic(i32 square, i32 isBishop);
static i32 initAttackTable(void);
static void calculateAttackTableOffsets();
static i32 transform(u64 b, u64 magic, i32 bits);

#ifdef DEBUG
static u64 debug_bishopAttacks(u64 occ, i32 sq);
static u64 debug_rookAttacks(u64 occ, i32 sq);
#endif

static u64 attack_table[108000];
static i32 attack_table_offsets[128];

typedef struct {
    u64* ptr;
    u64 mask;
    u64 magic;
    i32 shift;
} SMagic;

SMagic mBishopTbl[64];
SMagic mRookTbl[64];

static const u64 rook_magics[64] = {
    0x80002080400016ULL,
    0xc0004010002000ULL,
    0x880100008822000ULL,
    0x100090020041000ULL,
    0x8200200200040810ULL,
    0x4100020400010008ULL,
    0x200020004410088ULL,
    0x4080088002244300ULL,
    0x109800c80400025ULL,
    0x402000401000ULL,
    0x4110801000200080ULL,
    0x8800804801000ULL,
    0x4181001008010004ULL,
    0x180800400800200ULL,
    0x2100c100040200ULL,
    0x820801840801100ULL,
    0xa100848000400020ULL,
    0x44b00040004c2004ULL,
    0x3868020001000ULL,
    0x402020010200840ULL,
    0x40828004000800ULL,
    0x808004000200ULL,
    0x1080010100040200ULL,
    0x20004118641ULL,
    0x1080004040002000ULL,
    0xc810014140002002ULL,
    0x88a0100180200088ULL,
    0x8000240900100100ULL,
    0x2a0040080080081ULL,
    0x421002900020400ULL,
    0x400020400080110ULL,
    0x2002040200218047ULL,
    0x40804000800020ULL,
    0x8200044401004ULL,
    0x100200388801000ULL,
    0x640080080801004ULL,
    0x44000800808004ULL,
    0x802040080800200ULL,
    0x129004000148ULL,
    0x200009122000044ULL,
    0x804000218000ULL,
    0x6000c02010014002ULL,
    0x402001010010ULL,
    0x82002008120040ULL,
    0x82002008120004ULL,
    0x414040002008080ULL,
    0x800080102040090ULL,
    0x58000094004a0001ULL,
    0x2000800040002080ULL,
    0x8000200140008280ULL,
    0x2011002004401100ULL,
    0xa80801000080080ULL,
    0xa08000408110100ULL,
    0x4112001400800280ULL,
    0x840100822210400ULL,
    0x10008420410200ULL,
    0x2800150210441ULL,
    0x1204522180400105ULL,
    0x50402001000811ULL,
    0x1450210008041001ULL,
    0x1000204080011ULL,
    0x710008020c0003ULL,
    0x22000088010402ULL,
    0x40001880410c0022ULL
};

static const u64 bishop_magics[64] = {
    0x812210011a040040ULL,
    0x801210e160230ULL,
    0x12080049010200ULL,
    0x400404148000082aULL,
    0x441104196004140ULL,
    0x8982080289120424ULL,
    0x38100a805400020ULL,
    0x6001004a46201004ULL,
    0x821002a80803a0ULL,
    0x4082021042008502ULL,
    0x8000118404004100ULL,
    0x1002244048810840ULL,
    0x900220210002430ULL,
    0x18010422410410ULL,
    0x400404908184019ULL,
    0x1808004100a82080ULL,
    0xc0000810014200ULL,
    0x2008640410088220ULL,
    0x2001004001021ULL,
    0xc8000082024008ULL,
    0x8084022083a00000ULL,
    0x200804100a001ULL,
    0x1504000201014900ULL,
    0xa0020aa20802ULL,
    0x20088005510400ULL,
    0x484e001ca081300ULL,
    0x8032022021040400ULL,
    0x48080000220060ULL,
    0x920020006405001ULL,
    0xc000420001012114ULL,
    0x840000a20800ULL,
    0x9010203d04801ULL,
    0x140202a00090a000ULL,
    0x20008a100418b040ULL,
    0x90108204500400ULL,
    0x4020080880080ULL,
    0x11110400060020ULL,
    0x8100100249040ULL,
    0x610020222209080ULL,
    0x8022040602901ULL,
    0xc242301a8800e000ULL,
    0x200842420022200ULL,
    0x805001082011001ULL,
    0x104200800808ULL,
    0x2001400102104100ULL,
    0x4004009822000840ULL,
    0x5280825040a0155ULL,
    0x121140410802040ULL,
    0x12020120080000ULL,
    0x41140104028000ULL,
    0x8292048a211080ULL,
    0x8050000d08482480ULL,
    0x402a0c0850241004ULL,
    0x10400821210298ULL,
    0x840028404008201ULL,
    0x802084104008010ULL,
    0x2100840490842000ULL,
    0xc014404200900801ULL,
    0x1041002842009000ULL,
    0x21000008842400ULL,
    0x10800009502400ULL,
    0x42002120a02ULL,
    0x2400200244010400ULL,
    0x10200121020401c8ULL
};


const i32 BitTable[64] = {
  63, 30, 3, 32, 25, 41, 22, 33, 15, 50, 42, 13, 11, 53, 19, 34, 61, 29, 2,
  51, 21, 43, 45, 10, 18, 47, 1, 54, 9, 57, 0, 35, 62, 31, 40, 4, 49, 5, 52,
  26, 60, 6, 23, 44, 46, 27, 56, 16, 7, 39, 48, 24, 59, 14, 12, 55, 38, 28,
  58, 20, 37, 17, 36, 8
};

i32 RBits[64] = {
  12, 11, 11, 11, 11, 11, 11, 12,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  11, 10, 10, 10, 10, 10, 10, 11,
  12, 11, 11, 11, 11, 11, 11, 12
};

i32 BBits[64] = {
  6, 5, 5, 5, 5, 5, 5, 6,
  5, 5, 5, 5, 5, 5, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 9, 9, 7, 5, 5,
  5, 5, 7, 7, 7, 7, 5, 5,
  5, 5, 5, 5, 5, 5, 5, 5,
  6, 5, 5, 5, 5, 5, 5, 6
};

i32 generateMagics(void) {
    #ifdef __RAND_SEED
    srand(__RAND_SEED);
    #else
    time_t t;
    srand((unsigned) time(&t));
    #endif

    printf("info string Generating Magics!\n");
    calculateAttackTableOffsets();
    i32 square;
    for(square = 0; square < 64; square++){
        //u64 magic = find_magic(square, RBits[square], 0);
        mRookTbl[square].magic = rook_magics[square];
        mRookTbl[square].mask  = rmask(square);
        mRookTbl[square].shift = 64 - RBits[square];
        mRookTbl[square].ptr = &attack_table[attack_table_offsets[square]];
        //printf("0x%llxULL,\n", magic);
        
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

    printf("info string Finished generating rook magics, generating bishop magics.\n");
    for(square = 0; square < 64; square++){
        //u64 magic = find_magic(square, BBits[square], 1);
        mBishopTbl[square].magic = bishop_magics[square];
        mBishopTbl[square].mask  = bmask(square);
        mBishopTbl[square].shift = 64 - BBits[square];
        mBishopTbl[square].ptr   = &attack_table[attack_table_offsets[square + 64]];
        //printf("0x%llxULL,\n", magic);

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


    for (i32 square = 0; square < 64; ++square) {
        if (verifyMagic(square, 0)) {
            printf("info string Verification failed for rook magic at square %d\n", square);
            return -1;
        }
    }

    printf("info string All rook magics verified successfully\n");

    for (i32 square = 0; square < 64; ++square) {
        if (verifyMagic(square, 1)) {
            printf("info string Verification failed for bishop magic at square %d\n", square);
            return -1;
        }
    }

    printf("info string All bishop magics verified successfully\n");

    return 0;
}

static i32 verifyMagic(i32 square, i32 isBishop) {
    u64 mask = isBishop ? bmask(square) : rmask(square);
    i32 num_blocker_configs = 1 << count_1s(mask);

    for (i32 blockers = 0; blockers < num_blocker_configs; ++blockers) {
        u64 blocker_bits = index_to_uint64(blockers, count_1s(mask), mask);
        u64 expected_attack = isBishop ? batt(square, blocker_bits) : ratt(square, blocker_bits);
        #ifdef DEBUG
        u64 actual_attack = isBishop ? debug_bishopAttacks(blocker_bits, square) : debug_rookAttacks(blocker_bits, square);
        #else
        u64 actual_attack = isBishop ? bishopAttacks(blocker_bits, square) : rookAttacks(blocker_bits, square);
        #endif

        if (actual_attack != expected_attack) {
            printf("info string Failed with expected attack: 0x%llx, actual attack was: 0x%llx\n",
                (long long)expected_attack,
                (long long)actual_attack);
            return -1;
        }
    }

    return 0;
}


static i32 initAttackTable() {
    printf("info string Initializing the attack table!\n");

    // Rooks
    for (i32 sq = 0; sq < 64; ++sq) {
        u64 mask = rmask(sq);
        i32 num_blocker_configs = 1 << count_1s(mask);
        for (i32 blockers = 0; blockers < num_blocker_configs; ++blockers) {
            u64 blocker_bits = index_to_uint64(blockers, count_1s(mask), mask);
            u64 index = transform(blocker_bits, mRookTbl[sq].magic, RBits[sq]);
            attack_table[attack_table_offsets[sq] + index] = ratt(sq, blocker_bits);
        }
    }

    // Bishops
    for (i32 sq = 0; sq < 64; ++sq) {
        u64 mask = bmask(sq);
        i32 num_blocker_configs = 1 << count_1s(mask);
        for (i32 blockers = 0; blockers < num_blocker_configs; ++blockers) {
            u64 blocker_bits = index_to_uint64(blockers, count_1s(mask), mask);
            u64 index = transform(blocker_bits, mBishopTbl[sq].magic, BBits[sq]);
            attack_table[attack_table_offsets[sq + 64] + index] = batt(sq, blocker_bits);
        }
    }


    return 0;
}

static void calculateAttackTableOffsets() {
    i32 offset = 0;

    for (i32 sq = 0; sq < 64; ++sq) {
        u64 mask = rmask(sq);
        i32 num_blocker_configs = 1 << count_1s(mask);
        attack_table_offsets[sq] = offset;
        offset += num_blocker_configs;

        mask = bmask(sq);
        num_blocker_configs = 1 << count_1s(mask);
        attack_table_offsets[sq + 64] = offset;
        offset += num_blocker_configs;
    }
}


u64 bishopAttacks(u64 occ, i32 sq) {
   u64* aptr = mBishopTbl[sq].ptr;
   occ           &= mBishopTbl[sq].mask;
   occ           *= mBishopTbl[sq].magic;
   occ          >>= mBishopTbl[sq].shift;
   return aptr[occ];
}

u64 rookAttacks(u64 occ, i32 sq) {
   u64* aptr = mRookTbl[sq].ptr;
   occ           &= mRookTbl[sq].mask;
   occ           *= mRookTbl[sq].magic;
   occ          >>= mRookTbl[sq].shift;
   return aptr[occ];
}

#ifdef DEBUG
static u64 debug_bishopAttacks(u64 occ, i32 sq) {
    u64* aptr = mBishopTbl[sq].ptr;
    u64 original_occ = occ;
    occ &= mBishopTbl[sq].mask;
    u64 masked_occ = occ;
    occ *= mBishopTbl[sq].magic;
    u64 multiplied_occ = occ;
    occ >>= mBishopTbl[sq].shift;
    u64 index = occ;

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

static u64 debug_rookAttacks(u64 occ, i32 sq) {
    u64* aptr = mRookTbl[sq].ptr;
    u64 original_occ = occ;
    occ &= mRookTbl[sq].mask;
    u64 masked_occ = occ;
    occ *= mRookTbl[sq].magic;
    u64 multiplied_occ = occ;
    occ >>= mRookTbl[sq].shift;
    u64 index = occ;

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

u64 random_uint64_fewbits() {
  return random_uint64() & random_uint64() & random_uint64();
}

static i32 count_1s(u64 b) {
  return __builtin_popcountll(b);
  // i32 r;
  // for(r = 0; b; r++, b &= b - 1);
  // return r;
}

i32 pop_1st_bit(u64 *bb) {
  u64 b = *bb ^ (*bb - 1);
  u32 fold = (unsigned) ((b & 0xffffffff) ^ (b >> 32));
  *bb &= (*bb - 1);
  return BitTable[(fold * 0x783a9b23) >> 26];
}

static u64 index_to_uint64(i32 index, i32 bits, u64 m) {
  i32 i, j;
  u64 result = 0ULL;
  for(i = 0; i < bits; i++) {
    j = pop_1st_bit(&m);
    if(index & (1ULL << i)) result |= (1ULL << j);
  }
  return result;
}

static u64 rmask(i32 sq) {
  u64 result = 0ULL;
  i32 rk = sq/8, fl = sq%8, r, f;
  for(r = rk+1; r <= 6; r++) result |= (1ULL << (fl + r*8));
  for(r = rk-1; r >= 1; r--) result |= (1ULL << (fl + r*8));
  for(f = fl+1; f <= 6; f++) result |= (1ULL << (f + rk*8));
  for(f = fl-1; f >= 1; f--) result |= (1ULL << (f + rk*8));
  return result;
}

static u64 bmask(i32 sq) {
  u64 result = 0ULL;
  i32 rk = sq/8, fl = sq%8, r, f;
  for(r=rk+1, f=fl+1; r<=6 && f<=6; r++, f++) result |= (1ULL << (f + r*8));
  for(r=rk+1, f=fl-1; r<=6 && f>=1; r++, f--) result |= (1ULL << (f + r*8));
  for(r=rk-1, f=fl+1; r>=1 && f<=6; r--, f++) result |= (1ULL << (f + r*8));
  for(r=rk-1, f=fl-1; r>=1 && f>=1; r--, f--) result |= (1ULL << (f + r*8));
  return result;
}

static u64 ratt(i32 sq, u64 block) {
  u64 result = 0ULL;
  i32 rk = sq/8, fl = sq%8, r, f;
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

static u64 batt(i32 sq, u64 block) {
  u64 result = 0ULL;
  i32 rk = sq/8, fl = sq%8, r, f;
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


static i32 transform(u64 b, u64 magic, i32 bits) {
  return (i32)((b * magic) >> (64 - bits));
}

/*
static u64 find_magic(i32 sq, i32 m, i32 bishop) {
  u64 mask, b[4096], a[4096], magic;
  i32 i, j, k, n, fail;

  mask = bishop? bmask(sq) : rmask(sq);
  n = count_1s(mask);

  for(i = 0; i < (1 << n); i++) {
    b[i] = index_to_uint64(i, n, mask);
    a[i] = bishop? batt(sq, b[i]) : ratt(sq, b[i]);
  }
  for(k = 0; k < 100000000; k++) {
    magic = random_uint64_fewbits();
    if(count_1s((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
    //for(i = 0; i < 4096; i++) used[i] = 0ULL;
    u64 used[4096] = {0ULL};
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
*/

