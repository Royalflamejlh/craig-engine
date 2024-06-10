#include "movement.h"
#include <ctype.h>
#include "./bitboard/magic.h"
#include "./bitboard/bitboard.h"
#include "./bitboard/bbutils.h"
#include "evaluator.h"
#include "util.h"
#include "hash.h"


#ifdef DEBUG
#include "transposition.h"
#endif

/*
* Wow Cool! Totaly sensical move generation!
*/
u16 generateLegalMoves(Position position,  Move* moveList){
    i32 size[] = {0};
    i32 turn = position.flags & WHITE_TURN; //True for white false for black
    u64 ownPos = position.color[turn];
    u64 oppPos = position.color[!turn];
    u64 oppAttackMask = position.attack_mask[!turn];

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

/*
* Generate Moves that capture pieces, and put the opponents king in check
*/
u16 generateThreatMoves(Position position,  Move* moveList){
    i32 size[] = {0};
    i32 turn = position.flags & WHITE_TURN; //True for white false for black
    u64 ownPos = position.color[turn];
    u64 oppPos = position.color[!turn];
    u64 oppAttackMask = position.attack_mask[!turn];

    i32 kingSq = __builtin_ctzll(position.king[!turn]);
    u64 r_check_squares = rookAttacks(  ownPos | oppPos, kingSq) & ~(ownPos | oppPos);
    u64 b_check_squares = bishopAttacks(ownPos | oppPos, kingSq) & ~(ownPos | oppPos);

    if(position.flags & IN_CHECK){
        if(position.flags & IN_D_CHECK){
            getKingMovesAppend(position.king[turn], ownPos, oppPos, oppAttackMask, moveList, size);
        }
        else{
            getCheckMovesAppend(position, moveList, size);
        }
    }
    else if(position.pinned & ownPos){
        getPinnedThreatMovesAppend(position, r_check_squares, b_check_squares, kingSq, moveList, size);
    }
    else{
        getBishopThreatMovesAppend(position.queen[turn],  ownPos, oppPos, b_check_squares, moveList, size);
        getRookThreatMovesAppend(  position.queen[turn],  ownPos, oppPos, r_check_squares, moveList, size);
        getRookThreatMovesAppend(  position.rook[turn],   ownPos, oppPos, r_check_squares, moveList, size);
        getBishopThreatMovesAppend(position.bishop[turn], ownPos, oppPos, b_check_squares, moveList, size);
        getKnightThreatMovesAppend(position.knight[turn], ownPos, oppPos, kingSq, moveList, size);
        getKingThreatMovesAppend(  position.king[turn],   ownPos, oppPos, oppAttackMask, moveList, size);
        getPawnThreatMovesAppend(  position.pawn[turn],   ownPos, oppPos, position.en_passant, position.flags, kingSq, moveList, size);
    }
    return *size;
}


static void movePiece(Position *pos, i32 turn, i32 from, i32 to){
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
static void removeCaptured(Position *pos, i32 square){
    i32 turn = pos->flags & WHITE_TURN;
    switch(toupper(pos->charBoard[square])){
        case 'Q':
            pos->queen[!turn] = clearBit(pos->queen[!turn], square);
            break;
        case 'K':
            //pos->king[!turn] = clearBit(pos->king[!turn], square);
            #ifdef DEBUG
            printf("WARNING ATTEMPTED TO CAPTURE A KING AT POS:\n");
            printPosition(*pos, TRUE);
            while(TRUE){};
            #else
            printf("info string Found illegal position during search - King Capture.\n");
            #endif
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

i32 makeMove(Position *pos, Move move){
    #ifdef DEBUG
    if(move == NO_MOVE) printf("WARNING ILLEGAL NO-MOVE IN MAKE MOVE\n");
    #endif
    i32 turn = pos->flags & WHITE_TURN;
    i32 from = GET_FROM(move);
    i32 to   = GET_TO(move);
    
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
            movePiece(pos, turn, from, to);
            movePiece(pos, turn, turn ? 0 : 56, turn ? 3 : 59);
            break;
        case KING_CASTLE:
            pos->flags &= ~(turn ? W_SHORT_CASTLE : B_SHORT_CASTLE);
            movePiece(pos, turn, from, to);
            movePiece(pos, turn, turn ? 7 : 63, turn ? 5 : 61);
            break;

        case DOUBLE_PAWN_PUSH:
            pos->en_passant = 1ULL << (turn ? to - 8 : to + 8);
            // fall through
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
    if(from == 4)  pos->flags &= ~(W_LONG_CASTLE | W_SHORT_CASTLE);
    if(from == 60) pos->flags &= ~(B_LONG_CASTLE | B_SHORT_CASTLE);

    if(!turn) pos->fullmove_number++;

    setAttackMasks(pos);

    //Check Flag
    pos->flags = pos->attack_mask[turn] & pos->king[!turn] ? (pos->flags | IN_CHECK) : (pos->flags & ~IN_CHECK);

    //Double Check Flag
    if(pos->flags & IN_CHECK){
       i32 king_sq = __builtin_ctzll(pos->king[!turn]);
       u64 attackers = getAttackers(*pos, king_sq, turn);
       attackers &= attackers - 1;
       if(attackers) pos->flags |= IN_D_CHECK;
    }
    else pos->flags &= ~IN_D_CHECK;

    pos->flags ^= WHITE_TURN;

    pos->pinned = generatePinnedPieces(*pos);

    pos->stage = calculateStage(*pos);

    pos->quick_eval = quickEval(*pos);

    pos->hash = hashPosition(*pos);

    HashStack *hs = &pos->hashStack;
    if(hs->current_idx + 1 >= hs->size) doubleHashStack(hs);
    hs->current_idx++;
    if(pos->halfmove_clock == 0) hs->last_reset_idx = hs->current_idx;
    hs->ptr[hs->current_idx] = pos->hash;    


    #ifdef DEBUG
    
    if(count_bits(pos->king[0]) != 1 || count_bits(pos->king[1]) != 1){
        printf("ILLEGAL POSITION WITH TWO KINGS\n");
        printPosition(*pos, TRUE);
        printf("From move: ");
        printMove(move);
        printf(".\r\n");
        u64 prevHash = pos->hashStack.ptr[pos->hashStack.current_idx-1];
        u64 prevprevHash = pos->hashStack.ptr[pos->hashStack.current_idx-2];

        printf("Prev Move: ");
        TTEntry *prev = getTTEntry(prevHash);
        if(prev){
            printMove(prev->move);
        }
        else{
            printf("n/a");
        }
        printf(" Prev-prev move: ");
        prev = getTTEntry(prevprevHash);
        if(prev){
            printMove(prev->move);
        }
        else{
            printf("n/a");
        }
        printf("\r\n");
        
        fflush(stdout);
        while(TRUE){};
    }

    #endif

    return 0;
}

i32 makeNullMove(Position *pos){

    i32 turn = pos->flags & WHITE_TURN;

    pos->halfmove_clock++;
    if(!turn) pos->fullmove_number++;

    i32 regen_pinned = pos->en_passant != 0; // If there was an en passant square we have to regen pinned pieces
    pos->en_passant = 0ULL;

    pos->flags ^= WHITE_TURN;

    if(regen_pinned) pos->pinned = generatePinnedPieces(*pos);

    pos->hash = hashPosition(*pos);

    return 0;
}

void unmakeMove(Position *position, Move move){
    char from_piece = position->charBoard[GET_FROM(move)];
    switch(from_piece){
    }
}


static u64 generatePinnedPiecesColor(Position pos, i32 turn){
    i32 k_square = __builtin_ctzll(pos.king[turn]);
    //Lets do white first!
    u64 pos_pinners;
    u64 all_pieces = pos.color[0] | pos.color[1];  //Contains all the initial pieces
    u64 pinned = pos.color[turn];  //The peices belonging to white which become pinnged
    u64 d_attack_mask, h_attack_mask, attack_mask = 0ULL;  // A attack masks
    u64 pin_directions = 0ULL;

    u64 ep_pawn_square = 0ULL;
    //Make sure ep_pawn_square is the same row as the k_square
    //if white turn => king must be row 5
    //if black turn => king must be row 4
    if((pos.en_passant != 0) && (k_square / 8 == (turn ? 4 : 3))){ //If can capture en-passant
        ep_pawn_square = turn ? southOne(pos.en_passant) : northOne(pos.en_passant);
    }
    
    attack_mask |= rookAttacks(all_pieces & ~ep_pawn_square, k_square);  //Get a bb of squares being attacked by the king
    attack_mask |= bishopAttacks(all_pieces, k_square);

    pinned &= attack_mask; // The pinned white pieces now contains the ones with a ray to the king

    //Now get all pieces under attack as if king is queen again and these are the possible pinners
    h_attack_mask = rookAttacks(all_pieces & ~pinned & ~ep_pawn_square, k_square);
    pos_pinners = h_attack_mask & (pos.queen[!turn] | pos.rook[!turn]);
    while(pos_pinners){
        i32 pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    d_attack_mask = bishopAttacks(all_pieces & ~pinned, k_square);
    pos_pinners = d_attack_mask & (pos.queen[!turn] | pos.bishop[!turn]);
    while(pos_pinners){
        i32 pinner_sq = __builtin_ctzll(pos_pinners);
        pin_directions |= betweenMask[k_square][pinner_sq];
        pos_pinners &= pos_pinners - 1;
    }

    pinned &= pin_directions; //Only keep the pieces that are actually pinned
    return pinned;
}

u64 generatePinnedPieces(Position pos){
    return generatePinnedPiecesColor(pos, 0) | generatePinnedPiecesColor(pos, 1);
}

