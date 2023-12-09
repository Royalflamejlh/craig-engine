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

#define W_SHORT_CASTLE_MASK 0x0000000000000060ULL
#define W_LONG_CASTLE_MASK  0x000000000000000EULL
#define B_SHORT_CASTLE_MASK 0x6000000000000000ULL 
#define B_LONG_CASTLE_MASK  0x0E00000000000000ULL

static void generateKingMoveMasks(void);
static void generateKnightMoveMasks(void);
static void generatePawnMoveMasks(void);

static uint64_t kingMoves[64];
static uint64_t knightMoves[64];
static uint64_t pawnMoves[64][4];

void generateMasks(void){
    generateKingMoveMasks();
    generateKnightMoveMasks();
    generatePawnMoveMasks();
    generateBetweenMasks();
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

        uint64_t singleMove = 0ULL, doubleMove = 0ULL, leftCapture = 0ULL, rightCapture = 0ULL;
        uint64_t position = setBit(0ULL, square);

        // Single square forward
        if (rank < 7) {
            singleMove = northOne(position);
        }
        pawnMoves[square][0] = singleMove;

        // Double square forward
        if (rank == 1) {
            doubleMove = northTwo(position);
        }
        pawnMoves[square][1] = doubleMove;

        // Capture left
        if (file > 0 && rank < 7) {
            leftCapture = noWeOne(position);
        }
        pawnMoves[square][2] = leftCapture;

        // Capture right
        if (file < 7 && rank < 7) {
            rightCapture = noEaOne(position);
        }
        pawnMoves[square][3] = rightCapture;
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
    return moves & ~ownPieces;
}

