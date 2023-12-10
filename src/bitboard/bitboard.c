//
//  bitboard.c
//  godengine
//
//  Created by John Howard on 11/22/23.
//   Bit 0 = A1, Bit 63 = H8
//
//   A B C D E F G H
// 1 0 0 0 0 0 0 0 0
// 2 0 0 0 0 0 0 0 0
// 3 0 0 0 0 0 0 0 0
// 4 0 0 0 0 0 0 0 0
// 5 0 0 0 0 0 0 0 0
// 6 0 0 0 0 0 0 0 0
// 7 0 0 0 0 0 0 0 0
// 8 0 0 0 0 0 0 0 0
#include "bitboard.h"
#include "magic.h"
#include "../util.h"
#include "../types.h"


#define B_FILE_MASK         0x0202020202020202ULL
#define W_SHORT_CASTLE_MASK 0x0000000000000060ULL
#define W_LONG_CASTLE_MASK  0x000000000000000EULL
#define B_SHORT_CASTLE_MASK 0x6000000000000000ULL 
#define B_LONG_CASTLE_MASK  0x0E00000000000000ULL

static void generateKingMoveMasks(void);
static void generateKnightMoveMasks(void);
static void generatePawnMoveMasks(void);

static uint64_t kingMoves[64];
static uint64_t knightMoves[64];
static uint64_t pawnMoves[64][8]; // sq-0 single move white sq-1 double move white sq-2 attack-left white sq-3 attack-right white 
                                  // sq-4 -> sq-7 same for black

void generateMasks(void){
    generateKingMoveMasks();
    generateKnightMoveMasks();
    generatePawnMoveMasks();
    generateBetweenMasks();
    generateRankMasks();
    generateFileMasks();
    generateDiagonalMasks();
}


static void generateKingMoveMasks(void) {
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;

        kingMoves[square] = 0ULL;
        if (rank > 0) {
            kingMoves[square] |= 1ULL << (square - 8);
            if (file > 0) kingMoves[square] |= 1ULL << (square - 9);
            if (file < 7) kingMoves[square] |= 1ULL << (square - 7);
        }
        if (rank < 7) {
            kingMoves[square] |= 1ULL << (square + 8);
            if (file > 0) kingMoves[square] |= 1ULL << (square + 7); // Down-Left
            if (file < 7) kingMoves[square] |= 1ULL << (square + 9); // Down-Right
        }
        if (file > 0) kingMoves[square] |= 1ULL << (square - 1); // Left
        if (file < 7) kingMoves[square] |= 1ULL << (square + 1); // Right
    }
}

static void setKnightMove(int square, int r, int f) {
    if (r >= 0 && r < 8 && f >= 0 && f < 8)
        knightMoves[square] |= 1ULL << (r * 8 + f);
}

static void generateKnightMoveMasks(void) {
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;

        knightMoves[square] = 0ULL;

        setKnightMove(square, rank - 2, file - 1);
        setKnightMove(square, rank - 2, file + 1);
        setKnightMove(square, rank - 1, file - 2);
        setKnightMove(square, rank - 1, file + 2);
        setKnightMove(square, rank + 1, file - 2);
        setKnightMove(square, rank + 1, file + 2);
        setKnightMove(square, rank + 2, file - 1);
        setKnightMove(square, rank + 2, file + 1);
    }
}

void generatePawnMoveMasks(void){
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;

        uint64_t position = setBit(0ULL, square);

        // For White Pawns
        pawnMoves[square][0] = (rank < 7) ?  northOne(position) : 0ULL;
        pawnMoves[square][1] = (rank == 1) ? northTwo(position) : 0ULL;
        pawnMoves[square][2] = (file > 0 && rank < 7) ? noWeOne(position) : 0ULL;
        pawnMoves[square][3] = (file < 7 && rank < 7) ? noEaOne(position) : 0ULL;

        // For Black Pawns
        pawnMoves[square][4] = (rank > 0) ?  southOne(position) : 0ULL;
        pawnMoves[square][5] = (rank == 6) ? southTwo(position) : 0ULL;
        pawnMoves[square][6] = (file > 0 && rank > 0) ? soWeOne(position) : 0ULL;
        pawnMoves[square][7] = (file < 7 && rank > 0) ? soEaOne(position) : 0ULL;
    }
}


