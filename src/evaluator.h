#ifndef evaluate_h
#define evaluate_h

#include "types.h"
#include "util.h"

#define KING_VALUE     100000
#define QUEEN_VALUE     10000
#define ROOK_VALUE       5000
#define BISHOP_VALUE     3550
#define KNIGHT_VALUE     3500
#define PAWN_VALUE       1000

#ifdef DEBUG
i32 evaluate(Position pos, u8 verbose);
#else
i32 evaluate(Position pos);
#endif // DEBUG

static inline i32 quickEval(Position pos){
    i32 eval_val = 0;
    i32 turn = pos.flags & WHITE_TURN;
    eval_val += KING_VALUE   * (count_bits(pos.king[turn])   - count_bits(pos.king[!turn]));
    eval_val += QUEEN_VALUE  * (count_bits(pos.queen[turn])  - count_bits(pos.queen[!turn]));
    eval_val += ROOK_VALUE   * (count_bits(pos.rook[turn])   - count_bits(pos.rook[!turn]));
    eval_val += BISHOP_VALUE * (count_bits(pos.bishop[turn]) - count_bits(pos.bishop[!turn]));
    eval_val += KNIGHT_VALUE * (count_bits(pos.knight[turn]) - count_bits(pos.knight[!turn]));
    eval_val += PAWN_VALUE   * (count_bits(pos.pawn[turn])   - count_bits(pos.pawn[!turn]));
    return eval_val;
};

void initPST(void);
i32 evalMove(Move move, Position* pos);
void q_evalMoves(Move* moveList, i32* moveVals, i32 size, Position pos);
i32 see ( Position pos, u32 toSq, PieceIndex target, u32 frSq, PieceIndex aPiece);

#endif //evaluate_h
