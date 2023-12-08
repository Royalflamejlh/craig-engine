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

uint64_t generateWhiteAttacks(Position position){
    uint64_t attack_mask = 0ULL;
    attack_mask |= getBishopAttacks(position.w_bishop, position.white, position.black);
    attack_mask |= getRookAttacks(  position.w_rook,   position.white, position.black);
    attack_mask |= getBishopAttacks(position.w_queen,  position.white, position.black);
    attack_mask |= getRookAttacks(  position.w_queen,  position.white, position.black);
    attack_mask |= getKnightAttacks(position.w_knight, position.white);
    attack_mask |= getKingAttacks(  position.w_king,   position.white);
    attack_mask |= getPawnAttacks(  position.w_pawn,   position.white, position.black, position.en_passant, position.flags);
    return attack_mask;
}

uint64_t generateBlackAttacks(Position position){
    uint64_t attack_mask = 0ULL;
    attack_mask |= getBishopAttacks(position.b_bishop, position.black, position.white);
    attack_mask |= getRookAttacks(  position.b_rook,   position.black, position.white);
    attack_mask |= getBishopAttacks(position.b_queen,  position.black, position.white);
    attack_mask |= getRookAttacks(  position.b_queen,  position.black, position.white);
    attack_mask |= getKnightAttacks(position.b_knight, position.black);
    attack_mask |= getKingAttacks(  position.b_king,   position.black);
    attack_mask |= getPawnAttacks(  position.b_pawn,   position.black, position.white, position.en_passant, position.flags);
    return attack_mask;
}

static uint64_t generateWhiteMoves(Position position, Move* moveList, int* size){
    
    //Check for hashed moves
    //If in check, call check move generator
    uint64_t attack_mask = 0ULL;
    attack_mask |= getBishopMovesAppend(position.w_bishop, position.white, position.black, moveList, size);
    attack_mask |= getRookMovesAppend(  position.w_rook,   position.white, position.black, moveList, size);
    attack_mask |= getBishopMovesAppend(position.w_queen,  position.white, position.black, moveList, size);
    attack_mask |= getRookMovesAppend(  position.w_queen,  position.white, position.black, moveList, size);
    attack_mask |= getKnightMovesAppend(position.w_knight, position.white, position.black, moveList, size);
    attack_mask |= getKingMovesAppend(  position.w_king,   position.white, position.black, moveList, size);
    attack_mask |= getPawnMovesAppend(  position.w_pawn,   position.white, position.black, position.en_passant, position.flags, moveList, size);
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
    attack_mask |= getRookMovesAppend(  position.b_rook,   position.black, position.white, moveList, size);
    attack_mask |= getBishopMovesAppend(position.b_queen,  position.black, position.white, moveList, size);
    attack_mask |= getRookMovesAppend(  position.b_queen,  position.black, position.white, moveList, size);
    attack_mask |= getKnightMovesAppend(position.b_knight, position.black, position.white, moveList, size);
    attack_mask |= getKingMovesAppend(  position.b_king,   position.black, position.white, moveList, size);
    attack_mask |= getPawnMovesAppend(  position.b_pawn,   position.black, position.white, position.en_passant, position.flags, moveList, size);
    return attack_mask;
}

static void updateMask(uint64_t fromMask, uint64_t toMask, uint64_t *pieceSet, uint64_t *pieceColorSet) {
    *pieceSet       &= ~fromMask;
    *pieceSet       |= toMask;
    *pieceColorSet  &= ~fromMask;
    *pieceColorSet  |= toMask;
}


void makeMove(Position *position, Move move){
    switch(GET_FLAGS(move)){
        case QUEEN_PROMO_CAPTURE:
        case ROOK_PROMO_CAPTURE:
        case BISHOP_PROMO_CAPTURE:
        case KNIGHT_PROMO_CAPTURE:
        case QUEEN_PROMOTION:
        case ROOK_PROMOTION:
        case BISHOP_PROMOTION:
        case KNIGHT_PROMOTION:
        case EP_CAPTURE:
        case CAPTURE:
        case QUEEN_CASTLE:
        case KING_CASTLE:
        case DOUBLE_PAWN_PUSH:
        case QUIET:
        default:
            break;
    }

    char from = GET_FROM(move);
    char to   = GET_TO(move);

    char from_piece = position->charBoard[GET_FROM(move)];
    char to_piece   = position->charBoard[GET_TO(move)];

    switch(from_piece){
        case 'Q':
            //movePiece(from, to, from_piece, to_piece, &position->w_queen, &position->white, position);
            //Needs to update the states on all the bitboards
            //Use the attack mask to see if the move puts the opponent in check and set flags accordinly
            //If moving along a king ray, it needs to update the pinned pieces
            break;
        case 'K':
            break;
        case 'N':
            break;
        case 'B':
            break;
        case 'R':
            break;
        case 'P':
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