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

static u64 kingMoves[64];
static u64 knightMoves[64];
static u64 pawnMoves[64][8]; // sq-0 single move white sq-1 double move white sq-2 attack-left white sq-3 attack-right white 
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
    for (i32 square = 0; square < 64; square++) {
        i32 rank = square / 8;
        i32 file = square % 8;

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

static void setKnightMove(i32 square, i32 r, i32 f) {
    if (r >= 0 && r < 8 && f >= 0 && f < 8)
        knightMoves[square] |= 1ULL << (r * 8 + f);
}

static void generateKnightMoveMasks(void) {
    for (i32 square = 0; square < 64; square++) {
        i32 rank = square / 8;
        i32 file = square % 8;

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
    for (i32 square = 0; square < 64; square++) {
        i32 rank = square / 8;
        i32 file = square % 8;

        u64 position = setBit(0ULL, square);

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

//All Attacks
u64 generateAttacks(Position* position, i32 turn){
    u64 attack_mask = 0ULL;
    attack_mask |= getBishopAttacks(position->bishop[turn], position->color[turn], position->color[!turn] & ~position->king[!turn]);
    attack_mask |= getRookAttacks(  position->rook[turn],   position->color[turn], position->color[!turn] & ~position->king[!turn]);
    attack_mask |= getBishopAttacks(position->queen[turn],  position->color[turn], position->color[!turn] & ~position->king[!turn]);
    attack_mask |= getRookAttacks(  position->queen[turn],  position->color[turn], position->color[!turn] & ~position->king[!turn]);
    attack_mask |= getKnightAttacks(position->knight[turn]);
    attack_mask |= getKingAttacks(  position->king[turn]  );
    attack_mask |= getPawnAttacks(  position->pawn[turn], turn);
    return attack_mask;
}


//Bishop
u64 getBishopAttacks(u64 bishops, u64 ownPieces, u64 oppPieces) {
    u64 moves = 0ULL;
    while (bishops) {
        i32 square = getlsb(bishops);
        moves |= bishopAttacks(ownPieces | oppPieces, square);
        bishops &= bishops - 1;
    }
    return moves;
}

static u64 getBishopMovesCheckAppend(u64 bishops, u64 ownPieces, u64 oppPieces, u64 legalSquares, Move* moveList, i32* idx) {
    u64 all_moves = 0ULL;
    while (bishops) {
        i32 square = getlsb(bishops);
        u64 nocap_moves = bishopAttacks(ownPieces | oppPieces, square) & ~ownPieces & legalSquares;
        
        all_moves |= nocap_moves;
        u64 cap_moves = nocap_moves & oppPieces;
        nocap_moves &= ~cap_moves;

        while(nocap_moves){
            i32 move_sq = getlsb(nocap_moves);
            moveList[*idx] = create_move(square, move_sq, QUIET);
            (*idx)++;
            nocap_moves &= nocap_moves - 1;
        }

        while(cap_moves){
            i32 move_sq = getlsb(cap_moves);
            moveList[*idx] = create_move(square, move_sq, CAPTURE);
            (*idx)++;
            cap_moves &= cap_moves - 1;
        }

        bishops &= bishops - 1;
    }
    return all_moves;
}

u64 getBishopThreatMovesAppend(u64 bishops, u64 ownPieces, u64 oppPieces, u64 checkSquares, Move* moveList, i32* idx) {
    return getBishopMovesCheckAppend(bishops, ownPieces, oppPieces, checkSquares | oppPieces, moveList, idx);
}

u64 getBishopMovesAppend(u64 bishops, u64 ownPieces, u64 oppPieces, Move* moveList, i32* idx) {
    return getBishopMovesCheckAppend(bishops, ownPieces, oppPieces, ~(0x0ULL), moveList, idx);
}

//Rook
u64 getRookAttacks(u64 rooks, u64 ownPieces, u64 oppPieces) {
    u64 moves = 0ULL;
    while (rooks) {
        i32 square = getlsb(rooks);
        moves |= rookAttacks(ownPieces | oppPieces, square);
        rooks &= rooks - 1;
    }
    return moves;
}

static u64 getRookMovesCheckAppend(u64 rooks, u64 ownPieces, u64 oppPieces, u64 legalSquares, Move* moveList, i32* idx) {
    u64 all_moves = 0ULL;
    while (rooks) {
        i32 square = getlsb(rooks);
        u64 nocap_moves = rookAttacks(ownPieces | oppPieces, square) & ~ownPieces & legalSquares;
        
        all_moves |= nocap_moves;
        u64 cap_moves = nocap_moves & oppPieces;
        nocap_moves &= ~cap_moves;

        while(nocap_moves){
            i32 move_sq = getlsb(nocap_moves);
            moveList[*idx] = create_move(square, move_sq, QUIET);
            (*idx)++;
            nocap_moves &= nocap_moves - 1;
        }

        while(cap_moves){
            i32 move_sq = getlsb(cap_moves);
            moveList[*idx] = create_move(square, move_sq, CAPTURE);
            (*idx)++;
            cap_moves &= cap_moves - 1;
        }

        rooks &= rooks - 1;
    }
    return all_moves;
}

u64 getRookThreatMovesAppend(u64 rooks, u64 ownPieces, u64 oppPieces, u64 checkSquares, Move* moveList, i32* idx) {
    return getRookMovesCheckAppend(rooks, ownPieces, oppPieces, checkSquares | oppPieces, moveList, idx);
}

u64 getRookMovesAppend(u64 rooks, u64 ownPieces, u64 oppPieces, Move* moveList, i32* idx) {
    return getRookMovesCheckAppend(rooks, ownPieces, oppPieces, ~(0x0ULL), moveList, idx);
}

/*
* The worst piece in chess (in many ways) is below here.
*/

u64 pawnAttacks(u64 square, char turn){
    u64 moves = 0ULL;
    i32 pawn_mask_idx = (turn & TURN_MASK) ? 0 : 4;

    moves |= pawnMoves[square][pawn_mask_idx + 2];
    moves |= pawnMoves[square][pawn_mask_idx + 3];

    return moves;
}


u64 getPawnAttacks(u64 pawns, char turn){
    u64 moves = 0ULL;
    i32 pawn_mask_idx = (turn & TURN_MASK) ? 0 : 4;

    while (pawns) {
        i32 square = getlsb(pawns);

        moves |= pawnMoves[square][pawn_mask_idx + 2];
        moves |= pawnMoves[square][pawn_mask_idx + 3];

        pawns &= pawns - 1;
    }

    return moves;
}

u64 getPawnMovesAppend(u64 pawns, u64 ownPieces, u64 oppPieces,  u64 enPassant, char flags, Move* moveList, i32* idx) {
    u64 all_moves = 0ULL;
    u64 occ =  0ULL;
    char turn = (flags & WHITE_TURN);
    i32 pawn_mask_idx = turn ? 0 : 4;

    while (pawns) {
        u64 q_moves = 0ULL;         //Quiet
        u64 dp_moves = 0ULL;        //Double Pawn
        u64 cap_moves = 0ULL;       //Captures
        u64 ep_moves = 0ULL;        //En Passants

        i32 square = getlsb(pawns);
        i32 rank = square / 8;
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
            i32 move_sq = getlsb(q_moves);
            if (promotion) {
                Move baseMove = create_move(square, move_sq, QUIET);
                moveList[*idx] = SET_QUEEN_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_ROOK_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_BISHOP_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_KNIGHT_PROMOTION(baseMove);
                (*idx)++;
            } else {
                moveList[*idx] = create_move(square, move_sq, QUIET);
                (*idx)++;
            }
            q_moves &= q_moves - 1;
        }

        while(cap_moves){
            i32 move_sq = getlsb(cap_moves);
            if (promotion) {
                Move baseMove = create_move(square, move_sq, CAPTURE);
                moveList[*idx] = SET_QUEEN_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_ROOK_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_BISHOP_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_KNIGHT_PROMO_CAPTURE(baseMove);
                (*idx)++;
            } else {
                moveList[*idx] = create_move(square, move_sq, CAPTURE);
                (*idx)++;
            }
            cap_moves &= cap_moves - 1;
        }

        while(dp_moves){
            i32 move_sq = getlsb(dp_moves);
            moveList[*idx] = create_move(square, move_sq, DOUBLE_PAWN_PUSH);
            (*idx)++;
            dp_moves &= dp_moves - 1;
        }

        while(ep_moves){
            i32 move_sq = getlsb(ep_moves);
            moveList[*idx] = create_move(square, move_sq, EP_CAPTURE);
            (*idx)++;
            ep_moves &= ep_moves - 1;
        }

        pawns &= pawns - 1;
    }

    return all_moves;
}

u64 getPawnThreatMovesAppend(u64 pawns, u64 ownPieces, u64 oppPieces,  u64 enPassant, char flags, i32 opp_king_square, Move* moveList, i32* idx) {
    u64 all_moves = 0ULL;
    u64 occ =  0ULL;
    char turn = (flags & WHITE_TURN);
    i32 pawn_mask_idx = turn ? 0 : 4;

    i32 opp_pawn_mask_idx = turn ? 4 : 0;
    u64 check_squares = pawnMoves[opp_king_square][opp_pawn_mask_idx + 2] | pawnMoves[opp_king_square][opp_pawn_mask_idx + 3];

    while (pawns) {
        u64 q_moves = 0ULL;         //Quiet
        u64 dp_moves = 0ULL;        //Double Pawn
        u64 cap_moves = 0ULL;       //Captures
        u64 ep_moves = 0ULL;        //En Passants

        i32 square = getlsb(pawns);
        i32 rank = square / 8;
        char promotion = turn ? (rank == 6) : (rank == 1);
        char can_double = turn ? (rank == 1) : (rank == 6);

        //Single Step
        occ = (oppPieces | ownPieces) & pawnMoves[square][pawn_mask_idx + 0];
        if(!occ) q_moves |= (pawnMoves[square][pawn_mask_idx + 0] & check_squares);
        
        //Double Step
        if(occ == 0 && can_double){
            occ = (oppPieces | ownPieces) & pawnMoves[square][pawn_mask_idx + 1];
            if(!occ) dp_moves |= (pawnMoves[square][pawn_mask_idx + 1] & check_squares);
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
            i32 move_sq = getlsb(q_moves);
            if (promotion) {
                Move baseMove = create_move(square, move_sq, QUIET);
                moveList[*idx] = SET_QUEEN_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_ROOK_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_BISHOP_PROMOTION(baseMove);
                (*idx)++;
                moveList[*idx] = SET_KNIGHT_PROMOTION(baseMove);
                (*idx)++;
            } else {
                moveList[*idx] = create_move(square, move_sq, QUIET);
                (*idx)++;
            }
            q_moves &= q_moves - 1;
        }

        while(cap_moves){
            i32 move_sq = getlsb(cap_moves);
            if (promotion) {
                Move baseMove = create_move(square, move_sq, CAPTURE);
                moveList[*idx] = SET_QUEEN_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_ROOK_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_BISHOP_PROMO_CAPTURE(baseMove);
                (*idx)++;
                moveList[*idx] = SET_KNIGHT_PROMO_CAPTURE(baseMove);
                (*idx)++;
            } else {
                moveList[*idx] = create_move(square, move_sq, CAPTURE);
                (*idx)++;
            }
            cap_moves &= cap_moves - 1;
        }

        while(dp_moves){
            i32 move_sq = getlsb(dp_moves);
            moveList[*idx] = create_move(square, move_sq, DOUBLE_PAWN_PUSH);
            (*idx)++;
            dp_moves &= dp_moves - 1;
        }

        while(ep_moves){
            i32 move_sq = getlsb(ep_moves);
            moveList[*idx] = create_move(square, move_sq, EP_CAPTURE);
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
u64 knightAttacks(i32 square) {
    return knightMoves[square];
}

u64 getKnightAttacks(u64 knights) {
    u64 moves = 0ULL;

    while (knights) {
        i32 square = getlsb(knights);
        moves |= knightMoves[square];
        knights &= knights - 1;
    }

    return moves;
}

u64 getKnightMoves(Position* pos, Turn turn, i32* move_count) {
    u64 all_moves = 0ULL;
    u64 knights = pos->knight[turn];
    u64 ownPieces = pos->color[turn];

    while (knights) {
        i32 square = getlsb(knights);
        all_moves |= knightMoves[square] & ~ownPieces;
        u64 temp = all_moves;
        while(temp){
            (*move_count)++;
            temp &= temp - 1;
        }
        knights &= knights - 1;
    }

    return all_moves;
}

u64 getKnightMovesAppend(u64 knights, u64 ownPieces, u64 oppPieces, Move* moveList, i32* idx) {
    u64 all_moves = 0ULL;

    while (knights) {
        i32 square = getlsb(knights);

        u64 nocap_moves = knightMoves[square] & ~ownPieces;

        all_moves |= nocap_moves;
        u64 cap_moves = nocap_moves & oppPieces;
        nocap_moves &= ~cap_moves;

        while(nocap_moves){
            i32 move_sq = getlsb(nocap_moves);
            moveList[*idx] = create_move(square, move_sq, QUIET);
            (*idx)++;
            nocap_moves &= nocap_moves - 1;
        }
        while(cap_moves){
            i32 move_sq = getlsb(cap_moves);
            moveList[*idx] = create_move(square, move_sq, CAPTURE);
            (*idx)++;
            cap_moves &= cap_moves - 1;
        }
        
        knights &= knights - 1;
    }

    return all_moves;
}

u64 getKnightThreatMovesAppend(u64 knights, u64 ownPieces, u64 oppPieces, i32 opp_king_square, Move* moveList, i32* idx) {
    u64 all_moves = 0ULL;

    u64 check_squares = knightMoves[opp_king_square];

    while (knights) {
        i32 square = getlsb(knights);
    
        all_moves = knightMoves[square] & ~ownPieces;

        u64 check_moves = all_moves & check_squares;
        u64 cap_moves = all_moves & oppPieces;
        check_moves &= ~cap_moves;

        while(check_moves){
            i32 move_sq = getlsb(check_moves);
            moveList[*idx] = create_move(square, move_sq, QUIET);
            (*idx)++;
            check_moves &= check_moves - 1;
        }
        while(cap_moves){
            i32 move_sq = getlsb(cap_moves);
            moveList[*idx] = create_move(square, move_sq, CAPTURE);
            (*idx)++;
            cap_moves &= cap_moves - 1;
        }
        
        knights &= knights - 1;
    }

    return all_moves;
}

//King
u64 kingAttacks(Square sq) {
    return kingMoves[sq];
}

u64 getKingAttacks(u64 kings) {
    i32 square = getlsb(kings);
    return kingMoves[square];
}

u64 getKingMoves(Position* pos, Turn turn, i32* count) {
    u64 kings = pos->king[turn];
    u64 ownPieces = pos->color[turn];
    u64 oppAttackMask = pos->attack_mask[!turn];
    u64 all_moves = 0ULL;

    i32 square = getlsb(kings);
    u64 nocap_moves = kingMoves[square] & ~ownPieces & ~oppAttackMask;
    all_moves |= nocap_moves;

    u64 temp_moves = all_moves;
    while(temp_moves){
        (*count)++;
        temp_moves &= temp_moves - 1;
    }

    return all_moves;
}

u64 getKingMovesAppend(u64 kings, u64 ownPieces, u64 oppPieces, u64 oppAttackMask, Move* moveList, i32* idx) {
    u64 all_moves = 0ULL;

    i32 square = getlsb(kings);
    u64 nocap_moves = kingMoves[square] & ~ownPieces & ~oppAttackMask;
    all_moves |= nocap_moves;

    u64 cap_moves = nocap_moves & oppPieces;
    nocap_moves &= ~cap_moves;

    while(nocap_moves){
        i32 move_sq = getlsb(nocap_moves);
        moveList[*idx] = create_move(square, move_sq, QUIET);
        (*idx)++;
        nocap_moves &= nocap_moves - 1;
    }
    while(cap_moves){
        i32 move_sq = getlsb(cap_moves);
        moveList[*idx] = create_move(square, move_sq, CAPTURE);
        (*idx)++;
        cap_moves &= cap_moves - 1;
    }
    return all_moves;
}

u64 getKingThreatMovesAppend(u64 kings, u64 ownPieces, u64 oppPieces, u64 oppAttackMask, Move* moveList, i32* idx) {
    i32 square = getlsb(kings);
    u64 all_moves = kingMoves[square] & ~ownPieces & ~oppAttackMask;
    u64 cap_moves = all_moves & oppPieces;
    while(cap_moves){
        i32 move_sq = getlsb(cap_moves);
        moveList[*idx] = create_move(square, move_sq, CAPTURE);
        (*idx)++;
        cap_moves &= cap_moves - 1;
    }
    return all_moves;
}

void getCastleMovesAppend(u64 pieces, u64 attack_mask, char flags, Move* moveList, i32* idx){
    if(flags & WHITE_TURN){
        if ((flags & W_SHORT_CASTLE) && !((pieces | attack_mask) & W_SHORT_CASTLE_MASK)) {
            Move move = create_move(4, 6, KING_CASTLE);
            moveList[*idx] = move;
            (*idx)++;
        }

        if ((flags & W_LONG_CASTLE) && !((pieces | (attack_mask & ~B_FILE_MASK)) & W_LONG_CASTLE_MASK)) {
            Move move = create_move(4, 2, QUEEN_CASTLE);
            moveList[*idx] = move;
            (*idx)++;
        }
    }
    else{
        if ((flags & B_SHORT_CASTLE) && !((pieces | attack_mask) & B_SHORT_CASTLE_MASK)) {
            Move move = create_move(60, 62, KING_CASTLE);
            moveList[*idx] = move;
            (*idx)++;
        }

        if ((flags & B_LONG_CASTLE) && !((pieces | (attack_mask & ~B_FILE_MASK)) & B_LONG_CASTLE_MASK)) {
            Move move = create_move(60, 58, QUEEN_CASTLE);
            moveList[*idx] = move;
            (*idx)++;
        }
    }
}

/*
*  Returns all attackerColor pieces attacking a square
*/
u64 getAttackers(Position* pos, i32 square, i32 attackerColor){
    u64 attackers = 0ULL;
    u64 all_pieces = pos->color[0] | pos->color[1];
    i32 pawn_mask_idx = attackerColor ? 4 : 0; //Reverse of normal
    u64 attack_mask;

    attack_mask = rookAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos->queen[attackerColor] | pos->rook[attackerColor]));

    attack_mask = bishopAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos->queen[attackerColor] | pos->bishop[attackerColor]));

    attack_mask = knightMoves[square];
    attackers |= (attack_mask & pos->knight[attackerColor]);

    attack_mask = pawnMoves[square][pawn_mask_idx + 2] | pawnMoves[square][pawn_mask_idx + 3];
    attackers |= (attack_mask & pos->pawn[attackerColor]);

    attack_mask = kingMoves[square];
    attackers |= (attack_mask & pos->king[attackerColor]);

    return attackers;
}

u64 getXRayAttackers(Position* pos, i32 square, i32 attackerColor, u64 removed){
    u64 attackers = 0ULL;
    u64 all_pieces = (pos->color[0] | pos->color[1]) & ~removed;
    u64 attack_mask;

    attack_mask = rookAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos->queen[attackerColor] | pos->rook[attackerColor]));

    attack_mask = bishopAttacks(all_pieces, square);
    attackers |= (attack_mask & (pos->queen[attackerColor] | pos->bishop[attackerColor]));

    return attackers & ~removed;
}

