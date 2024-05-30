#include "evaluator.h"
#include <limits.h>
#include "types.h"
#include "util.h"
#include "tree.h"
#include "bitboard/bbutils.h"


//Piece values defined in header

#define ATTACK_BONUS        3  //Eval for each square under attack
#define D_ATTACK_BONUS      6  //Eval for each enemy under attack
#define DEFEND_BONUS        3  //Eval for each defended piece
#define HANGING_PEN        50  //Eval lost for hanging a piece

#define CHECK_PEN         100  //Eval lost under check

#define CASTLE_BONUS       10  //Eval gained in castled position

#define DOUBLE_PAWN_PEN   100  //Eval lost for double pawns
#define ISO_PAWN_PEN      100  //Eval lost for isolated pawns

#define PST_PAWN_MULT       4  //Multiplier Pawns in Eval
#define PST_KNIGHT_MULT     1  //Mult Knights in Eval
#define PST_BISHOP_MULT     1  //Mult Bishop in Eval
#define PST_QUEEN_MULT      1  //Mult Queen in Eval
#define PST_ROOK_MULT       1  //Mult Rook in Eval
#define PST_KING_MULT       1  //Mult King in Eval



static i32 PST[12][64];

static const i32 pieceValues[] = {
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

i32 quickEval(Position pos){
    i32 eval_val = 0;
    i32 turn = pos.flags & WHITE_TURN;
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
    return eval_val;
}

i32 evaluate(Position pos){
    i32 eval_val = 0;
    i32 turn = pos.flags & WHITE_TURN;

    //Get the stage
    i32 stage = pos.stage;
    (void) stage;
    
    #ifdef DEBUG
    printf("\nEval for stage: %d, Starting at:%d\n", stage, eval_val);
    #endif

    //Early Game Bonus for black
    // if(stage == EARLY_GAME){
    //     if(!turn){
    //         eval_val += EARLY_GAME_MOVES - pos.fullmove_number;
    //     }
    // }

    //Penalty for being in check
    if(pos.flags & IN_CHECK) eval_val -= CHECK_PEN;

    #ifdef DEBUG
    printf("After Check Penalty: %d\n", eval_val);
    #endif

    //Material
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

    #ifdef DEBUG
    printf("After Material: %d\n", eval_val);
    #endif

    //Overall attack map bonus
    eval_val += DEFEND_BONUS * count_bits(pos.color[turn] &  pos.attack_mask[turn]);
    eval_val += ATTACK_BONUS * count_bits(pos.attack_mask[turn]);
    eval_val += D_ATTACK_BONUS * count_bits(pos.color[!turn] &  pos.attack_mask[turn]);
    eval_val -= HANGING_PEN * count_bits((pos.color[turn] & ~pos.attack_mask[turn]) & pos.attack_mask[!turn]);

    eval_val -= DEFEND_BONUS * count_bits(pos.color[!turn] &  pos.attack_mask[!turn]);
    eval_val -= ATTACK_BONUS * count_bits(pos.attack_mask[!turn]);
    eval_val -= D_ATTACK_BONUS * count_bits(pos.color[turn] &  pos.attack_mask[!turn]);
    eval_val += HANGING_PEN * count_bits((pos.color[!turn] & ~pos.attack_mask[!turn]) & pos.attack_mask[turn]);

    #ifdef DEBUG
    printf("After Attack Maps: %d\n", eval_val);
    #endif

    //Castle Bonus
    //if()

    //Pawn Stuff
    for(i32 i = 0; i < 8; i++){ //Double Pawn Check
        i32 double_pawns = count_bits(pos.pawn[turn] & fileMask[i]) - 1;
        if(double_pawns > 1){
            eval_val -= DOUBLE_PAWN_PEN * double_pawns;
        }
        double_pawns = count_bits(pos.pawn[!turn] & fileMask[i]) - 1;
        if(double_pawns > 1){
            eval_val += DOUBLE_PAWN_PEN * double_pawns;
        }
    }

    #ifdef DEBUG
    printf("After Doubled Pawn Penalty: %d\n", eval_val);
    #endif


    //PST
    //Pawns
    i32 index = turn ? WHITE_PAWN : BLACK_PAWN;
    i32 index_op = turn ? BLACK_PAWN : WHITE_PAWN;
    u64 pieces = pos.pawn[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST_PAWN_MULT * PST[index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.pawn[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST_PAWN_MULT * PST[index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    printf("After Pawn: %d\n", eval_val);
    #endif

    //Knights
    index = turn ? WHITE_KNIGHT : BLACK_KNIGHT;
    index_op = turn ? BLACK_KNIGHT : WHITE_KNIGHT;
    pieces = pos.knight[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST_KNIGHT_MULT * PST[index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.knight[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST_KNIGHT_MULT * PST[index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    printf("After Knight: %d\n", eval_val);
    #endif

    //Bishops
    index = turn ? WHITE_BISHOP : BLACK_BISHOP;
    index_op = turn ? BLACK_BISHOP : WHITE_BISHOP;
    pieces = pos.bishop[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST_BISHOP_MULT * PST[index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.bishop[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST_BISHOP_MULT * PST[index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    printf("After Bishop: %d\n", eval_val);
    #endif

    //Rooks
    index = turn ? WHITE_ROOK : BLACK_ROOK;
    index_op = turn ? BLACK_ROOK : WHITE_ROOK;
    pieces = pos.rook[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST_ROOK_MULT * PST[index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.rook[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST_ROOK_MULT * PST[index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    printf("After Rook: %d\n", eval_val);
    #endif

    //Queens
    index = turn ? WHITE_QUEEN : BLACK_QUEEN;
    index_op = turn ? BLACK_QUEEN : WHITE_QUEEN;
    pieces = pos.queen[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST_QUEEN_MULT * PST[index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.queen[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST_QUEEN_MULT * PST[index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    printf("After Queen: %d\n", eval_val);
    #endif

    //King
    index = turn ? WHITE_KING : BLACK_KING; // 5 : 11
    index_op = turn ? BLACK_KING : WHITE_KING;
    pieces = pos.king[turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val += PST_KING_MULT * PST[index][square];
        pieces &= pieces - 1;
    }
    pieces = pos.king[!turn];
    while (pieces) {
        i32 square = __builtin_ctzll(pieces);
        eval_val -= PST_KING_MULT * PST[index_op][square];
        pieces &= pieces - 1;
    }
    #ifdef DEBUG
    printf("After King: %d\n", eval_val);
    #endif

    
    
    



    return eval_val;
}


/*
*  Below here is the move evaluating code
*/

void initPST(){

    i32 PST_PAWN[64] = {
        0,  0,  0,  0,  0,  0,  0,  0,
        1,  1,  1,  0,  0,  1,  1,  1,
        0,  1,  1, -1, -1,  1,  1,  0,
        0,  0,  0,  1,  1,  0,  0,  0,
        0,  0,  0,  2,  2,  0,  0,  0,
        1,  1,  2,  3,  3,  2,  1,  1,
        5,  5,  5,  5,  5,  5,  5,  5,
        0,  0,  0,  0,  0,  0,  0,  0
    };

    i32 PST_KNIGHT[64] = {
        -5, -4, -3, -3, -3, -3, -4, -5,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  2,  2,  1,  0, -3,
        -3,  0,  1,  1,  1,  1,  0, -3,
        -4, -2,  0,  0,  0,  0, -2, -4,
        -5, -4, -3, -3, -3, -3, -4, -5
    };

    i32 PST_BISHOP[64] = {
        -2, -1, -1, -1, -1, -1, -1, -2,
        -1,  1,  0,  0,  0,  0,  1, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  1,  1,  1,  1,  1,  1, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -2, -1, -1, -1, -1, -1, -1, -2
    };

    i32 PST_ROOK[64] = {
        0,  0,  3,  4,  4,  3,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,
        5,  5,  5,  5,  5,  5,  5,  5,
       10, 10, 10, 10, 10, 10, 10, 10
    };

    i32 PST_QUEEN[64] = {
        -2, -1, -1, -1, -1, -1, -1, -2,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  1,  1,  1,  1,  0, -1,
        -1,  0,  0,  0,  0,  0,  0, -1,
        -2, -1, -1, -1, -1, -1, -1, -2
    };

    i32 PST_KING[64] = {
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
    for (i32 i = 0; i < 64; i++) {
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

#define HIST_SCORE_SHIFT 16
#define HIST_MAX_SCORE   PAWN_VALUE - 100

#define TT_SCORE     MAX_EVAL - QUEEN_VALUE
#define K_MOVE_SCORE MAX_EVAL - 2*QUEEN_VALUE


void evalMoves(Move* moveList, i32* moveVals, i32 size, Position pos){
    for(i32 i = 0; i < size; i++){
        moveVals[i] = 0;
        
        Move move = moveList[i];

        i32 fr_piece = (i32)pos.charBoard[GET_FROM(move)];
        i32 to_piece = (i32)pos.charBoard[GET_TO(move)];
        
        i32 fr_piece_i = pieceToIndex[fr_piece];
        i32 to_piece_i = pieceToIndex[to_piece];

        #ifdef DEBUG
        if(from_piece_i >= 12 || to_piece_i >= 12){
            printf("Warning illegal piece found at:");
            printPosition(pos, TRUE);
            printf("from piece: %d", pos.charBoard[GET_FROM(move)]);
            printf(" to piece: %d", pos.charBoard[GET_TO(move)]);
        }
        #endif

        //Add on the PST values
        moveVals[i] += PST[fr_piece_i][GET_TO(move)];
        
        //Add on the flag values
        //i32 histScore;
        switch(GET_FLAGS(move)){
            case QUEEN_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece_i] + QUEEN_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case ROOK_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece_i] + ROOK_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case BISHOP_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece_i] + BISHOP_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case KNIGHT_PROMO_CAPTURE:
                moveVals[i] += ((pieceValues[to_piece_i] + KNIGHT_VALUE - PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
                
            case EP_CAPTURE:
                moveVals[i] += ((PAWN_VALUE)+QUEEN_VALUE) - PAWN_VALUE;
                break;
            case CAPTURE:
                moveVals[i] += ((pieceValues[to_piece_i])+QUEEN_VALUE) - pieceValues[fr_piece_i];
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
                //histScore = getHistoryScore(pos.flags, moveList[i]) >> HIST_SCORE_SHIFT;
                //moveVals[i] += MIN(histScore, HIST_MAX_SCORE);
            default:
                break;
        }
    }

    return;
}


