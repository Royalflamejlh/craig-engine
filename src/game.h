#ifndef GAME_H
#define GAME_H
#include "types.h"

GameState* initGameState(Position pos);
int updateGameState(GameState* gamestate, Move move);
int popGameState(GameState* gameState, Move move);
int freeGameState();

#endif
