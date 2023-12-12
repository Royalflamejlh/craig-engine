#include "movement.h"
#include <ctype.h>
#include "./bitboard/magic.h"
#include "./bitboard/bitboard.h"
#include "./bitboard/bbutils.h"
#include "util.h"

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
        getCastleMovesAppend(ownPos | oppPos, oppAttackMask, position.flags, moveList, size);

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

static void movePiece(Position *pos, int turn, int from, int to){
    switch(toupper(pos->charBoard[from])){
        case 'Q':
            pos->queen[turn] = clearBit(pos->queen[turn], from);
            pos->queen[turn] = setBit(pos->queen[turn], to);
            break;
        case 'K':
            pos->king[turn] = clearBit(pos->king[turn], from);
            pos->king[turn] = setBit(pos->king[turn], to);
            break;
        case 'N':
            pos->knight[turn] = clearBit(pos->knight[turn], from);
            pos->knight[turn] = setBit(pos->knight[turn], to);
            break;
        case 'B':
            pos->bishop[turn] = clearBit(pos->bishop[turn], from);
            pos->bishop[turn] = setBit(pos->bishop[turn], to);
            break;
        case 'R':
            pos->rook[turn] = clearBit(pos->rook[turn], from);
            pos->rook[turn] = setBit(pos->rook[turn], to);
            break;
        case 'P':
            pos->pawn[turn] = clearBit(pos->pawn[turn], from);
            pos->pawn[turn] = setBit(pos->pawn[turn], to);
            pos->halfmove_clock = 0;
            break;
    }
    pos->charBoard[to] = pos->charBoard[from];
    pos->charBoard[from] = 0;

    pos->color[turn] = clearBit(pos->color[turn], from);
    pos->color[turn] = setBit(pos->color[turn], to);
}


/*
* Used to remove the captured piece
*/
static void removeCaptured(Position *pos, int square){
    int turn = pos->flags & WHITE_TURN;
    switch(toupper(pos->charBoard[square])){
        case 'Q':
            pos->queen[!turn] = clearBit(pos->queen[!turn], square);
            break;
        case 'K':
            //pos->king[!turn] = clearBit(pos->king[!turn], square);
            printf("WARNING ATTEMPTED TO CAPTURE A KING AT POS:\n");
            printPosition(*pos);
            break;
        case 'N':
            pos->knight[!turn] = clearBit(pos->knight[!turn], square);
            break;
        case 'B':
            pos->bishop[!turn] = clearBit(pos->bishop[!turn], square);
            break;
        case 'R':
            pos->rook[!turn] = clearBit(pos->rook[!turn], square);
            break;
        case 'P':
            pos->pawn[!turn] = clearBit(pos->pawn[!turn], square);
            break;
    }
    pos->color[!turn] = clearBit(pos->color[!turn], square);
    pos->charBoard[square] = 0;
    pos->halfmove_clock = 0;
}

