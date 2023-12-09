//
//  bbutils.c
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#include "bbutils.h"
#include "../movement.h"
#include "bitboard.h"
#include <string.h>
#include <stdlib.h>


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

uint64_t betweenMask[64][64];
uint64_t rankMask[64], fileMask[64], NESWMask[64], NWSEMask[64];

static void updateBit(uint64_t* bitboard, int square) {
    *bitboard |= (1ULL << square);
}

static void setAttackMasks(Position *pos);

Position fenToPosition(char* FEN) {
    Position pos = {0};
    memset(pos.charBoard, 0, sizeof(pos.charBoard));
    int square = 56; // Start at A8

    while (*FEN && *FEN != ' ') {
        if (*FEN == '/') {
            square -= 16; // Move to the start of the next rank below
        } else if (*FEN >= '1' && *FEN <= '8') {
            square += *FEN - '0'; // Skip empty squares
        } else {
            pos.charBoard[square] = *FEN;
            if(isupper(*FEN)) updateBit(&pos.white, square);
            else updateBit(&pos.black, square);
            switch (*FEN) {
                case 'P': updateBit(&pos.w_pawn, square); break;
                case 'N': updateBit(&pos.w_knight, square); break;
                case 'B': updateBit(&pos.w_bishop, square); break;
                case 'R': updateBit(&pos.w_rook, square); break;
                case 'Q': updateBit(&pos.w_queen, square); break;
                case 'K': updateBit(&pos.w_king, square); break;
                case 'p': updateBit(&pos.b_pawn, square); break;
                case 'n': updateBit(&pos.b_knight, square); break;
                case 'b': updateBit(&pos.b_bishop, square); break;
                case 'r': updateBit(&pos.b_rook, square); break;
                case 'q': updateBit(&pos.b_queen, square); break;
                case 'k': updateBit(&pos.b_king, square); break;
            }
            square++;
        }
        FEN++;
    }

    while (*FEN && *FEN != ' ') FEN++;
    if (*FEN == ' ') FEN++;

    // Player to move
    pos.flags = 0;
    if (*FEN == 'w') pos.flags |= WHITE_TURN;
    else if (*FEN == 'b') pos.flags &= ~WHITE_TURN;
    FEN += 2;

    // Castling availability
    while (*FEN && *FEN != ' ') {
        switch (*FEN) {
            case 'K': pos.flags |= W_SHORT_CASTLE; break;
            case 'Q': pos.flags |= W_LONG_CASTLE; break;
            case 'k': pos.flags |= B_SHORT_CASTLE; break;
            case 'q': pos.flags |= B_LONG_CASTLE; break;
            case '-': break;
        }
        FEN++;
    }
    FEN++;

    // En passant target square
    pos.en_passant = 0;
    if (*FEN != '-') {
        int file = *FEN - 'a'; // Convert file character to 0-7
        int rank = *(FEN + 1) - '1'; // Convert rank character to 0-7
        int en_passant_square = rank * 8 + file;
        updateBit(&pos.en_passant, en_passant_square);
    }

    while (*FEN && *FEN != ' ') FEN++;
    if (*FEN == ' ') FEN++;

    sscanf(FEN, "%d", &pos.halfmove_clock);

    while (*FEN && *FEN != ' ') FEN++;
    if (*FEN == ' ') FEN++;

    sscanf(FEN, "%d", &pos.fullmove_number);

    setAttackMasks(&pos);

    //Check Flag
    if(pos.b_attack_mask & pos.w_king) pos.flags |= IN_CHECK;
    if(pos.w_attack_mask & pos.b_king) pos.flags |= IN_CHECK;

    //Double Check Flag
    if(pos.flags & IN_CHECK){
       int kign_sq = __builtin_ctzll(pos.w_king);
       uint64_t attackers = getBlackAttackers(pos, kign_sq);
       attackers &= attackers - 1;
       if(attackers) pos.flags |= IN_D_CHECK;

       kign_sq = __builtin_ctzll(pos.b_king);
       attackers = getWhiteAttackers(pos, kign_sq);
       attackers &= attackers - 1;
       if(attackers) pos.flags |= IN_D_CHECK;
    }

    pos.pinned = generatePinnedPieces(pos);

    return pos;
}

void generateBetweenMasks() {
    for (int sq1 = 0; sq1 < 64; sq1++) {
        int rank1 = sq1 / 8;
        int file1 = sq1 % 8;

        for (int sq2 = 0; sq2 < 64; sq2++) {
            int rank2 = sq2 / 8;
            int file2 = sq2 % 8;

            if (sq1 == sq2) {
                betweenMask[sq1][sq2] = 0;
                continue;
            }

            uint64_t mask = 0;
            
            if (rank1 == rank2) {
                for (int f = 1; f < abs(file2 - file1); f++) {
                    int file = file1 + f * ((file2 > file1) ? 1 : -1);
                    mask |= 1ULL << (rank1 * 8 + file);
                }
            } else if (file1 == file2) { 
                for (int r = 1; r < abs(rank2 - rank1); r++) {
                    int rank = rank1 + r * ((rank2 > rank1) ? 1 : -1);
                    mask |= 1ULL << (rank * 8 + file1);
                }
            } else if (abs(rank1 - rank2) == abs(file1 - file2)) { 
                for (int i = 1; i < abs(rank2 - rank1); i++) {
                    int rank = rank1 + i * ((rank2 > rank1) ? 1 : -1);
                    int file = file1 + i * ((file2 > file1) ? 1 : -1);
                    mask |= 1ULL << (rank * 8 + file);
                }
            }

            betweenMask[sq1][sq2] = mask;
        }
    }
}

