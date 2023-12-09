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

uint64_t getKnightAttacks(uint64_t knights, uint64_t ownPieces);
uint64_t getKnightMovesAppend(uint64_t knights, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx);

uint64_t getBishopAttacks(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces);
uint64_t getBishopMovesAppend(uint64_t bishops, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx);

uint64_t getRookAttacks(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces);
uint64_t getRookMovesAppend(uint64_t rooks, uint64_t ownPieces, uint64_t oppPieces, Move* moveList, int* idx);

uint64_t getPawnAttacks(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, char flags);
uint64_t getPawnMovesAppend(uint64_t pawns, uint64_t ownPieces, uint64_t oppPieces,  uint64_t enPassant, char flags, Move* moveList, int* idx);

uint64_t getKingAttacks(uint64_t kings, uint64_t ownPieces);
uint64_t getKingMovesAppend(uint64_t kings, uint64_t ownPieces, uint64_t oppPieces, uint64_t oppAttackMask, Move* moveList, int* idx);

void getCastleMovesWhiteAppend(uint64_t white, char flags, Move* moveList, int* idx);


void getCheckMovesWhiteAppend(Position position, Move* moveList, int* idx);

uint64_t getWhiteAttackers(Position pos, int square);
uint64_t getBlackAttackers(Position pos, int square);
#endif /* bitboard_h */
