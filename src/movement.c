#include "types.h"
#include "./bitboard/bitboard.h"

static void generateWhiteMoves(Position position, Move* moveList, int* size);
static void generateBlackMoves(Position position, Move* moveList, int* size);


void generateMoves(Position position,  Move* moveList, int* size){
    printf("Generating moves\n");
    *size = 0;

    if(position.flags & WHITE_TURN){
        printf("Generating moves for black\n");
        generateWhiteMoves(position, moveList, size);
    }
    else{
        printf("Generating moves for black\n");
        generateBlackMoves(position, moveList, size);
    }

}

static void generateWhiteMoves(Position position, Move* moveList, int* size){
    //Check for hashed moves
    //Check for capturing moves
    //Check for killer moves
    //Check rest of moves
    getBishopMovesAppend(position.w_bishop, position.white, position.black, moveList, size);
}

static void generateBlackMoves(Position position, Move* moveList, int* size){
    //Check for hashed moves
    //Check for capturing moves
    //Check for killer moves
    //Check rest of moves
    getBishopMovesAppend(position.b_bishop, position.black, position.white, moveList, size);
}