//Bishop
uint64_t getBishopAttacks(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces) {
    uint64_t moves = 0ULL;
    while (bishops) {
        int square = __builtin_ctzll(bishops);
        moves |= bishopAttacks(ownPieces | oppPieces, square);
        bishops &= bishops - 1;
    }
    return moves;
}

static uint64_t getBishopMovesCheckAppend(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces, uint64_t legalSquares, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    while (bishops) {
        int square = __builtin_ctzll(bishops);
        uint64_t nocap_moves = bishopAttacks(ownPieces | oppPieces, square) & ~ownPieces & legalSquares;
        
        all_moves |= nocap_moves;
        uint64_t cap_moves = nocap_moves & oppPieces;
        nocap_moves &= ~cap_moves;

        while(nocap_moves){
            int move_sq = __builtin_ctzll(nocap_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, QUIET);
            (*idx)++;
            nocap_moves &= nocap_moves - 1;
        }

        while(cap_moves){
            int move_sq = __builtin_ctzll(cap_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, CAPTURE);
            (*idx)++;
            cap_moves &= cap_moves - 1;
        }

        bishops &= bishops - 1;
    }
    return all_moves;
}

uint64_t getBishopMovesAppend(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx) {
    return getBishopMovesCheckAppend(bishops, ownPieces, oppPieces, ~(0x0ULL), moveList, idx);
}

//Rook
uint64_t getRookAttacks(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces) {
    uint64_t moves = 0ULL;
    while (rooks) {
        int square = __builtin_ctzll(rooks);
        moves |= rookAttacks(ownPieces | oppPieces, square);
        rooks &= rooks - 1;
    }
    return moves;
}

static uint64_t getRookMovesCheckAppend(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces, uint64_t legalSquares, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    while (rooks) {
        int square = __builtin_ctzll(rooks);
        uint64_t nocap_moves = rookAttacks(ownPieces | oppPieces, square) & ~ownPieces & legalSquares;
        
        all_moves |= nocap_moves;
        uint64_t cap_moves = nocap_moves & oppPieces;
        nocap_moves &= ~cap_moves;

        while(nocap_moves){
            int move_sq = __builtin_ctzll(nocap_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, QUIET);
            (*idx)++;
            nocap_moves &= nocap_moves - 1;
        }

        while(cap_moves){
            int move_sq = __builtin_ctzll(cap_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, CAPTURE);
            (*idx)++;
            cap_moves &= cap_moves - 1;
        }

        rooks &= rooks - 1;
    }
    return all_moves;
}

uint64_t getRookMovesAppend(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx) {
    return getRookMovesCheckAppend(rooks, ownPieces, oppPieces, ~(0x0ULL), moveList, idx);
}

/*
* The worst piece in chess (in many ways) is below here.
*/
uint64_t getPawnAttacks(uint64_t pawns, uint64_t ownPieces, char flags){
    uint64_t moves = 0ULL;
    int pawn_mask_idx = (flags & WHITE_TURN) ? 0 : 4;

    while (pawns) {
        int square = __builtin_ctzll(pawns);

        moves |= pawnMoves[square][pawn_mask_idx + 2];
        moves |= pawnMoves[square][pawn_mask_idx + 3];

        pawns &= pawns - 1;
    }

    return moves;
}

