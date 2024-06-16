#include "masks.h"

u64 PassedPawnMask[2][64] = {0};

void init_masks(){

    // Initialize the passed pawn masks
    // This is all the squares 3 wide ahead of a pawn that must be
    // un occupied by enemy pawns to be a past pawn
    u64 mask = 0ULL;
    for(i32 sq = 0; sq < 64; sq++){
        mask = mask << sq;
    }
}



