#include "evaluator.h"
#include "util.h"

#define KING_VALUE     100000
#define QUEEN_VALUE     10000
#define ROOK_VALUE       5000
#define BISHOP_VALUE     3550
#define KNIGHT_VALUE     3500
#define PAWN_VALUE       1000

#define ATTACK_BONUS       10  //Eval for each square under attack
#define D_ATTACK_BONUS    100  //Eval for each enemy under attack
#define DEFEND_BONUS      200  //Eval for each defended piece
#define CHECK_PEN         100  //Eval lost under check
#define TURN_BONUS          5


#define DOUBLE_PAWN_PEN   200  //Eval lost for double pawns
#define ISO_PAWN_PEN      100  //Eval lost for isolated pawns



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