uint64_t getPawnMovesAppend(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, char flags, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    uint64_t occ =  0ULL;
    char turn = (flags & WHITE_TURN);
    int pawn_mask_idx = turn ? 0 : 4;

    while (pawns) {
        uint64_t q_moves = 0ULL;         //Quiet
        uint64_t dp_moves = 0ULL;        //Double Pawn
        uint64_t cap_moves = 0ULL;       //Captures
        uint64_t ep_moves = 0ULL;        //En Passants

        int square = __builtin_ctzll(pawns);
        int rank = square / 8;
        char promotion = turn ? (rank == 6) : (rank == 1);
        char can_double = turn ? (rank == 1) : (rank == 6);

        //Single Step
        occ = (oppPieces | ownPieces) & pawnMoves[square][pawn_mask_idx + 0];
        if(!occ) q_moves |= pawnMoves[square][pawn_mask_idx + 0];
        
        //Double Step
        if(occ == 0 && can_double){
            occ = (oppPieces | ownPieces) & pawnMoves[square][pawn_mask_idx + 1];
            if(!occ) dp_moves |= pawnMoves[square][pawn_mask_idx + 1];
        }

        //Capture Left
        occ = (oppPieces) & pawnMoves[square][pawn_mask_idx + 2];
        if(occ) cap_moves |= pawnMoves[square][pawn_mask_idx + 2];

        occ = (enPassant) & pawnMoves[square][pawn_mask_idx + 2];
        if(occ) ep_moves |= pawnMoves[square][pawn_mask_idx + 2];

        //Capture Right
        occ = (oppPieces) & pawnMoves[square][pawn_mask_idx + 3];
        if(occ) cap_moves |= pawnMoves[square][pawn_mask_idx + 3];

        occ = (enPassant) & pawnMoves[square][pawn_mask_idx + 3];
        if(occ) ep_moves |= pawnMoves[square][pawn_mask_idx + 3];

        all_moves |= q_moves | dp_moves | cap_moves | ep_moves;

        while(q_moves){
            int move_sq = __builtin_ctzll(q_moves);
            if (promotion) {
                Move baseMove = MAKE_MOVE(square, move_sq, QUIET);
                moveList[*idx] = SET_QUEEN_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_ROOK_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_BISHOP_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_KNIGHT_PROMOTION(baseMove);
                (*idx)++;
            } else {
                moveList[*idx] = MAKE_MOVE(square, move_sq, QUIET);
                (*idx)++;
            }
            q_moves &= q_moves - 1;
        }

        while(cap_moves){
            int move_sq = __builtin_ctzll(cap_moves);
            if (promotion) {
                Move baseMove = MAKE_MOVE(square, move_sq, CAPTURE);
                moveList[*idx] = SET_QUEEN_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_ROOK_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_BISHOP_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_KNIGHT_PROMO_CAPTURE(baseMove);
                (*idx)++;
            } else {
                moveList[*idx] = MAKE_MOVE(square, move_sq, CAPTURE);
                (*idx)++;
            }
            cap_moves &= cap_moves - 1;
        }

        while(dp_moves){
            int move_sq = __builtin_ctzll(dp_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, DOUBLE_PAWN_PUSH);
            (*idx)++;
            dp_moves &= dp_moves - 1;
        }

        while(ep_moves){
            int move_sq = __builtin_ctzll(ep_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, EP_CAPTURE);
            (*idx)++;
            ep_moves &= ep_moves - 1;
        }

        pawns &= pawns - 1;
    }

    return all_moves;
}


/*
* Jumpity jump these bad boys are the easiest thing to implement in chess somehow
*/
uint64_t getKnightAttacks(uint64_t knights, uint64_t ownPieces) {
    uint64_t moves = 0ULL;

    while (knights) {
        int square = __builtin_ctzll(knights);
        moves |= knightMoves[square];
        knights &= knights - 1;
    }

    return moves;
}

uint64_t getKnightMovesAppend(uint64_t knights, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;

    while (knights) {
        int square = __builtin_ctzll(knights);

        uint64_t nocap_moves = knightMoves[square] & ~ownPieces;

        all_moves |= nocap_moves;
        uint64_t cap_moves = nocap_moves & oppPieces;
        nocap_moves &= ~cap_moves;

        while(nocap_moves){
            int move_sq = __builtin_ctzll(nocap_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, QUIET);
            (*idx)++;
            nocap_moves &= nocap_moves - 1;
        }
        while(cap_moves){
            int move_sq = __builtin_ctzll(cap_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, CAPTURE);
            (*idx)++;
            cap_moves &= cap_moves - 1;
        }
        
        knights &= knights - 1;
    }

    return all_moves;
}

