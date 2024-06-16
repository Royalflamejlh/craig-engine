//
//  bitboard.h
//  godengine
//
//  Created by John Howard on 11/22/23.
//

#ifndef bitboard_h
#define bitboard_h
#include <stdint.h>
#include <stdio.h>
#include "bbutils.h"
#include "../types.h"

void generateMasks(void);

u64 getKnightAttacks(u64 knights);
u64 getKnightMovesAppend(u64 knights, u64 ownPieces, u64 oppPieces, Move* moveList, i32* idx);
u64 getKnightThreatMovesAppend(u64 knights, u64 ownPieces, u64 oppPieces, i32 opp_king_square, Move* moveList, i32* idx);

u64 getBishopAttacks(u64 bishops, u64 ownPieces, u64 oppPieces);
u64 getBishopMovesAppend(u64 bishops, u64 ownPieces, u64 oppPieces, Move* moveList, i32* idx);
u64 getBishopThreatMovesAppend(u64 bishops, u64 ownPieces, u64 oppPieces, u64 checkSquares, Move* moveList, i32* idx);

u64 getRookAttacks(u64 rooks, u64 ownPieces, u64 oppPieces);
u64 getRookMovesAppend(u64 rooks, u64 ownPieces, u64 oppPieces, Move* moveList, i32* idx);
u64 getRookThreatMovesAppend(u64 rooks, u64 ownPieces, u64 oppPieces, u64 checkSquares, Move* moveList, i32* idx);

u64 getPawnAttacks(u64 pawns, char flags);
u64 getPawnMovesAppend(u64 pawns, u64 ownPieces, u64 oppPieces,  u64 enPassant, char flags, Move* moveList, i32* idx);
u64 getPawnThreatMovesAppend(u64 pawns, u64 ownPieces, u64 oppPieces,  u64 enPassant, char flags, i32 opp_king_square, Move* moveList, i32* idx);

u64 getKingAttacks(u64 kings);
u64 getKingMoves(Position* pos, Turn turn, i32* count);
u64 getKingMovesAppend(u64 kings, u64 ownPieces, u64 oppPieces, u64 oppAttackMask, Move* moveList, i32* idx);
u64 getKingThreatMovesAppend(u64 kings, u64 ownPieces, u64 oppPieces, u64 oppAttackMask, Move* moveList, i32* idx);

void getCastleMovesAppend(u64 white, u64 b_attack_mask, char flags, Move* moveList, i32* idx);

void getCheckMovesAppend(Position* position, Move* moveList, i32* idx);

void getPinnedMovesAppend(Position* position, Move* moveList, i32* idx);
void getPinnedThreatMovesAppend(Position* position, u64 r_check_squares, u64 b_check_squares, i32 kingSq, Move* moveList, i32* idx);

u64 getAttackers(Position* pos, i32 square, i32 attackerColor);
u64 getXRayAttackers(Position* pos, i32 square, i32 attackerColor, u64 removed);

u64 generateAttacks(Position* position, i32 turn);

static inline void setAttackMasks(Position* pos){
    pos->attack_mask[1] = generateAttacks(pos, 1);
    pos->attack_mask[0] = generateAttacks(pos, 0);
}
#endif /* bitboard_h */