uint64_t getBishopMovesAppend(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    while (bishops) {
        int square = __builtin_ctzll(bishops);
        uint64_t nocap_moves = bishopAttacks(ownPieces | oppPieces, square) & ~ownPieces;
        
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

//Rook
uint64_t getRookAttacks(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces) {
    uint64_t moves = 0ULL;
    while (rooks) {
        int square = __builtin_ctzll(rooks);
        moves |= rookAttacks(ownPieces | oppPieces, square);
        rooks &= rooks - 1;
    }
    return moves & ~ownPieces;
}

uint64_t getRookMovesAppend(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    while (rooks) {
        int square = __builtin_ctzll(rooks);
        uint64_t nocap_moves = rookAttacks(ownPieces | oppPieces, square) & ~ownPieces;
        
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


//Pawn
uint64_t getWhitePawnAttacks(uint64_t pawns, uint64_t ownPieces) {
    uint64_t moves = 0ULL;

    while (pawns) {
        int square = __builtin_ctzll(pawns);

        moves |= pawnMoves[square][2];
        moves |= pawnMoves[square][3];

        pawns &= pawns - 1;
    }

    return (moves & ~ownPieces);
}

uint64_t getPawnAttacks(uint64_t pawns, uint64_t ownPieces, char flags){

    if(flags & WHITE_TURN) return getWhitePawnAttacks(pawns, ownPieces);

    return flipVertical( getWhitePawnAttacks( flipVertical(pawns), flipVertical(ownPieces)));
}

uint64_t getWhitePawnMovesAppend(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, Move* moveList, int* idx) {
    uint64_t all_moves = 0ULL;
    uint64_t occ =  0ULL;

    while (pawns) {
        uint64_t q_moves = 0ULL;         //Quiet
        uint64_t dp_moves = 0ULL;        //Double Pawn
        uint64_t cap_moves = 0ULL;       //Captures

        int square = __builtin_ctzll(pawns);
        int rank = square / 8;
        char promotion = (rank == 6) ? 1 : 0;

        //Single Step
        occ = (oppPieces | ownPieces) & pawnMoves[square][0];
        if(!occ) q_moves |= pawnMoves[square][0];
        
        //Double Step
        if(occ == 0 && rank == 1){
            occ = (oppPieces | ownPieces) & pawnMoves[square][1];
            if(!occ) dp_moves |= pawnMoves[square][1];
        }

        //Capture Left
        occ = (oppPieces | enPassant) & pawnMoves[square][2];
        if(occ) cap_moves |= pawnMoves[square][2];

        //Capture Right
        occ = (oppPieces | enPassant) & pawnMoves[square][3];
        if(occ) cap_moves |= pawnMoves[square][3];

        all_moves |= q_moves | dp_moves | cap_moves;

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

        pawns &= pawns - 1;
    }

    return all_moves;
}

uint64_t getPawnMovesAppend(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, char flags, Move* moveList, int* idx){
    if(flags & WHITE_TURN) return getWhitePawnMovesAppend(pawns, ownPieces, oppPieces, enPassant, moveList, idx);
    return flipVertical(
        getWhitePawnMovesAppend(
            flipVertical(pawns), 
            flipVertical(ownPieces), 
            flipVertical(oppPieces), 
            flipVertical(enPassant), 
            moveList, 
            idx)
        );
}


//Knight
uint64_t getKnightAttacks(uint64_t knights, uint64_t ownPieces) {
    uint64_t moves = 0ULL;

    while (knights) {
        int square = __builtin_ctzll(knights);
        moves |= knightMoves[square];
        knights &= knights - 1;
    }

    return moves & ~ownPieces;
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
    return kingMoves[square] & ~ownPieces;
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

void getCastleMovesWhiteAppend(uint64_t white, char flags, Move* moveList, int* idx){
    if ((flags & W_SHORT_CASTLE) && !(white & W_SHORT_CASTLE_MASK)) {
        Move move = MAKE_MOVE(4, 6, KING_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }

    if ((flags & W_LONG_CASTLE) && !(white & W_LONG_CASTLE_MASK)) {
        Move move = MAKE_MOVE(4, 2, QUEEN_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }
}

void getCastleMovesBlackAppend(uint64_t black, char flags, Move* moveList, int* idx){
    if ((flags & B_SHORT_CASTLE) && !(black & B_SHORT_CASTLE_MASK)) {
        Move move = MAKE_MOVE(60, 62, KING_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }

    if ((flags & B_LONG_CASTLE) && !(black & B_LONG_CASTLE_MASK)) {
        Move move = MAKE_MOVE(60, 58, QUEEN_CASTLE);
        moveList[*idx] = move;
        (*idx)++;
    }
}


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

    attack_mask = flipVertical(pawnMoves[square][2]) | flipVertical(pawnMoves[square][3]);
    attackers |= (attack_mask & pos.w_pawn);

    attack_mask = kingMoves[square];
    attackers |= (attack_mask & pos.w_king);

    return attackers;
}

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



void getCheckMovesWhiteAppend(Position pos, Move* moveList, int* idx){
    int king_sq = __builtin_ctzll(pos.w_king);
    uint64_t checker_mask = getBlackAttackers(pos, king_sq);
    int checker_sq = __builtin_ctzll(checker_mask);
    uint64_t between_squares = betweenMask[king_sq][checker_sq];


    if(pos.en_passant){
        printf("ADD IN THE EN PASSANT CHECK LOGIC STUFF \n");
    }
    else{
        getWhitePawnMovesAppend(pos.w_pawn, ~(between_squares | checker_mask), checker_mask, 0, moveList, idx);
        
        //Handle the case of double forward moves
        uint64_t pawns = pos.w_pawn;
        while (pawns) {
            uint64_t dp_moves = 0ULL;
            int square = __builtin_ctzll(pawns);
            int rank = square / 8;

            uint64_t occ = (pos.white | pos.black) & pawnMoves[square][0];

            //Double Step
            if(occ == 0 && rank == 1){
                occ = (pos.white | pos.black) & pawnMoves[square][1];
                if(!occ) dp_moves |= pawnMoves[square][1] & (between_squares); //Allow double move to between square
            }
            while(dp_moves){
                int move_sq = __builtin_ctzll(dp_moves);
                moveList[*idx] = MAKE_MOVE(square, move_sq, DOUBLE_PAWN_PUSH);
                (*idx)++;
                dp_moves &= dp_moves - 1;
            }
        }

    }
    getKingMovesAppend(pos.w_king, pos.white,  pos.black, pos.b_attack_mask, moveList, idx);

    getRookMovesAppend(pos.w_queen, ~(between_squares | checker_mask), checker_mask, moveList, idx);
    getBishopMovesAppend(pos.w_queen, ~(between_squares | checker_mask), checker_mask, moveList, idx);

    getRookMovesAppend(pos.w_rook, ~(between_squares | checker_mask), checker_mask, moveList, idx);

    getBishopMovesAppend(pos.w_bishop, ~(between_squares | checker_mask), checker_mask, moveList, idx);

    getKnightMovesAppend(pos.w_knight, ~(between_squares | checker_mask), checker_mask, moveList, idx);
}

void getPinnedMovesWhiteAppend(Position pos, Move* moveList, int* idx){
    uint64_t pinned = pos.pinned;
    int king_sq = __builtin_ctzll(pos.w_king);
    int king_rank = king_sq / 8;
    int king_file = king_sq % 8;

    //Pinned Knights Cannot Move
    pinned &= ~(pos.w_knight & pinned);

    //Proccess Pinned Queens
    if(pos.w_queen & pinned){
        
        pinned &= ~(pos.w_queen & pinned);
    }

    //Process Pinned Rooks
    if(pos.w_rook & pinned){

        pinned &= ~(pos.w_rook & pinned);
    }

    //Process Pinned Bishops
    if(pos.w_bishop & pinned){

        pinned &= ~(pos.w_bishop & pinned);
    }

    //Process Pinned Pawns
    if(pos.w_pawn & pinned){
        uint64_t pinned_pieces = pos.w_pawn & pinned;
        while(pinned_pieces){
            int sq = __builtin_ctzll(pinned_pieces);
            if(sq/8 = king_rank) continue; //Same file cant get unpinned
            pinned_pieces &= pinned_pieces - 1;
        }
        pinned &= ~(pos.w_pawn & pinned);
    }
}