//King
uint64_t getKingAttacks(uint64_t kings, uint64_t ownPieces) {
    int square = __builtin_ctzll(kings);
    return kingMoves[square];
}

uint64_t getKingMovesAppend(uint64_t kings, uint64_t ownPieces, uint64_t oppPieces, uint64_t oppAttackMask, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;

    int square = __builtin_ctzll(kings);
    uint64_t nocap_moves = kingMoves[square] & ~ownPieces & ~oppAttackMask;
    all_moves |= nocap_moves;

    uint64_t cap_moves = nocap_moves & oppPieces;
    nocap_moves &= ~cap_moves;

    while(nocap_moves){
        int move_sq = __builtin_ctzll(nocap_moves);
        moveList[*idx] = MAKE_MOVE(square, move_sq, QUIET);
        (*idx)++;
        nocap_moves &= nocap_moves - 1;
    }
    while(cap_moves){
        int move_sq = __builtin_ctzll(cap_moves);
        moveList[*idx] = MAKE_MOVE(square, move_sq, CAPTURE);
        (*idx)++;
        cap_moves &= cap_moves - 1;
    }
    return all_moves;
}

void getCastleMovesWhiteAppend(uint64_t white, uint64_t b_attack_mask, char flags, Move* moveList, int* idx){
    if ((flags & W_SHORT_CASTLE) && !((white | b_attack_mask) & W_SHORT_CASTLE_MASK)) {
        Move move = MAKE_MOVE(4, 6, KING_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }

    if ((flags & W_LONG_CASTLE) && !((white | (b_attack_mask & ~B_FILE_MASK)) & W_LONG_CASTLE_MASK)) {
        Move move = MAKE_MOVE(4, 2, QUEEN_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }
}

void getCastleMovesBlackAppend(uint64_t black, uint64_t w_attack_mask, char flags, Move* moveList, int* idx){
    if ((flags & B_SHORT_CASTLE) && !((black | w_attack_mask) & B_SHORT_CASTLE_MASK)) {
        Move move = MAKE_MOVE(60, 62, KING_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }

    if ((flags & B_LONG_CASTLE) && !((black | (w_attack_mask & ~B_FILE_MASK)) & B_LONG_CASTLE_MASK)) {
        Move move = MAKE_MOVE(60, 58, QUEEN_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }
}

/*
*  Returns all white pieces attacking a square
*/
uint64_t getWhiteAttackers(Position pos, int square){
    uint64_t attackers = 0ULL;
    uint64_t all_pieces = pos.white | pos.black;
    uint64_t attack_mask;

    attack_mask = rookAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos.w_queen | pos.w_rook));

    attack_mask = bishopAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos.w_queen | pos.w_bishop));

    attack_mask = knightMoves[square];
    attackers |= (attack_mask & pos.w_knight);

    attack_mask = pawnMoves[square][6] | pawnMoves[square][7];
    attackers |= (attack_mask & pos.w_pawn);

    attack_mask = kingMoves[square];
    attackers |= (attack_mask & pos.w_king);

    return attackers;
}

/*
*  Returns all black pieces attacking a square
*/
uint64_t getBlackAttackers(Position pos, int square){
    uint64_t attackers = 0ULL;
    uint64_t all_pieces = pos.white | pos.black;
    uint64_t attack_mask;

    attack_mask = rookAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos.b_queen | pos.b_rook));

    attack_mask = bishopAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos.b_queen | pos.b_bishop));

    attack_mask = knightMoves[square];
    attackers |= (attack_mask & pos.b_knight);

    attack_mask = pawnMoves[square][2] | pawnMoves[square][3];
    attackers |= (attack_mask & pos.b_pawn);

    attack_mask = kingMoves[square];
    attackers |= (attack_mask & pos.b_king);

    return attackers;
}


