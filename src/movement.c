#include "types.h"
#include "./bitboard/magic.h"
#include "./bitboard/bitboard.h"
#include "./bitboard/bbutils.h"

static void generateWhiteMoves(Position position, Move* moveList, int* size);
static void generateBlackMoves(Position position, Move* moveList, int* size);


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
    attack_mask |= getPawnAttacks(  position.w_pawn,   position.white, WHITE_TURN);
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
    attack_mask |= getPawnAttacks(  position.b_pawn,   position.black, ~WHITE_TURN);
    return attack_mask;
}

static void generateWhiteMoves(Position position, Move* moveList, int* size){
    
    //Check for hashed moves
    //If in check, call check move generator

    if(position.flags & IN_CHECK){
        if(position.flags & IN_D_CHECK){
            getKingMovesAppend(position.w_king, position.white, position.black, position.b_attack_mask, moveList, size);
        }
        else{
            getCheckMovesWhiteAppend(position, moveList, size);
        }
    }
    else{
        getCastleMovesWhiteAppend(position.white, position.flags, moveList, size);

        getBishopMovesAppend(position.w_queen,  position.white, position.black, moveList, size);
        getRookMovesAppend(  position.w_queen,  position.white, position.black, moveList, size);
        getRookMovesAppend(  position.w_rook,   position.white, position.black, moveList, size);
        getBishopMovesAppend(position.w_bishop, position.white, position.black, moveList, size);
        getKnightMovesAppend(position.w_knight, position.white, position.black, moveList, size);
        getKingMovesAppend(  position.w_king,   position.white, position.black, position.b_attack_mask, moveList, size);
        getPawnMovesAppend(  position.w_pawn,   position.white, position.black, position.en_passant, position.flags, moveList, size);
    }

}

static void generateBlackMoves(Position position, Move* moveList, int* size){
    //Check for hashed moves
    //Check for capturing moves
    //Check for killer moves
    //Check rest of movesgetBishopMovesAppend(position.b_bishop, position.black, position.white, moveList, size);
    //getRookMovesAppend(  position.b_rook,   position.black, position.white, moveList, size);
    //getBishopMovesAppend(position.b_queen,  position.black, position.white, moveList, size);
    //getRookMovesAppend(  position.b_queen,  position.black, position.white, moveList, size);
    //getKnightMovesAppend(position.b_knight, position.black, position.white, moveList, size);
    //getKingMovesAppend(  position.b_king,   position.black, position.white, moveList, size);
    //getPawnMovesAppend(  position.b_pawn,   position.black, position.white, position.en_passant, position.flags, moveList, size);

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

    int from = GET_FROM(move);
    //int to   = GET_TO(move);

    char from_piece = position->charBoard[from];
    //char to_piece   = position->charBoard[to];

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

static uint64_t generatePinnedPiecesWhite(Position pos){
    int k_square = __builtin_ctzll(pos.w_king);
    //Lets do white first!
    uint64_t pos_pinners;
    uint64_t all_pieces = pos.white | pos.black;  //Contains all the initial pieces
    uint64_t pinned = pos.white;  //The peices belonging to white which become pinnged
    uint64_t d_attack_mask, h_attack_mask, attack_mask = 0ULL;  // A attack masks
    uint64_t pin_directions = 0ULL;
    
    attack_mask |= rookAttacks(all_pieces, k_square);    //Get a bb of squares being attacked by the king
    attack_mask |= bishopAttacks(all_pieces, k_square);

    pinned &= attack_mask; // The pinned white pieces now contains the ones with a ray to the king

    //Now get all pieces under attack as if king is queen again and these are the possible pinners
    h_attack_mask = rookAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = h_attack_mask & (pos.b_queen | pos.b_rook);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    d_attack_mask = bishopAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = d_attack_mask & (pos.b_queen | pos.b_bishop);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    pinned &= pin_directions; //Only keep the pieces that are actually pinned
    return pinned;
}

static uint64_t generatePinnedPiecesBlack(Position pos){
    int k_square = __builtin_ctzll(pos.b_king);
    //Lets do white first!
    uint64_t pos_pinners;
    uint64_t all_pieces = pos.white | pos.black;  //Contains all the initial pieces
    uint64_t pinned = pos.black;  //The peices belonging to white which become pinnged
    uint64_t d_attack_mask, h_attack_mask, attack_mask = 0ULL;  // A attack masks
    uint64_t pin_directions = 0ULL;
    
    attack_mask |= rookAttacks(all_pieces, k_square);    //Get a bb of squares being attacked by the king
    attack_mask |= bishopAttacks(all_pieces, k_square);

    pinned &= attack_mask; // The pinned white pieces now contains the ones with a ray to the king

    //Now get all pieces under attack as if king is queen again and these are the possible pinners
    h_attack_mask = rookAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = h_attack_mask & (pos.w_queen | pos.w_rook);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    d_attack_mask = bishopAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = d_attack_mask & (pos.w_queen | pos.w_bishop);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    pinned &= pin_directions; //Only keep the pieces that are actually pinned
    return pinned;
}

uint64_t generatePinnedPieces(Position pos){
    return generatePinnedPiecesBlack(pos) | generatePinnedPiecesWhite(pos);
}

