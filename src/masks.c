#include "masks.h"
#include "bitboard/bbutils.h"
#include "bitboard/bitboard.h"
#include "types.h"
#include "util.h"

u64 PassedPawnMask[2][64] = {0};

u64 KnightOutpostMask[2] = {0};

u64 KingAreaMask[64] = {0};

u64 BishopOutpostMask[2] = {0};

void init_masks(){

    // Passed Pawn Mask
    //     0 0 0
    //     0 0 0
    //       p 
    for(Turn turn = 0; turn < 2; turn++){
        for(Square sq = A1; sq < H8; sq++){
            PassedPawnMask[turn][sq] = 0;
            i32 file = sq % 8;
            i32 rank = sq / 8;
            rank = turn ? MIN(7, rank) : MAX(0, rank);
            for(i32 i = MAX(0, file-1); i <= MIN(7, file+1); i++){
                u32  promo_sq = turn ? A8 + i : A1 + i;
                u32 bottom_sq = (rank*8) + i;
                PassedPawnMask[turn][sq] |= betweenMask[bottom_sq][promo_sq];
            }
        }
    }

    // Knight Outpost Mask
    KnightOutpostMask[WHITE_TURN] = 0x00007E7E7E000000;
    KnightOutpostMask[BLACK_TURN] = 0x0000007E7E7E0000;

    // King Area Mask
    //      0
    //    0 0 0
    //  0 0 0 0 0
    //    0 0 0
    //      0
    for(Square sq = A1; sq < H8; sq++){
        u64 mask = 1ULL << sq;
        mask |= kingAttacks(sq);
        mask |= northTwo(1ULL << sq);
        mask |= southTwo(1ULL << sq);
        mask |= westTwo(1ULL << sq);
        mask |= eastTwo(1ULL << sq);
        KingAreaMask[sq] = mask;
    }

    // Bishop Outpost Mask
    BishopOutpostMask[WHITE_TURN] = 0x00007E7E7E000000;
    BishopOutpostMask[BLACK_TURN] = 0x0000007E7E7E0000;
}