/*
* Here be ye function to get moves for white when they are in check!
*/
void getCheckMovesAppend(Position pos, Move* moveList, int* idx){
    int turn = pos.flags & WHITE_TURN;
    int king_sq = __builtin_ctzll(turn ? pos.w_king : pos.b_king);
    uint64_t checker_mask = turn ? getBlackAttackers(pos, king_sq) : getWhiteAttackers(pos, king_sq);
    int checker_sq = __builtin_ctzll(checker_mask);
    int pawn_mask_idx = turn ? 0 : 4;
    uint64_t ownPieces = turn ? pos.white : pos.black;
    uint64_t oppPieces = turn ? pos.black : pos.white;
    uint64_t between_squares = betweenMask[king_sq][checker_sq];

    
    if(pos.en_passant){
        uint64_t test_mask = turn ? southOne(pos.en_passant) : northOne(pos.en_passant);
        if(test_mask == checker_mask){
            getPawnMovesAppend(turn ? pos.w_pawn : pos.b_pawn, ~(pos.en_passant), 0ULL, pos.en_passant, pos.flags, moveList, idx); 
        }
    }
    
    
    getPawnMovesAppend(turn ? pos.w_pawn : pos.b_pawn, ~(between_squares | checker_mask), checker_mask, 0ULL, pos.flags, moveList, idx); 
    uint64_t pawns = turn ? pos.w_pawn : pos.b_pawn;
    while (pawns) { //Handle the case of double forward moves
        uint64_t dp_moves = 0ULL;
        int square = __builtin_ctzll(pawns);
        int rank = square / 8;
        char can_double = turn ? (rank == 1) : (rank == 6);
        uint64_t occ = (ownPieces | oppPieces) & pawnMoves[square][pawn_mask_idx + 0];
        if(occ == 0 && can_double){ //Double Step
            occ = (ownPieces | oppPieces) & pawnMoves[square][pawn_mask_idx + 1];
            if(!occ) dp_moves |= pawnMoves[square][pawn_mask_idx + 1] & (between_squares); //Allow double move to between square
        }
        while(dp_moves){
            int move_sq = __builtin_ctzll(dp_moves);
            moveList[*idx] = MAKE_MOVE(square, move_sq, DOUBLE_PAWN_PUSH);
            (*idx)++;
            dp_moves &= dp_moves - 1;
        }
        pawns &= pawns - 1;
    }

    getKingMovesAppend(  turn ? pos.w_king   : pos.b_king, ownPieces,  oppPieces, turn ? pos.b_attack_mask : pos.w_attack_mask, moveList, idx);
    getKnightMovesAppend(turn ? pos.w_knight : pos.b_knight, ~(between_squares | checker_mask), checker_mask, moveList, idx);


    getRookMovesCheckAppend(  turn ? pos.w_queen  : pos.b_queen,  ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
    getBishopMovesCheckAppend(turn ? pos.w_queen  : pos.b_queen,  ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
    getRookMovesCheckAppend(  turn ? pos.w_rook   : pos.b_rook,   ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
    getBishopMovesCheckAppend(turn ? pos.w_bishop : pos.b_bishop, ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
}



/*
*
*   Here lay the Pinned Move functions xDDDDD lmao
*
*/
static void getPinnedQueenMovesAppend(int king_rank, int king_file, uint64_t pinned_queens, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* size) {
    while(pinned_queens){ //Process Each Pinned Queen Individually
        int queen_sq = __builtin_ctzll(pinned_queens);
        int queen_rank = queen_sq / 8;
        int queen_file = queen_sq % 8;
        if(queen_rank == king_rank)                                getRookMovesAppend(1ULL << queen_sq, ownPieces | rankMask[queen_sq], oppPieces, moveList, size);
        else if(queen_file == king_file)                           getRookMovesAppend(1ULL << queen_sq, ownPieces | fileMask[queen_sq], oppPieces, moveList, size);
        else if(queen_rank - queen_file == king_rank - king_file)  getBishopMovesAppend(1ULL << queen_sq, ownPieces | NWSEMask[queen_sq], oppPieces, moveList, size);
        else if(queen_rank + queen_file == king_rank + king_file)  getBishopMovesAppend(1ULL << queen_sq, ownPieces | NESWMask[queen_sq], oppPieces, moveList, size);
        pinned_queens &= pinned_queens - 1;
    }
}

static void getPinnedRookMovesAppend(int king_rank, int king_file, uint64_t pinned_rooks, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* size) {
    while(pinned_rooks){ //Process Each Pinned rook Individually
        int rook_sq = __builtin_ctzll(pinned_rooks);
        int rook_rank = rook_sq / 8;
        int rook_file = rook_sq % 8;
        if(rook_rank == king_rank)      getRookMovesAppend(1ULL << rook_sq, ownPieces | rankMask[rook_sq], oppPieces, moveList, size);
        else if(rook_file == king_file) getRookMovesAppend(1ULL << rook_sq, ownPieces | fileMask[rook_sq], oppPieces, moveList, size);
        pinned_rooks &= pinned_rooks - 1;
    }
}

static void getPinnedBishopMovesAppend(int king_rank, int king_file, uint64_t pinned_bishops, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* size) {
    while(pinned_bishops){ //Process Each Pinned bishop Individually
        int bishop_sq = __builtin_ctzll(pinned_bishops);
        int bishop_rank = bishop_sq / 8;
        int bishop_file = bishop_sq % 8;

        if(bishop_rank - bishop_file == king_rank - king_file){
            getBishopMovesAppend(1ULL << bishop_sq, ownPieces | NWSEMask[bishop_sq], oppPieces, moveList, size);
        }
        
        else if(bishop_rank + bishop_file == king_rank + king_file){
            getBishopMovesAppend(1ULL << bishop_sq, ownPieces | NESWMask[bishop_sq], oppPieces, moveList, size);
        }
        pinned_bishops &= pinned_bishops - 1;
    }
}

static void getPinnedPawnMovesAppend(int king_rank, int king_file, uint64_t pinned_pawns, uint64_t ownPieces, uint64_t oppPieces, uint64_t en_passant, char flags, Move* moveList, int* size) {
    while(pinned_pawns){ //Process Each Pinned pawns Individually
        int pawn_sq = __builtin_ctzll(pinned_pawns);
        int pawn_rank = pawn_sq / 8;
        int pawn_file = pawn_sq % 8;
        int pawn_mask_idx = (flags & WHITE_TURN) ? 0 : 4;

        if(pawn_file == king_file) getPawnMovesAppend(1ULL << pawn_sq, ownPieces, (oppPieces & ~pawnMoves[pawn_sq][pawn_mask_idx + 2] & ~pawnMoves[pawn_sq][pawn_mask_idx + 3]), 0ULL, flags, moveList, size); //Hide black pieces that could be captured and no en-passant
        else if(pawn_rank - pawn_file == king_rank - king_file){
            uint64_t dir = pawnMoves[pawn_sq][pawn_mask_idx + 2];
            uint64_t occ = oppPieces & dir;
            getPawnMovesAppend(1ULL << pawn_sq, (ownPieces | ~occ), occ, en_passant & dir, flags, moveList, size);
        }
        else if(pawn_rank + pawn_file == king_rank + king_file){
            uint64_t dir = pawnMoves[pawn_sq][pawn_mask_idx + 3];
            uint64_t occ = oppPieces & dir;
            getPawnMovesAppend(1ULL << pawn_sq, (ownPieces | ~occ), occ, en_passant & dir, flags, moveList, size);
        }
        pinned_pawns &= pinned_pawns - 1;
    }
}

void getPinnedMovesWhiteAppend(Position pos, Move* moveList, int* size){
    uint64_t pinned = pos.pinned;
    int king_sq = __builtin_ctzll(pos.w_king);
    int king_rank = king_sq / 8;
    int king_file = king_sq % 8;

    //King does his thang
    getKingMovesAppend(pos.w_king, pos.white,  pos.black, pos.b_attack_mask, moveList, size);
    getCastleMovesWhiteAppend(pos.white, pos.b_attack_mask, pos.flags, moveList, size);

    //Pinned Knights Cannot Move
    uint64_t pinned_knights = pos.w_knight & pinned;
    getKnightMovesAppend(pos.w_knight & ~pinned_knights, pos.white, pos.black, moveList, size);
    
    uint64_t pinned_queens = pos.w_queen & pinned;
    getBishopMovesAppend(pos.w_queen & ~pinned_queens, pos.white, pos.black, moveList, size);
    getRookMovesAppend(  pos.w_queen & ~pinned_queens, pos.white, pos.black, moveList, size);
    getPinnedQueenMovesAppend(king_rank, king_file, pinned_queens, pos.white, pos.black, moveList, size);
    
    uint64_t pinned_rooks = pos.w_rook & pinned;
    getRookMovesAppend(pos.w_rook & ~pinned_rooks, pos.white, pos.black, moveList, size);
    getPinnedRookMovesAppend(king_rank, king_file, pinned_rooks, pos.white, pos.black, moveList, size);

    //Proccess Pinned Bishops
    uint64_t pinned_bishops = pos.w_bishop & pinned;
    getBishopMovesAppend(pos.w_bishop & ~pinned_bishops, pos.white, pos.black, moveList, size);
    getPinnedBishopMovesAppend(king_rank, king_file, pinned_bishops, pos.white, pos.black, moveList, size);


    //Proccess Pinned Pawns
    uint64_t pinned_pawns = pos.w_pawn & pinned;
    getPawnMovesAppend(pos.w_pawn & ~pinned_pawns, pos.white, pos.black, pos.en_passant, WHITE_TURN, moveList, size);
    getPinnedPawnMovesAppend(king_rank, king_file, pinned_pawns, pos.white, pos.black, pos.en_passant, pos.flags, moveList, size);
}

void getPinnedMovesBlackAppend(Position pos, Move* moveList, int* size){
    uint64_t pinned = pos.pinned;
    int king_sq = __builtin_ctzll(pos.b_king);
    int king_rank = king_sq / 8;
    int king_file = king_sq % 8;
    uint64_t ownPieces = pos.black;
    uint64_t oppPieces = pos.white;

    //King does his thang
    getKingMovesAppend(pos.b_king, ownPieces,  oppPieces, pos.w_attack_mask, moveList, size);
    getCastleMovesBlackAppend(ownPieces, pos.w_attack_mask, pos.flags, moveList, size);

    //Pinned Knights Cannot Move
    uint64_t pinned_knights = pos.b_knight & pinned;
    getKnightMovesAppend(pos.b_knight & ~pinned_knights, ownPieces, oppPieces, moveList, size);
    
    uint64_t pinned_queens = pos.b_queen & pinned;
    getBishopMovesAppend(pos.b_queen & ~pinned_queens, ownPieces, oppPieces, moveList, size);
    getRookMovesAppend(  pos.b_queen & ~pinned_queens, ownPieces, oppPieces, moveList, size);
    getPinnedQueenMovesAppend(king_rank, king_file, pinned_queens, ownPieces, oppPieces, moveList, size);
    
    uint64_t pinned_rooks = pos.b_rook & pinned;
    getRookMovesAppend(pos.b_rook & ~pinned_rooks, ownPieces, oppPieces, moveList, size);
    getPinnedRookMovesAppend(king_rank, king_file, pinned_rooks, ownPieces, oppPieces, moveList, size);

    //Proccess Pinned Bishops
    uint64_t pinned_bishops = pos.b_bishop & pinned;
    getBishopMovesAppend(pos.b_bishop & ~pinned_bishops, ownPieces, oppPieces, moveList, size);
    getPinnedBishopMovesAppend(king_rank, king_file, pinned_bishops, ownPieces, oppPieces, moveList, size);


    //Proccess Pinned Pawns
    uint64_t pinned_pawns = pos.b_pawn & pinned;
    getPawnMovesAppend(pos.b_pawn & ~pinned_pawns, ownPieces, oppPieces, pos.en_passant, pos.flags, moveList, size);
    getPinnedPawnMovesAppend(king_rank, king_file, pinned_pawns, ownPieces, oppPieces, pos.en_passant, pos.flags, moveList, size);
}