int makeMove(Position *pos, Move move){
    int turn = pos->flags & WHITE_TURN;
    int from = GET_FROM(move);
    int to   = GET_TO(move);
    
    pos->halfmove_clock++;
    pos->en_passant = 0ULL;

    switch(GET_FLAGS(move)){
        case QUEEN_PROMO_CAPTURE:
            removeCaptured(pos, to);
            pos->pawn[turn] = clearBit(pos->pawn[turn], from); 
            pos->charBoard[from] = 0;
            pos->queen[turn] = setBit(pos->queen[turn], to); 
            pos->charBoard[to] = turn ? 'Q' : 'q';
            pos->color[turn] = clearBit(pos->color[turn], from);
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->halfmove_clock = 0;
            break;
        case ROOK_PROMO_CAPTURE:
            removeCaptured(pos, to);
            pos->pawn[turn] = clearBit(pos->pawn[turn], from); 
            pos->charBoard[from] = 0;
            pos->rook[turn] = setBit(pos->rook[turn], to); 
            pos->charBoard[to] = turn ? 'R' : 'r';
            pos->color[turn] = clearBit(pos->color[turn], from);
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->halfmove_clock = 0;
            break;
        case BISHOP_PROMO_CAPTURE:
            removeCaptured(pos, to);
            pos->pawn[turn] = clearBit(pos->pawn[turn], from); 
            pos->charBoard[from] = 0;
            pos->bishop[turn] = setBit(pos->bishop[turn], to); 
            pos->charBoard[to] = turn ? 'B' : 'b';
            pos->color[turn] = clearBit(pos->color[turn], from);
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->halfmove_clock = 0;
            break;
        case KNIGHT_PROMO_CAPTURE:
            removeCaptured(pos, to);
            pos->pawn[turn] = clearBit(pos->pawn[turn], from); 
            pos->charBoard[from] = 0;
            pos->knight[turn] = setBit(pos->knight[turn], to); 
            pos->charBoard[to] = turn ? 'N' : 'n';
            pos->color[turn] = clearBit(pos->color[turn], from);
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->halfmove_clock = 0;
            break;
            
        case EP_CAPTURE:
            removeCaptured(pos, (turn ? to - 8 : to + 8));
            movePiece(pos, turn, from, to);
            break;
        case CAPTURE:
            removeCaptured(pos, to);
            movePiece(pos, turn, from, to);
            break;

        case QUEEN_PROMOTION:
            pos->color[turn] = clearBit(pos->color[turn], from); //Update Color BB
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->queen[turn] = setBit(pos->queen[turn], to);  //Update Queen BB
            pos->pawn[turn] = clearBit(pos->pawn[turn], from);  //Update Pawn BB
            pos->charBoard[from] = 0;  //Update Charboard
            pos->charBoard[to] = turn ? 'Q' : 'q';
            pos->halfmove_clock = 0;
            break;
        case ROOK_PROMOTION:
            pos->color[turn] = clearBit(pos->color[turn], from);
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->rook[turn] = setBit(pos->rook[turn], to);
            pos->pawn[turn] = clearBit(pos->pawn[turn], from);
            pos->charBoard[from] = 0;
            pos->charBoard[to] = turn ? 'R' : 'r';
            pos->halfmove_clock = 0;
            break;
        case BISHOP_PROMOTION:
            pos->color[turn] = clearBit(pos->color[turn], from);
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->bishop[turn] = setBit(pos->bishop[turn], to);
            pos->pawn[turn] = clearBit(pos->pawn[turn], from);
            pos->charBoard[from] = 0;
            pos->charBoard[to] = turn ? 'B' : 'b';
            pos->halfmove_clock = 0;
            break;
        case KNIGHT_PROMOTION:
            pos->color[turn] = clearBit(pos->color[turn], from);
            pos->color[turn] = setBit(pos->color[turn], to);
            pos->knight[turn] = setBit(pos->knight[turn], to);
            pos->pawn[turn] = clearBit(pos->pawn[turn], from);
            pos->charBoard[from] = 0;
            pos->charBoard[to] = turn ? 'N' : 'n';
            pos->halfmove_clock = 0;
            break;
            
        case QUEEN_CASTLE:
            pos->flags &= ~(turn ? W_LONG_CASTLE : B_LONG_CASTLE);
            break;
        case KING_CASTLE:
            pos->flags &= ~(turn ? W_SHORT_CASTLE : B_SHORT_CASTLE);
            break;

        case DOUBLE_PAWN_PUSH:
            pos->en_passant = 1ULL << (turn ? to - 8 : to + 8);
        case QUIET:
        default:
            movePiece(pos, turn, from, to);
            break;
    }

    //Castle Aval Rooks Moves
    if(to == 0 || from == 0)   pos->flags &= ~W_LONG_CASTLE;
    if(to == 7 || from == 7)   pos->flags &= ~W_SHORT_CASTLE;
    if(to == 56 || from == 56) pos->flags &= ~B_LONG_CASTLE;
    if(to == 63 || from == 63) pos->flags &= ~B_SHORT_CASTLE;

    //King Moves
    if(from == 4) pos->flags &= ~(W_LONG_CASTLE | W_SHORT_CASTLE);
    if(from == 60) pos->flags &= ~(B_LONG_CASTLE | B_SHORT_CASTLE);



    if(!turn) pos->fullmove_number++;

    setAttackMasks(pos);

    //Check Flag
    pos->flags = pos->attack_mask[turn] & pos->king[!turn] ? (pos->flags | IN_CHECK) : (pos->flags & ~IN_CHECK);

    //Double Check Flag
    if(pos->flags & IN_CHECK){
       int king_sq = __builtin_ctzll(pos->king[!turn]);
       uint64_t attackers = getAttackers(*pos, king_sq, turn);
       attackers &= attackers - 1;
       if(attackers) pos->flags |= IN_D_CHECK;
    }
    else pos->flags &= ~IN_D_CHECK;

    pos->flags ^= WHITE_TURN;

    pos->pinned = generatePinnedPieces(*pos);

    return 0;
}

void unmakeMove(Position *position, Move move){
    char from_piece = position->charBoard[GET_FROM(move)];
    switch(from_piece){
    }
}


static uint64_t generatePinnedPiecesColor(Position pos, int turn){
    int k_square = __builtin_ctzll(pos.king[turn]);
    //Lets do white first!
    uint64_t pos_pinners;
    uint64_t all_pieces = pos.color[0] | pos.color[1];  //Contains all the initial pieces
    uint64_t pinned = pos.color[turn];  //The peices belonging to white which become pinnged
    uint64_t d_attack_mask, h_attack_mask, attack_mask = 0ULL;  // A attack masks
    uint64_t pin_directions = 0ULL;

    uint64_t ep_pawn_square = 0ULL;
    //Make sure ep_pawn_square is the same row as the k_square
    //if white turn => king must be row 5
    //if black turn => king must be row 4
    if((pos.en_passant != 0) && (turn) && (k_square / 8 == (turn ? 4 : 3))){ //If can capture en-passant
        ep_pawn_square = turn ? southOne(pos.en_passant) : northOne(pos.en_passant);
    }
    
    attack_mask |= rookAttacks(all_pieces & ~ep_pawn_square, k_square);  //Get a bb of squares being attacked by the king
    attack_mask |= bishopAttacks(all_pieces, k_square);

    pinned &= attack_mask; // The pinned white pieces now contains the ones with a ray to the king

    //Now get all pieces under attack as if king is queen again and these are the possible pinners
    h_attack_mask = rookAttacks(all_pieces & ~pinned & ~ep_pawn_square, k_square);
    pos_pinners = h_attack_mask & (pos.queen[!turn] | pos.rook[!turn]);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    d_attack_mask = bishopAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = d_attack_mask & (pos.queen[!turn] | pos.bishop[!turn]);
    while(pos_pinners){
        int pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    pinned &= pin_directions; //Only keep the pieces that are actually pinned
    return pinned;
}

uint64_t generatePinnedPieces(Position pos){
    return generatePinnedPiecesColor(pos, 0) | generatePinnedPiecesColor(pos, 1);
}

