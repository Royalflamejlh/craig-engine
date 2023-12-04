#include "types.h"
#include "./bitboard/bitboard.h"

static void generateWhiteMoves(Position position, Move* moveList, int* size);
static void generateBlackMoves(Position position, Move* moveList, int* size);


void generateMoves(Position position,  Move* moveList, int* size){
    printf("Generating moves\n");
    *size = 0;

    if(position.flags & WHITE_TURN){
        printf("Generating moves for white\n");
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
    uint64_t attack_mask = 0ULL;
    attack_mask |= getBishopMovesAppend(position.w_bishop, position.white, position.black, moveList, size);
    attack_mask |= getRookMovesAppend(position.w_rook, position.white, position.black, moveList, size);
    attack_mask |= getBishopMovesAppend(position.w_queen, position.white, position.black, moveList, size);
    attack_mask |= getRookMovesAppend(position.w_queen, position.white, position.black, moveList, size);
    attack_mask |= getKnightMovesAppend(position.w_knight, position.white, moveList, size);
    attack_mask |= getKingMovesAppend(position.w_king, position.white, moveList, size);
    attack_mask |= getPawnMovesAppend(position.w_pawn, position.white, position.black, position.en_passant, position.flags, moveList, size);
}

static void generateBlackMoves(Position position, Move* moveList, int* size){
    //Check for hashed moves
    //Check for capturing moves
    //Check for killer moves
    //Check rest of moves
    getBishopMovesAppend(position.b_bishop, position.black, position.white, moveList, size);
}