void generateRankMasks() {
    for (int sq = 0; sq < 64; ++sq) {
        int rank = sq / 8;
        rankMask[sq] = 0xFFULL << (rank * 8);
    }
}

void generateFileMasks() {
    for (int sq = 0; sq < 64; ++sq) {
        int file = sq % 8;
        fileMask[sq] = 0x0101010101010101ULL << file;
    }
}

void generateDiagonalMasks() {
    for (int sq = 0; sq < 64; ++sq) {
        int rank = sq / 8;
        int file = sq % 8;
        NESWMask[sq] = 0;
        NWSEMask[sq] = 0;

        // Northeast-Southwest Diagonal
        for (int r = rank, f = file; r < 8 && f < 8; ++r, ++f)
            NESWMask[sq] |= (1ULL << (r * 8 + f));
        for (int r = rank, f = file; r >= 0 && f >= 0; --r, --f)
            NESWMask[sq] |= (1ULL << (r * 8 + f));

        // Northwest-Southeast Diagonal
        for (int r = rank, f = file; r < 8 && f >= 0; ++r, --f)
            NWSEMask[sq] |= (1ULL << (r * 8 + f));
        for (int r = rank, f = file; r >= 0 && f < 8; --r, ++f)
            NWSEMask[sq] |= (1ULL << (r * 8 + f));
    }
}


void printPosition(Position position){
    printf("-------------------------------------------------------------------------\n");
    printf("  A B C D E F G H\n");
    for (int rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            uint64_t mask = 1ULL << square;

            if (position.w_pawn & mask) printf("P ");
            else if (position.w_knight & mask) printf("N ");
            else if (position.w_bishop & mask) printf("B ");
            else if (position.w_rook & mask) printf("R ");
            else if (position.w_queen & mask) printf("Q ");
            else if (position.w_king & mask) printf("K ");
            else if (position.b_pawn & mask) printf("p ");
            else if (position.b_knight & mask) printf("n ");
            else if (position.b_bishop & mask) printf("b ");
            else if (position.b_rook & mask) printf("r ");
            else if (position.b_queen & mask) printf("q ");
            else if (position.b_king & mask) printf("k ");
            else if (position.en_passant & mask) printf("E ");
            else printf(". ");

            
            if (file == 7){
                printf("%d   |  ", rank + 1);
                if(rank == 7) printf("Current Turn: %s", (position.flags & WHITE_TURN) ? "White" : "Black");
                if(rank == 5) printf("Halfmove Clock: %d -- Fullmove Number: %d", position.halfmove_clock, position.fullmove_number);
                if(rank == 3) printf("In Check: %s -- In Double-Check: %s", (position.flags & IN_CHECK) ? "Yes" : "No", (position.flags & IN_D_CHECK) ? "Yes" : "No");
                if(rank == 1) printf("Castling Availability: ");
                if(rank == 0){
                    printf("W-Long: %s, ", (position.flags & W_LONG_CASTLE)   ? "Yes" : "No");
                    printf("W-Short: %s, ", (position.flags & W_SHORT_CASTLE) ? "Yes" : "No");
                    printf("B-Long: %s, ", (position.flags & B_LONG_CASTLE)   ? "Yes" : "No");
                    printf("B-Short: %s", (position.flags & B_SHORT_CASTLE)   ? "Yes" : "No");
                }
                printf("\n");
            }
        }
    }
    printf("  A B C D E F G H\n");
    printf("------------------------------------------------------------------------------------------------------------\n");
    printf("  Color Bitboard   |      White Attack   |      Black Attack   |     Pinned Pieces   |       Char Board    |\n");
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    for (int rank = 7; rank >= 0; rank--) {
        //Color BB
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            uint64_t mask = 1ULL << square;

            if (position.white & mask) printf("W ");
            else if (position.black & mask) printf("b ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //White Attack
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            uint64_t mask = 1ULL << square;

            if (position.w_attack_mask & mask) printf("A ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black Attack
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            uint64_t mask = 1ULL << square;

            if (position.b_attack_mask & mask) printf("a ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Pinned Pieces
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            uint64_t mask = 1ULL << square;

            if (position.pinned & mask) printf("P ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Char Board
        printf("%d ", rank + 1);
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if (position.charBoard[square]) printf("%c ",position.charBoard[square]);
            else printf(". ");
            
            if (file == 7) printf(" |\n");
        }
    }
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    printf("------------------------------------------------------------------------------------------------------------\n");
}


void printBB(uint64_t BB) {
    printf("  A B C D E F G H\n");
    for (int rank = 8; rank >= 1; rank--) {
        printf("%d ", rank);
        for (int file = 0; file < 8; file++) {
            int bitPosition = (rank - 1) * 8 + file;
            if (BB & (1ULL << bitPosition)) {
                printf("1 ");
            } else {
                printf("0 ");
            }
        }
        printf("\n");
    }
}

uint64_t northOne(uint64_t bb) { return bb << 8;  }
uint64_t northTwo(uint64_t bb) { return bb << 16; }
uint64_t noEaOne (uint64_t bb) { return (bb & ~0x8080808080808080ULL) << 9; }
uint64_t noWeOne (uint64_t bb) { return (bb & ~0x0101010101010101ULL) << 7; }


uint64_t setBit(uint64_t bb, int square) {
    return bb | (1ULL << square);
}


static void setAttackMasks(Position *pos){
    pos->w_attack_mask = generateWhiteAttacks(*pos);
    pos->b_attack_mask = generateBlackAttacks(*pos);
}

/**
 * Flip a bitboard vertically about the centre ranks.
 * Rank 1 is mapped to rank 8 and vice versa.
 * @param board any bitboard
 * @return bitboard x flipped vertically
 */
uint64_t flipVertical(uint64_t board) {
   return bswap_64(board);
}