/*
* Here be ye function to get moves for white when they are in check!
*/
void getCheckMovesAppend(Position* pos, Move* moveList, i32* idx){
    i32 turn = pos->flags & WHITE_TURN;
    i32 king_sq = getlsb(pos->king[turn]);
    u64 checker_mask = getAttackers(pos, king_sq, !turn);
    i32 checker_sq = getlsb(checker_mask);
    i32 pawn_mask_idx = turn ? 0 : 4;
    u64 ownPieces = pos->color[turn];
    u64 oppPieces = pos->color[!turn];
    u64 between_squares = betweenMask[king_sq][checker_sq];

    u64 upin = ~pos->pinned; //If in check, only unpinned pieces can moves (i believe havent proven though)

    
    if(pos->en_passant){
        u64 test_mask = turn ? southOne(pos->en_passant) : northOne(pos->en_passant);
        if(test_mask == checker_mask){
            getPawnMovesAppend(pos->pawn[turn] & upin, ~(pos->en_passant), 0ULL, pos->en_passant, pos->flags, moveList, idx); 
        }
    }
    
    u64 pawns = pos->pawn[turn] & upin;
    getPawnMovesAppend(pawns, ~(between_squares | checker_mask), checker_mask, 0ULL, pos->flags, moveList, idx); 
    while (pawns) { //Handle the case of double forward moves
        u64 dp_moves = 0ULL;
        i32 square = getlsb(pawns);
        i32 rank = square / 8;
        char can_double = turn ? (rank == 1) : (rank == 6);
        u64 occ = (ownPieces | oppPieces) & pawnMoves[square][pawn_mask_idx + 0];
        if(occ == 0 && can_double){ //Double Step
            occ = (ownPieces | oppPieces) & pawnMoves[square][pawn_mask_idx + 1];
            if(!occ) dp_moves |= pawnMoves[square][pawn_mask_idx + 1] & (between_squares); //Allow double move to between square
        }
        while(dp_moves){
            i32 move_sq = getlsb(dp_moves);
            moveList[*idx] = create_move(square, move_sq, DOUBLE_PAWN_PUSH);
            (*idx)++;
            dp_moves &= dp_moves - 1;
        }
        pawns &= pawns - 1;
    }

    getKingMovesAppend(    pos->king[turn] & upin, ownPieces, oppPieces, pos->attack_mask[!turn], moveList, idx);

    getKnightMovesAppend(pos->knight[turn] & upin, ~(between_squares | checker_mask), checker_mask, moveList, idx);

    getRookMovesCheckAppend(  pos->queen[turn]  & upin, ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
    getBishopMovesCheckAppend(pos->queen[turn]  & upin, ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
    getRookMovesCheckAppend(  pos->rook[turn]   & upin, ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
    getBishopMovesCheckAppend(pos->bishop[turn] & upin, ownPieces, oppPieces, (between_squares | checker_mask), moveList, idx);
}



/*
*
*   Here lay the Pinned Move functions xDDDDD lmao
*
*/
static void getPinnedQueenMovesAppend(i32 king_rank, i32 king_file, u64 pinned_queens, u64 ownPieces, u64 oppPieces, Move* moveList, i32* size) {
    while(pinned_queens){ //Process Each Pinned Queen Individually
        i32 queen_sq = getlsb(pinned_queens);
        i32 queen_rank = queen_sq / 8;
        i32 queen_file = queen_sq % 8;
        if(queen_rank == king_rank)                                getRookMovesAppend(1ULL << queen_sq, ownPieces | fileMask[queen_sq], oppPieces, moveList, size);
        else if(queen_file == king_file)                           getRookMovesAppend(1ULL << queen_sq, ownPieces | rankMask[queen_sq], oppPieces, moveList, size);
        else if(queen_rank - queen_file == king_rank - king_file)  getBishopMovesAppend(1ULL << queen_sq, ownPieces | NWSEMask[queen_sq], oppPieces, moveList, size);
        else if(queen_rank + queen_file == king_rank + king_file)  getBishopMovesAppend(1ULL << queen_sq, ownPieces | NESWMask[queen_sq], oppPieces, moveList, size);
        pinned_queens &= pinned_queens - 1;
    }
}

static void getPinnedQueenThreatMovesAppend(i32 king_rank, i32 king_file, u64 pinned_queens, u64 ownPieces, u64 oppPieces, u64 b_check_squares, u64 r_check_squares, Move* moveList, i32* size) {
    while(pinned_queens){ //Process Each Pinned Queen Individually
        i32 queen_sq = getlsb(pinned_queens);
        i32 queen_rank = queen_sq / 8;
        i32 queen_file = queen_sq % 8;
        if(queen_rank == king_rank)                                getRookThreatMovesAppend(1ULL << queen_sq, ownPieces | fileMask[queen_sq], oppPieces, r_check_squares, moveList, size);
        else if(queen_file == king_file)                           getRookThreatMovesAppend(1ULL << queen_sq, ownPieces | rankMask[queen_sq], oppPieces, r_check_squares, moveList, size);
        else if(queen_rank - queen_file == king_rank - king_file)  getBishopThreatMovesAppend(1ULL << queen_sq, ownPieces | NWSEMask[queen_sq], oppPieces, b_check_squares, moveList, size);
        else if(queen_rank + queen_file == king_rank + king_file)  getBishopThreatMovesAppend(1ULL << queen_sq, ownPieces | NESWMask[queen_sq], oppPieces, b_check_squares, moveList, size);
        pinned_queens &= pinned_queens - 1;
    }
}

static void getPinnedRookMovesAppend(i32 king_rank, i32 king_file, u64 pinned_rooks, u64 ownPieces, u64 oppPieces, Move* moveList, i32* size) {
    while(pinned_rooks){ //Process Each Pinned rook Individually
        i32 rook_sq = getlsb(pinned_rooks);
        i32 rook_rank = rook_sq / 8;
        i32 rook_file = rook_sq % 8;
        if(rook_rank == king_rank)      getRookMovesAppend(1ULL << rook_sq, ownPieces | fileMask[rook_sq], oppPieces, moveList, size);
        else if(rook_file == king_file) getRookMovesAppend(1ULL << rook_sq, ownPieces | rankMask[rook_sq], oppPieces, moveList, size);
        pinned_rooks &= pinned_rooks - 1;
    }
}

static void getPinnedRookThreatMovesAppend(i32 king_rank, i32 king_file, u64 pinned_rooks, u64 ownPieces, u64 oppPieces, u64 r_check_squares, Move* moveList, i32* size) {
    while(pinned_rooks){ //Process Each Pinned rook Individually
        i32 rook_sq = getlsb(pinned_rooks);
        i32 rook_rank = rook_sq / 8;
        i32 rook_file = rook_sq % 8;
        if(rook_rank == king_rank)      getRookThreatMovesAppend(1ULL << rook_sq, ownPieces | fileMask[rook_sq], oppPieces, r_check_squares, moveList, size);
        else if(rook_file == king_file) getRookThreatMovesAppend(1ULL << rook_sq, ownPieces | rankMask[rook_sq], oppPieces, r_check_squares, moveList, size);
        pinned_rooks &= pinned_rooks - 1;
    }
}


static void getPinnedBishopMovesAppend(i32 king_rank, i32 king_file, u64 pinned_bishops, u64 ownPieces, u64 oppPieces, Move* moveList, i32* size) {
    while(pinned_bishops){ //Process Each Pinned bishop Individually
        i32 bishop_sq = getlsb(pinned_bishops);
        i32 bishop_rank = bishop_sq / 8;
        i32 bishop_file = bishop_sq % 8;

        if(bishop_rank - bishop_file == king_rank - king_file){
            getBishopMovesAppend(1ULL << bishop_sq, ownPieces | NWSEMask[bishop_sq], oppPieces, moveList, size);
        }
        
        else if(bishop_rank + bishop_file == king_rank + king_file){
            getBishopMovesAppend(1ULL << bishop_sq, ownPieces | NESWMask[bishop_sq], oppPieces, moveList, size);
        }
        pinned_bishops &= pinned_bishops - 1;
    }
}

static void getPinnedBishopThreatMovesAppend(i32 king_rank, i32 king_file, u64 pinned_bishops, u64 ownPieces, u64 oppPieces, u64 b_check_squares, Move* moveList, i32* size) {
    while(pinned_bishops){ //Process Each Pinned bishop Individually
        i32 bishop_sq = getlsb(pinned_bishops);
        i32 bishop_rank = bishop_sq / 8;
        i32 bishop_file = bishop_sq % 8;

        if(bishop_rank - bishop_file == king_rank - king_file){
            getBishopThreatMovesAppend(1ULL << bishop_sq, ownPieces | NWSEMask[bishop_sq], oppPieces, b_check_squares, moveList, size);
        }
        
        else if(bishop_rank + bishop_file == king_rank + king_file){
            getBishopThreatMovesAppend(1ULL << bishop_sq, ownPieces | NESWMask[bishop_sq], oppPieces, b_check_squares, moveList, size);
        }
        pinned_bishops &= pinned_bishops - 1;
    }
}

static void getPinnedPawnMovesAppend(i32 king_rank, i32 king_file, u64 pinned_pawns, u64 ownPieces, u64 oppPieces, u64 en_passant, char flags, Move* moveList, i32* size) {
    while(pinned_pawns){ //Process Each Pinned pawn Individually
        i32 pawn_sq = getlsb(pinned_pawns);
        i32 pawn_rank = pawn_sq / 8;
        i32 pawn_file = pawn_sq % 8;
        char turn = (flags & WHITE_TURN);
        i32 pawn_mask_idx = turn ? 0 : 4;

        if(pawn_rank == king_rank){
            if(en_passant && (pawn_rank == (turn ? 4 : 3))){
                i32 ep_sq = getlsb(pinned_pawns);
                if(ep_sq % 8 == pawn_file){
                    getPawnMovesAppend(1ULL << pawn_sq, ownPieces, oppPieces, 0ULL, flags, moveList, size);
                    pinned_pawns &= pinned_pawns - 1;
                    continue;
                }
            }
        }

        if(pawn_file == king_file) getPawnMovesAppend(1ULL << pawn_sq, ownPieces, (oppPieces & ~pawnMoves[pawn_sq][pawn_mask_idx + 2] & ~pawnMoves[pawn_sq][pawn_mask_idx + 3]), 0ULL, flags, moveList, size); //Hide black pieces that could be captured and no en-passant
        else if(pawn_rank - pawn_file == king_rank - king_file){ //Up right
            u64 dir = pawnMoves[pawn_sq][turn ? 3 : 6];
            u64 occ = oppPieces & dir;
            getPawnMovesAppend(1ULL << pawn_sq, (ownPieces | ~occ), occ, en_passant & dir, flags, moveList, size);
        }
        else if(pawn_rank + pawn_file == king_rank + king_file){ //Up Left
            u64 dir = pawnMoves[pawn_sq][turn ? 2 : 7]; 
            u64 occ = oppPieces & dir;
            getPawnMovesAppend(1ULL << pawn_sq, (ownPieces | ~occ), occ, en_passant & dir, flags, moveList, size);
        }
        pinned_pawns &= pinned_pawns - 1;
    }
}

static void getPinnedPawnThreatMovesAppend(i32 king_rank, i32 king_file, u64 pinned_pawns, u64 ownPieces, u64 oppPieces, u64 en_passant, char flags, i32 opp_king_square, Move* moveList, i32* size) {
    while(pinned_pawns){ //Process Each Pinned pawn Individually
        i32 pawn_sq = getlsb(pinned_pawns);
        i32 pawn_rank = pawn_sq / 8;
        i32 pawn_file = pawn_sq % 8;
        char turn = (flags & WHITE_TURN);
        i32 pawn_mask_idx = turn ? 0 : 4;

        if(pawn_rank == king_rank){
            if(en_passant && (pawn_rank == (turn ? 4 : 3))){
                i32 ep_sq = getlsb(pinned_pawns);
                if(ep_sq % 8 == pawn_file){
                    getPawnThreatMovesAppend(1ULL << pawn_sq, ownPieces, oppPieces, 0ULL, flags, opp_king_square, moveList, size);
                    pinned_pawns &= pinned_pawns - 1;
                    continue;
                }
            }
        }

        if(pawn_file == king_file) getPawnThreatMovesAppend(1ULL << pawn_sq, ownPieces, (oppPieces & ~pawnMoves[pawn_sq][pawn_mask_idx + 2] & ~pawnMoves[pawn_sq][pawn_mask_idx + 3]), 0ULL, flags, opp_king_square, moveList, size); //Hide black pieces that could be captured and no en-passant
        else if(pawn_rank - pawn_file == king_rank - king_file){ //Up right
            u64 dir = pawnMoves[pawn_sq][turn ? 3 : 6];
            u64 occ = oppPieces & dir;
            getPawnThreatMovesAppend(1ULL << pawn_sq, (ownPieces | ~occ), occ, en_passant & dir, flags, opp_king_square, moveList, size);
        }
        else if(pawn_rank + pawn_file == king_rank + king_file){ //Up Left
            u64 dir = pawnMoves[pawn_sq][turn ? 2 : 7]; 
            u64 occ = oppPieces & dir;
            getPawnThreatMovesAppend(1ULL << pawn_sq, (ownPieces | ~occ), occ, en_passant & dir, flags, opp_king_square, moveList, size);
        }
        pinned_pawns &= pinned_pawns - 1;
    }
}

void getPinnedMovesAppend(Position* pos, Move* moveList, i32* size){
    i32 turn = pos->flags & WHITE_TURN;
    u64 pinned = pos->pinned;
    i32 king_sq = getlsb(pos->king[turn]);
    i32 king_rank = king_sq / 8;
    i32 king_file = king_sq % 8;

    //King does his thang
    getKingMovesAppend(pos->king[turn], pos->color[turn],  pos->color[!turn], pos->attack_mask[!turn], moveList, size);
    getCastleMovesAppend(pos->color[0] | pos->color[1], pos->attack_mask[!turn], pos->flags, moveList, size);

    //Pinned Knights Cannot Move
    u64 pinned_knights = pos->knight[turn] & pinned;
    getKnightMovesAppend(pos->knight[turn] & ~pinned_knights, pos->color[turn], pos->color[!turn], moveList, size);
    
    u64 pinned_queens = pos->queen[turn] & pinned;
    getBishopMovesAppend(pos->queen[turn] & ~pinned_queens, pos->color[turn], pos->color[!turn], moveList, size);
    getRookMovesAppend(  pos->queen[turn] & ~pinned_queens, pos->color[turn], pos->color[!turn], moveList, size);
    getPinnedQueenMovesAppend(king_rank, king_file, pinned_queens, pos->color[turn], pos->color[!turn], moveList, size);
    
    u64 pinned_rooks = pos->rook[turn] & pinned;
    getRookMovesAppend(pos->rook[turn] & ~pinned_rooks, pos->color[turn], pos->color[!turn], moveList, size);
    getPinnedRookMovesAppend(king_rank, king_file, pinned_rooks, pos->color[turn], pos->color[!turn], moveList, size);

    //Process Pinned Bishops
    u64 pinned_bishops = pos->bishop[turn] & pinned;
    getBishopMovesAppend(pos->bishop[turn] & ~pinned_bishops, pos->color[turn], pos->color[!turn], moveList, size);
    getPinnedBishopMovesAppend(king_rank, king_file, pinned_bishops, pos->color[turn], pos->color[!turn], moveList, size);


    //Process Pinned Pawns
    u64 pinned_pawns = pos->pawn[turn] & pinned;
    getPawnMovesAppend(pos->pawn[turn] & ~pinned_pawns, pos->color[turn], pos->color[!turn], pos->en_passant, pos->flags, moveList, size);
    getPinnedPawnMovesAppend(king_rank, king_file, pinned_pawns, pos->color[turn], pos->color[!turn], pos->en_passant, pos->flags, moveList, size);
}

void getPinnedThreatMovesAppend(Position* pos, u64 r_check_squares, u64 b_check_squares, i32 opp_king_sq, Move* moveList, i32* size){
    i32 turn = pos->flags & TURN_MASK;
    u64 pinned = pos->pinned;
    i32 king_sq = getlsb(pos->king[turn]);
    i32 king_rank = king_sq / 8;
    i32 king_file = king_sq % 8;

    //King does his thang
    getKingThreatMovesAppend(pos->king[turn], pos->color[turn],  pos->color[!turn], pos->attack_mask[!turn], moveList, size);

    //Pinned Knights Cannot Move
    u64 pinned_knights = pos->knight[turn] & pinned;
    getKnightThreatMovesAppend(pos->knight[turn] & ~pinned_knights, pos->color[turn], pos->color[!turn], opp_king_sq, moveList, size);
    
    u64 pinned_queens = pos->queen[turn] & pinned;
    getBishopThreatMovesAppend(pos->queen[turn] & ~pinned_queens, pos->color[turn], pos->color[!turn], b_check_squares, moveList, size);
    getRookThreatMovesAppend(  pos->queen[turn] & ~pinned_queens, pos->color[turn], pos->color[!turn], r_check_squares, moveList, size);
    getPinnedQueenThreatMovesAppend(king_rank, king_file, pinned_queens, pos->color[turn], pos->color[!turn], b_check_squares, r_check_squares,  moveList, size);
    
    u64 pinned_rooks = pos->rook[turn] & pinned;
    getRookThreatMovesAppend(pos->rook[turn] & ~pinned_rooks, pos->color[turn], pos->color[!turn], r_check_squares, moveList, size);
    getPinnedRookThreatMovesAppend(king_rank, king_file, pinned_rooks, pos->color[turn], pos->color[!turn], r_check_squares, moveList, size);

    //Process Pinned Bishops
    u64 pinned_bishops = pos->bishop[turn] & pinned;
    getBishopThreatMovesAppend(pos->bishop[turn] & ~pinned_bishops, pos->color[turn], pos->color[!turn], b_check_squares, moveList, size);
    getPinnedBishopThreatMovesAppend(king_rank, king_file, pinned_bishops, pos->color[turn], pos->color[!turn], b_check_squares, moveList, size);

    //Process Pinned Pawns
    u64 pinned_pawns = pos->pawn[turn] & pinned;
    getPawnThreatMovesAppend(pos->pawn[turn] & ~pinned_pawns, pos->color[turn], pos->color[!turn], pos->en_passant, pos->flags, opp_king_sq, moveList, size);
    getPinnedPawnThreatMovesAppend(king_rank, king_file, pinned_pawns, pos->color[turn], pos->color[!turn], pos->en_passant, pos->flags, opp_king_sq, moveList, size);
}