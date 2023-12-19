#include "evaluator.h"
#include "util.h"

#define KING_VALUE     100000
#define QUEEN_VALUE     10000
#define ROOK_VALUE       5000
#define BISHOP_VALUE     3550
#define KNIGHT_VALUE     3500
#define PAWN_VALUE       1000

#define ATTACK_BONUS       10  //Eval for each square under attack
#define D_ATTACK_BONUS     50  //Eval for each enemy under attack
#define DEFEND_BONUS      100  //Eval for each defended piece
#define CHECK_PEN         100  //Eval lost under check

#define CASTLE_BONUS        10 //Eval gained in castled position

#define DOUBLE_PAWN_PEN   200  //Eval lost for double pawns
#define ISO_PAWN_PEN      100  //Eval lost for isolated pawns

enum {
    OPENING,
    MID_GAME,
    END_GAME
};

int evaluate(Position pos){
    int eval_val = 0;
    int turn = pos.flags & WHITE_TURN;

    if(pos.flags & IN_CHECK) eval_val -= CHECK_PEN;

    //Pieces
    eval_val += KING_VALUE * count_bits(pos.king[turn]);
    eval_val += QUEEN_VALUE * count_bits(pos.queen[turn]);
    eval_val += ROOK_VALUE * count_bits(pos.rook[turn]);
    eval_val += BISHOP_VALUE * count_bits(pos.bishop[turn]);
    eval_val += KNIGHT_VALUE * count_bits(pos.knight[turn]);
    eval_val += PAWN_VALUE * count_bits(pos.pawn[turn]);

    eval_val -= KING_VALUE * count_bits(pos.king[!turn]);
    eval_val -= QUEEN_VALUE * count_bits(pos.queen[!turn]);
    eval_val -= ROOK_VALUE * count_bits(pos.rook[!turn]);
    eval_val -= BISHOP_VALUE * count_bits(pos.bishop[!turn]);
    eval_val -= KNIGHT_VALUE * count_bits(pos.knight[!turn]);
    eval_val -= PAWN_VALUE * count_bits(pos.pawn[!turn]);

    eval_val += DEFEND_BONUS * count_bits(pos.color[turn] &  pos.attack_mask[turn]);
    eval_val += ATTACK_BONUS * count_bits(pos.attack_mask[turn]);
    eval_val += D_ATTACK_BONUS * count_bits(pos.color[!turn] &  pos.attack_mask[turn]);

    eval_val -= DEFEND_BONUS * count_bits(pos.color[!turn] &  pos.attack_mask[!turn]);
    eval_val -= ATTACK_BONUS * count_bits(pos.attack_mask[!turn]);
    eval_val -= D_ATTACK_BONUS * count_bits(pos.color[turn] &  pos.attack_mask[!turn]);

    return eval_val;
}


/*
*  Below here is the move evaluating code
*/
int PST[12][64];

void initPST(){
    //put a piece on each square and get attack
    for(int i = 0; i < 64; i++){
        return;
    }
}

//Move iterating logic
//First find TTMove, test, and remove from movelist
//Then find killerMoves, test, and remove from movelist
//Then generate values for the remaining moves
//Selection sort remaining moves



void evalMoves(Move* moveList, int* moveVals, int size, Position pos){

    int i = 0;
    for(int i = 0; i < size; i++){
        Move move = moveList[i];
        int to_value = 0;
        int from_value = 0;
        char from_piece = pos.charBoard[GET_FROM(move)];

        //Add on the PST values
        moveVals[i] += PST[pieceToIndex[(int)from_piece]][GET_TO(move)];
        
        //Add on the flag values
        switch(GET_FLAGS(move)){
            case QUEEN_PROMO_CAPTURE:
                moveVals[i] += ((to_value + QUEEN_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case ROOK_PROMO_CAPTURE:
                moveVals[i] += ((to_value + ROOK_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case BISHOP_PROMO_CAPTURE:
                moveVals[i] += ((to_value + BISHOP_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case KNIGHT_PROMO_CAPTURE:
                moveVals[i] += ((to_value + KNIGHT_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
                
            case EP_CAPTURE:
                moveVals[i] += ((PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case CAPTURE:
                moveVals[i] += ((to_value)+QUEEN_VALUE) - from_value;
                break;

            case QUEEN_PROMOTION:
                moveVals[i] += (QUEEN_VALUE - PAWN_VALUE);    
                break;
            case ROOK_PROMOTION:
                moveVals[i] += (ROOK_VALUE - PAWN_VALUE);    
                break;
            case BISHOP_PROMOTION:
                moveVals[i] += (BISHOP_VALUE - PAWN_VALUE);    
                break;
            case KNIGHT_PROMOTION:
                moveVals[i] += (KNIGHT_VALUE - PAWN_VALUE);    
                break;
                
            case QUEEN_CASTLE:
            case KING_CASTLE:
                moveVals[i] += CASTLE_BONUS;
                break;

            case DOUBLE_PAWN_PUSH:
            case QUIET:
            default:
                break;
        }
        //mmv - lva
    }

    return;
}


