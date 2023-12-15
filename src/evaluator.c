#include "evaluator.h"

int evaluate(Position pos){
    int eval_val = 0;
    uint64_t white_pieces = pos.color[1]; 
    uint64_t black_pieces = pos.color[0]; 
    while(white_pieces){
        eval_val++;
        white_pieces &= white_pieces - 1;
    }
    while(black_pieces){
        eval_val--;
        black_pieces &= black_pieces - 1;
    }
    return eval_val;
}