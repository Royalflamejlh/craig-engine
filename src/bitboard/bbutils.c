//
//  bbutils.c
//  godengine
//
//  Created by John Howard on 11/23/23.
//

#include "bbutils.h"
#include "../movement.h"
#include "bitboard.h"
#include "../evaluator.h"
#include "../hash.h"
#include "../util.h"
#include <string.h>
#include <stdlib.h>

u64 betweenMask[64][64];
u64 rankMask[64], fileMask[64], NESWMask[64], NWSEMask[64];

static void updateBit(u64* bitboard, i32 square) {
    *bitboard |= (1ULL << square);
}

Position fen_to_position(char* FEN) {
    Position pos = {0};
    memset(pos.charBoard, 0, sizeof(pos.charBoard));
    i32 square = 56; // Start at A8

    while (*FEN && *FEN != ' ') {
        if (*FEN == '/') {
            square -= 16; // Move to the start of the next rank below
        } else if (*FEN >= '1' && *FEN <= '8') {
            square += *FEN - '0'; // Skip empty squares
        } else {
            pos.charBoard[square] = *FEN;
            if(isupper(*FEN)) updateBit(&pos.color[1], square);
            else updateBit(&pos.color[0], square);
            switch (*FEN) {
                case 'P': updateBit(&pos.pawn[1], square); break;
                case 'N': updateBit(&pos.knight[1], square); break;
                case 'B': updateBit(&pos.bishop[1], square); break;
                case 'R': updateBit(&pos.rook[1], square); break;
                case 'Q': updateBit(&pos.queen[1], square); break;
                case 'K': updateBit(&pos.king[1], square); break;
                case 'p': updateBit(&pos.pawn[0], square); break;
                case 'n': updateBit(&pos.knight[0], square); break;
                case 'b': updateBit(&pos.bishop[0], square); break;
                case 'r': updateBit(&pos.rook[0], square); break;
                case 'q': updateBit(&pos.queen[0], square); break;
                case 'k': updateBit(&pos.king[0], square); break;
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
        i32 file = *FEN - 'a'; // Convert file character to 0-7
        i32 rank = *(FEN + 1) - '1'; // Convert rank character to 0-7
        i32 en_passant_square = rank * 8 + file;
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
    if(pos.attack_mask[0] & pos.king[1]) pos.flags |= IN_CHECK;
    if(pos.attack_mask[1] & pos.king[0]) pos.flags |= IN_CHECK;

    //Double Check Flag
    if(pos.flags & IN_CHECK){
       i32 kign_sq = getlsb(pos.king[1]);
       u64 attackers = getAttackers(&pos, kign_sq, 0);
       attackers &= attackers - 1; // Allow underflow
       if(attackers) pos.flags |= IN_D_CHECK;
       
       kign_sq = getlsb(pos.king[0]);
       attackers = getAttackers(&pos, kign_sq, WHITE_TURN);
       attackers &= attackers - 1; // Allow underflow
       if(attackers) pos.flags |= IN_D_CHECK;
       
    }

    pos.pinned = generatePinnedPieces(&pos);

    pos.stage = calculateStage(&pos);

    pos.material_eval = eval_material(&pos);

    pos.hash = hashPosition(&pos);

    return pos;
}

i32 PositionToFen(Position pos, char* FEN) {
    i32 index = 0;
    for (i32 rank = 7; rank >= 0; rank--) {
        i32 emptyCount = 0;
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            char piece = pos.charBoard[square];
            if (piece == 0) {
                emptyCount++;
            } else {
                if (emptyCount != 0) {
                    FEN[index++] = '0' + emptyCount;
                    emptyCount = 0;
                }
                FEN[index++] = piece;
            }
        }
        if (emptyCount != 0) {
            FEN[index++] = '0' + emptyCount;
        }
        if (rank > 0) {
            FEN[index++] = '/';
        }
    }

    // Active color
    FEN[index++] = ' ';
    FEN[index++] = (pos.flags & WHITE_TURN) ? 'w' : 'b';

    // Castling availability
    FEN[index++] = ' ';
    if (pos.flags & W_SHORT_CASTLE) FEN[index++] = 'K';
    if (pos.flags & W_LONG_CASTLE) FEN[index++] = 'Q';
    if (pos.flags & B_SHORT_CASTLE) FEN[index++] = 'k';
    if (pos.flags & B_LONG_CASTLE) FEN[index++] = 'q';
    if (index == 0 || FEN[index - 1] == ' ') FEN[index++] = '-';

    // En passant target square
    FEN[index++] = ' ';
    if (pos.en_passant) {
        i32 square = getlsb(pos.en_passant);
        FEN[index++] = 'a' + (square % 8);
        FEN[index++] = '1' + (square / 8);
    } else {
        FEN[index++] = '-';
    }

    // Halfmove clock
    FEN[index++] = ' ';
    index += snprintf(&FEN[index], MAX_FEN_LEN, "%d", pos.halfmove_clock);

    // Fullmove number
    FEN[index++] = ' ';
    index += snprintf(&FEN[index], MAX_FEN_LEN, "%d", pos.fullmove_number);

    FEN[index] = '\0';

    return index;
}

i64 get_perft_fen_depth(const char *fen, int depth) {
    char pattern[10];
    snprintf(pattern, sizeof(pattern), "D%d", depth);
    const char *pos = strstr(fen, pattern);

    if (pos) {
        pos += strlen(pattern);
        while (*pos && isspace((unsigned char)*pos)) {
            pos++;
        }
        return strtoll(pos, NULL, 10);
    }

    return -1;
}




void generateBetweenMasks() {
    for (i32 sq1 = 0; sq1 < 64; sq1++) {
        i32 rank1 = sq1 / 8;
        i32 file1 = sq1 % 8;

        for (i32 sq2 = 0; sq2 < 64; sq2++) {
            i32 rank2 = sq2 / 8;
            i32 file2 = sq2 % 8;

            if (sq1 == sq2) {
                betweenMask[sq1][sq2] = 0;
                continue;
            }

            u64 mask = 0;
            
            if (rank1 == rank2) {
                for (i32 f = 1; f < abs(file2 - file1); f++) {
                    i32 file = file1 + f * ((file2 > file1) ? 1 : -1);
                    mask |= 1ULL << (rank1 * 8 + file);
                }
            } else if (file1 == file2) { 
                for (i32 r = 1; r < abs(rank2 - rank1); r++) {
                    i32 rank = rank1 + r * ((rank2 > rank1) ? 1 : -1);
                    mask |= 1ULL << (rank * 8 + file1);
                }
            } else if (abs(rank1 - rank2) == abs(file1 - file2)) { 
                for (i32 i = 1; i < abs(rank2 - rank1); i++) {
                    i32 rank = rank1 + i * ((rank2 > rank1) ? 1 : -1);
                    i32 file = file1 + i * ((file2 > file1) ? 1 : -1);
                    mask |= 1ULL << (rank * 8 + file);
                }
            }

            betweenMask[sq1][sq2] = mask;
        }
    }
}

void generateRankMasks() {
    for (i32 sq = 0; sq < 64; ++sq) {
        i32 rank = sq / 8;
        rankMask[sq] = 0xFFULL << (rank * 8);
    }
}

void generateFileMasks() {
    for (i32 sq = 0; sq < 64; ++sq) {
        i32 file = sq % 8;
        fileMask[sq] = 0x0101010101010101ULL << file;
    }
}

void generateDiagonalMasks() {
    for (i32 sq = 0; sq < 64; ++sq) {
        i32 rank = sq / 8;
        i32 file = sq % 8;
        NESWMask[sq] = 0;
        NWSEMask[sq] = 0;

        // Northeast-Southwest Diagonal
        for (i32 r = rank, f = file; r < 8 && f < 8; ++r, ++f)
            NESWMask[sq] |= (1ULL << (r * 8 + f));
        for (i32 r = rank, f = file; r >= 0 && f >= 0; --r, --f)
            NESWMask[sq] |= (1ULL << (r * 8 + f));

        // Northwest-Southeast Diagonal
        for (i32 r = rank, f = file; r < 8 && f >= 0; ++r, --f)
            NWSEMask[sq] |= (1ULL << (r * 8 + f));
        for (i32 r = rank, f = file; r >= 0 && f < 8; --r, ++f)
            NWSEMask[sq] |= (1ULL << (r * 8 + f));
    }
}

void printDebug(Position position){
    char fen[128];
    PositionToFen(position, fen);
    printf("\nFEN: %s \n\n", fen);
    printf("Hash: %" PRIu64 " \n\n", position.hash);
}

void printPosition(Position position, char verbose){
    char fen[128];
    PositionToFen(position, fen);
    printf("----------------------------------------------------------------------------------------------------------------------------------\n");
    printf("\nFEN: %s \n\n", fen);
    printf("Hash: %" PRIu64 " \n\n", position.hash);
    printf("  A B C D E F G H\n");
    for (i32 rank = 7; rank >= 0; rank--) {
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.pawn[1] & mask) printf("P ");
            else if (position.knight[1] & mask) printf("N ");
            else if (position.bishop[1] & mask) printf("B ");
            else if (position.rook[1] & mask) printf("R ");
            else if (position.queen[1] & mask) printf("Q ");
            else if (position.king[1] & mask) printf("K ");
            else if (position.pawn[0] & mask) printf("p ");
            else if (position.knight[0] & mask) printf("n ");
            else if (position.bishop[0] & mask) printf("b ");
            else if (position.rook[0] & mask) printf("r ");
            else if (position.queen[0] & mask) printf("q ");
            else if (position.king[0] & mask) printf("k ");
            else if (position.en_passant & mask) printf("E ");
            else printf(". ");

            
            if (file == 7){
                printf("%d   |  ", rank + 1);
                if(rank == 7) printf("Current Turn: %s -- Quick Evaluation: %d", ((position.flags & WHITE_TURN) ? "White" : "Black"), position.material_eval);
                if(rank == 5) printf("Halfmove Clock: %d -- Fullmove Number: %d -- Game Stage: %d", position.halfmove_clock, position.fullmove_number, position.stage);
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
    printf("----------------------------------------------------------------------------------------------------------------------------------\n");
    if(!verbose) return;
    printf("  Color Bitboard   |      White Attack   |      Black Attack   |     Pinned Pieces   |         EP Board    |       Char Board    |\n");
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    for (i32 rank = 7; rank >= 0; rank--) {
        //Color BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.color[1] & mask) printf("W ");
            else if (position.color[0] & mask) printf("b ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //White Attack
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.attack_mask[1] & mask) printf("A ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black Attack
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.attack_mask[0] & mask) printf("a ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Pinned Pieces
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.pinned & mask) printf("X ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //En-Passant Pieces
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.en_passant & mask) printf("E ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Char Board
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;

            if (position.charBoard[square]) printf("%c ",position.charBoard[square]);
            else printf(". ");
            
            if (file == 7) printf(" |\n");
        }
        
    }
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("        White      |     White Pawn      |      White Bishop   |     White Knight    |      White Rook     |      White Queen    |      White King     |\n");
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    for (i32 rank = 7; rank >= 0; rank--) {
        //White BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.color[1] & mask) printf("W ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //W Pawn BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.pawn[1] & mask) printf("P ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //W Bishop BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.bishop[1] & mask) printf("B ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //W Knight BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.knight[1] & mask) printf("N ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //W Rook
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.rook[1] & mask) printf("R ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //W Queen
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.queen[1] & mask) printf("Q ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //White King
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.king[1] & mask) printf("K ");
            else printf(". ");
            
            if (file == 7) printf(" |\n");
        }
    }
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------\n");
    printf("        Black      |       Black Pawn    |      Black Bishop   |     Black Knight    |      Black Rook     |      Black Queen    |      Black King     |\n");
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    for (i32 rank = 7; rank >= 0; rank--) {
        //Black BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.color[0] & mask) printf("b ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black Pawn BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.pawn[0] & mask) printf("p ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black Bishop BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.bishop[0] & mask) printf("b ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black Knight BB
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.knight[0] & mask) printf("n ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black Rook
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.rook[0] & mask) printf("r ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black Queen
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.queen[0] & mask) printf("q ");
            else printf(". ");

            if (file == 7) printf(" |  ");
        }

        //Black King
        printf("%d ", rank + 1);
        for (i32 file = 0; file < 8; file++) {
            i32 square = rank * 8 + file;
            u64 mask = 1ULL << square;

            if (position.king[0] & mask) printf("k ");
            else printf(". ");
            
            if (file == 7) printf(" |\n");
        }
    }
    printf("  A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |    A B C D E F G H  |\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------------\n");
}


void printBB(u64 BB) {
    printf("  A B C D E F G H\n");
    for (i32 rank = 8; rank >= 1; rank--) {
        printf("%d ", rank);
        for (i32 file = 0; file < 8; file++) {
            i32 bitPosition = (rank - 1) * 8 + file;
            if (BB & (1ULL << bitPosition)) {
                printf("1 ");
            } else {
                printf("0 ");
            }
        }
        printf("\n");
    }
}




