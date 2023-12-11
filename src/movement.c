#include "types.h"
#include "./bitboard/magic.h"
#include "./bitboard/bitboard.h"
#include "./bitboard/bbutils.h"


/*
* Wow Cool! Totaly sensical move generation!
*/
uint16_t generateLegalMoves(Position position,  Move* moveList){
    int size[] = {0};
    int turn = position.flags & WHITE_TURN; //True for white false for black
    uint64_t ownPos = position.color[turn];
    uint64_t oppPos = position.color[!turn];
    uint64_t oppAttackMask = position.attack_mask[!turn];

    if(position.flags & IN_CHECK){
        if(position.flags & IN_D_CHECK){
            getKingMovesAppend(position.king[turn], ownPos, oppPos, oppAttackMask, moveList, size);
        }
        else{
            getCheckMovesAppend(position, moveList, size);
        }
    }
    else if(position.pinned & ownPos){
        getPinnedMovesAppend(position, moveList, size);
    }
    else{
        getCastleMovesAppend(ownPos, oppAttackMask, position.flags, moveList, size);

        getBishopMovesAppend(position.queen[turn],  ownPos, oppPos, moveList, size);
        getRookMovesAppend(  position.queen[turn],  ownPos, oppPos, moveList, size);
        getRookMovesAppend(  position.rook[turn],   ownPos, oppPos, moveList, size);
        getBishopMovesAppend(position.bishop[turn], ownPos, oppPos, moveList, size);
        getKnightMovesAppend(position.knight[turn], ownPos, oppPos, moveList, size);
        getKingMovesAppend(  position.king[turn],   ownPos, oppPos, oppAttackMask, moveList, size);
        getPawnMovesAppend(  position.pawn[turn],   ownPos, oppPos, position.en_passant, position.flags, moveList, size);
    }
    return *size;
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
    int to   = GET_TO(move);

    char from_piece = position->charBoard[from];
    char to_piece   = position->charBoard[to];

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

void unmakeMove(Position *position, Move move){
    char from_piece = position->charBoard[GET_FROM(move)];
    switch(from_piece){
    }
}






static uint64_t generatePinnedPiecesWhite(Position pos){
    int k_square = __builtin_ctzll(pos.king[1]);
    //Lets do white first!
    uint64_t pos_pinners;
    uint64_t all_pieces = pos.color[0] | pos.color[1];  //Contains all the initial pieces
    uint64_t pinned = pos.color[1];  //The peices belonging to white which become pinnged
    uint64_t d_attack_mask, h_attack_mask, attack_mask = 0ULL;  // A attack masks
    uint64_t pin_directions = 0ULL;

    uint64_t ep_pawn_square = 0ULL;
    if((pos.en_passant != 0) && (pos.flags & WHITE_TURN)){ //If white can capture en-passant
        ep_pawn_square = southOne(pos.en_passant);
    }
    
    attack_mask |= rookAttacks(all_pieces & ~ep_pawn_square, k_square);  //Get a bb of squares being attacked by the king
    attack_mask |= bishopAttacks(all_pieces, k_square);

    pinned &= attack_mask; // The pinned white pieces now contains the ones with a ray to the king

    //Now get all pieces under attack as if king is queen again and these are the possible pinners
    h_attack_mask = rookAttacks(all_pieces & ~pinned & ~ep_pawn_square, k_square);
    pos_pinners = h_attack_mask & (pos.queen[0] | pos.rook[0]);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    d_attack_mask = bishopAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = d_attack_mask & (pos.queen[0] | pos.bishop[0]);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    pinned &= pin_directions; //Only keep the pieces that are actually pinned
    return pinned;
}

static uint64_t generatePinnedPiecesBlack(Position pos){
    int k_square = __builtin_ctzll(pos.king[0]);
    //Lets do white first!
    uint64_t pos_pinners;
    uint64_t all_pieces = pos.color[0] | pos.color[1];  //Contains all the initial pieces
    uint64_t pinned = pos.color[0];  //The peices belonging to white which become pinnged
    uint64_t d_attack_mask, h_attack_mask, attack_mask = 0ULL;  // A attack masks
    uint64_t pin_directions = 0ULL;

    uint64_t ep_pawn_square = 0ULL;
    if((pos.en_passant != 0) && !(pos.flags & WHITE_TURN)){ //If white can capture en-passant
        ep_pawn_square = northOne(pos.en_passant);
    }
    
    attack_mask |= rookAttacks(all_pieces & ~ep_pawn_square, k_square);    //Get a bb of squares being attacked by the king
    attack_mask |= bishopAttacks(all_pieces, k_square);

    pinned &= attack_mask; // The pinned white pieces now contains the ones with a ray to the king

    //Now get all pieces under attack as if king is queen again and these are the possible pinners
    h_attack_mask = rookAttacks(all_pieces & ~pinned & ~ep_pawn_square, k_square);
    pos_pinners = h_attack_mask & (pos.queen[1] | pos.rook[1]);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    d_attack_mask = bishopAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = d_attack_mask & (pos.queen[1] | pos.bishop[1]);
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

