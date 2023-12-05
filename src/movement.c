#include "types.h"
#include "./bitboard/bitboard.h"

static uint64_t generateWhiteMoves(Position position, Move* moveList, int* size);
static uint64_t generateBlackMoves(Position position, Move* moveList, int* size);


uint16_t generateLegalMoves(Position position,  Move* moveList, int* size){
    *size = 0;
    if(position.flags & WHITE_TURN){
        generateWhiteMoves(position, moveList, size);
        return *size;
    }
    else{
        generateBlackMoves(position, moveList, size);
        return *size;
    }
}


static uint64_t generateWhiteMoves(Position position, Move* moveList, int* size){
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
    getCastleMovesWhiteAppend(position.white, position.flags, moveList, size);
    return attack_mask;
}

static uint64_t generateBlackMoves(Position position, Move* moveList, int* size){
    //Check for hashed moves
    //Check for capturing moves
    //Check for killer moves
    //Check rest of moves
    uint64_t attack_mask = 0ULL;
    attack_mask |= getBishopMovesAppend(position.b_bishop, position.black, position.white, moveList, size);
    attack_mask |= getRookMovesAppend(position.b_rook, position.black, position.white, moveList, size);
    attack_mask |= getBishopMovesAppend(position.b_queen, position.black, position.white, moveList, size);
    attack_mask |= getRookMovesAppend(position.b_queen, position.black, position.white, moveList, size);
    attack_mask |= getKnightMovesAppend(position.b_knight, position.black, moveList, size);
    attack_mask |= getKingMovesAppend(position.b_king, position.black, moveList, size);
    attack_mask |= getPawnMovesAppend(position.b_pawn, position.black, position.white, position.en_passant, position.flags, moveList, size);
    return attack_mask;
}

static void updatePosition(Position *position, uint64_t fromMask, uint64_t toMask, uint64_t *pieceSet) {
    position->charBoard[0] = 'A';//TODO: Stuff
    *pieceSet &= ~fromMask;
    *pieceSet |= toMask;
}


void makeMove(Position *position, Move move){
    if(move & MOVE_CASTLE_MASK){
        //Castle Logic;
    }
    if(move & MOVE_PROMOTION_MASK){
        //Promotion logic
    }
    
    uint64_t from = 0x1ULL << GET_FROM(move);
    uint64_t to = 0x1ULL << GET_TO(move);

    char from_piece = position->charBoard[GET_FROM(move)];

    switch(from_piece){
        case 'Q':
            updatePosition(position, from, to, &position->w_queen);
            updatePosition(position, from, to, &position->white);
            break;
        case 'K':
            position->w_king   |= to;
            position->white    |= to;
            break;
        case 'N':
            position->w_knight |= to;
            position->white    |= to;
            break;
        case 'B':
            position->w_bishop |= to;
            position->white    |= to;
            break;
        case 'R':
            position->w_rook   |= to;
            position->white    |= to;
            break;
        case 'P':
            position->w_pawn   |= to;
            position->white    |= to;
            break;
    }
}

static inline void clearBlackSquare(Position *position, uint64_t to){
    position->black    &= ~to; 
    position->b_pawn   &= ~to;
    position->b_queen  &= ~to;
    position->b_bishop &= ~to;
    position->b_knight &= ~to;
    position->b_rook   &= ~to;
}


void unmakeMove(Position *position, Move move){
    char from_piece = position->charBoard[GET_FROM(move)];
    switch(from_piece){
    }
}