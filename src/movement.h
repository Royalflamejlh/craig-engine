#ifndef MOVEMENT_H
#define MOVEMENT_H
#endif

uint16_t generateLegalMoves(Position position,  Move* moveList, int* size);
uint64_t generateWhiteAttacks(Position position);
uint64_t generateBlackAttacks(Position position);
uint64_t generatePinnedPieces(Position pos);