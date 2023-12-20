#include "evaluator.h"
#include <limits.h>
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

static const int pieceValues[] = {
    [WHITE_PAWN] = PAWN_VALUE,
    [WHITE_KNIGHT] = KNIGHT_VALUE,
    [WHITE_BISHOP] = BISHOP_VALUE,
    [WHITE_ROOK] = ROOK_VALUE,
    [WHITE_QUEEN] = QUEEN_VALUE,
    [WHITE_KING] = KING_VALUE,
    [BLACK_PAWN] = PAWN_VALUE,
    [BLACK_KNIGHT] = KNIGHT_VALUE,
    [BLACK_BISHOP] = BISHOP_VALUE,
    [BLACK_ROOK] = ROOK_VALUE,
    [BLACK_QUEEN] = QUEEN_VALUE,
    [BLACK_KING] = KING_VALUE
};

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
static int PST[12][64];

void initPST(){

    int PST_PAWN[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        1,  1,  1,  0,  0,  1,  1,  1,
        0,  1,  1, -1, -1,  1,  1,  0,
        0,  0,  0,  1,  1,  0,  0,  0,
        0,  0,  0,  2,  2,  0,  0,  0,
        1,  1,  2,  3,  3,  2,  1,  1,
        5,  5,  5,  5,  5,  5,  5,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    int PST_KNIGHT[64] = {
        -5, -4, -3, -3, -3, -3, -4, -5,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -5, -4, -3, -3, -3, -3, -4, -5
    };

    int PST_BISHOP[64] = {
        -2, -1, -1, -1, -1, -1, -1, -2,
        -1,  1,  0,  0,  0,  0,  1, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  1,  1,  1,  1,  1,  1, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -2, -1, -1, -1, -1, -1, -1, -2
    };

    int PST_ROOK[64] = {
        0,  0,  3,  4,  4,  3,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        5,  5,  5,  5,  5,  5,  5,  5,
       10, 10, 10, 10, 10, 10, 10, 10
    };

    int PST_QUEEN[64] = {
        -2, -1, -1, -1, -1, -1, -1, -2,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -2, -1, -1, -1, -1, -1, -1, -2
    };

    int PST_KING[64] = {
        3,  4,  4, -2, -2,  4,  4,  3,
        2,  2,  0,  0,  0,  0,  2,  2,
        -1, -2, -2, -2, -2, -2, -2, -1,
        -2, -3, -3, -4, -4, -3, -3, -2,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3,
        -3, -4, -4, -5, -5, -4, -4, -3
    };


    // Initialize PST for white pieces
    for (int i = 0; i < 64; i++) {
        PST[WHITE_PAWN][i] = PST_PAWN[i];
        PST[WHITE_KNIGHT][i] = PST_KNIGHT[i];
        PST[WHITE_BISHOP][i] = PST_BISHOP[i];
        PST[WHITE_ROOK][i] = PST_ROOK[i];
        PST[WHITE_QUEEN][i] = PST_QUEEN[i];
        PST[WHITE_KING][i] = PST_KING[i];

        PST[BLACK_PAWN][63 - i] = PST_PAWN[i];
        PST[BLACK_KNIGHT][63 - i] = PST_KNIGHT[i];
        PST[BLACK_BISHOP][63 - i] = PST_BISHOP[i];
        PST[BLACK_ROOK][63 - i] = PST_ROOK[i];
        PST[BLACK_QUEEN][63 - i] = PST_QUEEN[i];
        PST[BLACK_KING][63 - i] = PST_KING[i];
    }

}

//Move iterating logic
//First find TTMove, test, and remove from movelist
//Then find killerMoves, test, and remove from movelist
//Then generate values for the remaining moves
//Selection sort remaining moves



void evalMoves(Move* moveList, int* moveVals, int size, Move ttMove, Move *killerMoves, int kmv_size, Position pos){
    for(int i = 0; i < size; i++){
        Move move = moveList[i];
        //Score the TT move
        if(move == ttMove){
            moveVals[i] = INT_MAX;
            continue;
        }

        int found = 0;
        for(int j = 0; j < kmv_size; j++){
            if(move == killerMoves[j]){
                moveVals[i] = INT_MAX - 1;
                break;
            }
        }
        if(found) continue;

        int from_piece = (int)pos.charBoard[GET_FROM(move)];
        int to_piece = (int)pos.charBoard[GET_TO(move)];

        //Add on the PST values
        moveVals[i] += PST[pieceToIndex[(int)from_piece]][GET_TO(move)];
        
        //Add on the flag values
        switch(GET_FLAGS(move)){
            case QUEEN_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece] + QUEEN_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case ROOK_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece] + ROOK_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case BISHOP_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece] + BISHOP_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case KNIGHT_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece] + KNIGHT_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
                
            case EP_CAPTURE:
                moveVals[i] += ((PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case CAPTURE:
                moveVals[i] += ((pieceValues[to_piece])+QUEEN_VALUE) - pieceValues[from_piece];
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
    }

    return;
}


