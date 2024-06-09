#include "util.h"
#include "movement.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define HASHSTACK_INIT_SIZE 256

#ifdef PYTHON
static i32 fifo_fd_in, fifo_fd_out;
static pid_t pid;
#endif

void printMove(Move move){
    i32 from = GET_FROM(move);
    i32 to = GET_TO(move);

    i32 rank_from = from / 8;
    i32 file_from = from % 8;

    i32 rank_to = to / 8;
    i32 file_to = to % 8;

    char file_char_from = 'a' + file_from;
    char file_char_to = 'a' + file_to;

    i32 rank_num_from = rank_from + 1;
    i32 rank_num_to = rank_to + 1;

    printf("%c%d%c%d", file_char_from, rank_num_from, file_char_to, rank_num_to);

    switch(GET_FLAGS(move)){
        case DOUBLE_PAWN_PUSH:
            printf(" (DPP)");
            break;
        case KING_CASTLE:
            printf(" (KCl)");
            break;
        case QUEEN_CASTLE:
            printf(" (QCl)");
            break;
        case CAPTURE:
            printf(" (C)");
            break;
        case EP_CAPTURE:
            printf(" (EpC)");
            break;
        case KNIGHT_PROMOTION:
            printf(" (NP)");
            break;
        case BISHOP_PROMOTION:
            printf(" (BP)");
            break;
        case ROOK_PROMOTION:
            printf(" (RP)");
            break;
        case QUEEN_PROMOTION:
            printf(" (QP)");
            break;
        case KNIGHT_PROMO_CAPTURE:
            printf(" (NPC)");
            break;
        case BISHOP_PROMO_CAPTURE:
            printf(" (BPC)");
            break;
        case ROOK_PROMO_CAPTURE:
            printf(" (RPC)");
            break;
        case QUEEN_PROMO_CAPTURE:
            printf(" (QPC)");
            break;
        case QUIET:
        default:
            break;
    }
}

/*
 * Prints the provided move as the best move in the UCI format
 */
void printBestMove(Move move){
    char str[6];
    str[0] = (GET_FROM(move) % 8) + 'a';
    str[1] = (GET_FROM(move) / 8) + '1';
    str[2] = (GET_TO(move) % 8) + 'a';
    str[3] = (GET_TO(move) / 8) + '1';
    str[4] = '\0';

    switch(GET_FLAGS(move)){
        case QUEEN_PROMO_CAPTURE:
        case QUEEN_PROMOTION:
            str[4] = 'q';
            str[5] = '\0';
            break;
        case ROOK_PROMO_CAPTURE:
        case ROOK_PROMOTION:
            str[4] = 'r';
            str[5] = '\0';
            break;
        case BISHOP_PROMO_CAPTURE:
        case BISHOP_PROMOTION:
            str[4] = 'b';
            str[5] = '\0';
            break;
        case KNIGHT_PROMO_CAPTURE:
        case KNIGHT_PROMOTION:
            str[4] = 'n';
            str[5] = '\0';
            break;
        default:
            break;
    }

    #if defined(_WIN32) || defined(_WIN64)
    printf("bestmove %s\r\n", str);
    #else
    printf("bestmove %s\n", str);
    #endif

    fflush(stdout);
    return;
}

void printMoveShort(Move move){
    i32 from = GET_FROM(move);
    i32 to = GET_TO(move);

    i32 rank_from = from / 8;
    i32 file_from = from % 8;

    i32 rank_to = to / 8;
    i32 file_to = to % 8;

    char file_char_from = 'a' + file_from;
    char file_char_to = 'a' + file_to;

    i32 rank_num_from = rank_from + 1;
    i32 rank_num_to = rank_to + 1;

    printf("%c%d%c%d", file_char_from, rank_num_from, file_char_to, rank_num_to);
}

void printMoveSpaced(Move move){
    printMove(move);
    switch(GET_FLAGS(move)){
        case CAPTURE:
            printf("  ");
            break;
        case KNIGHT_PROMOTION:
        case BISHOP_PROMOTION:
        case ROOK_PROMOTION:
        case QUEEN_PROMOTION:
            printf(" ");
            break;
        case QUIET:
            printf("      ");
        default:
            break;
    }
}

u64 perft(i32 depth, Position pos){
  Move move_list[256];
  i32 n_moves, i;
  u64 nodes = 0;

  if (depth == 0) 
    return 1ULL;

  n_moves = generateLegalMoves(pos, move_list);

  for (i = 0; i < n_moves; i++) {
    Position prevPos = pos;
    makeMove(&pos, move_list[i]);
    
    #ifdef PYTHON
    checkMoveCount(pos);
    #endif
    nodes += perft(depth - 1, pos);
    pos = prevPos;
  }
  
  return nodes;
}

char getPiece(Position pos, i32 square){
    return pos.charBoard[square];
}

Move moveStrToType(Position pos, char* str){
    Move moveList[MAX_MOVES] = {0};
    i32 size = generateLegalMoves(pos, moveList);

    i32 from  = (str[0] - 'a') + ((str[1] - '1') * 8);
    i32 to = (str[2] - 'a') + ((str[3] - '1') * 8);
    char promo = str[4];

    for(i32 i = 0; i < size; i++){
        i32 fromCur = GET_FROM(moveList[i]);
        i32 toCur = GET_TO(moveList[i]);
        char promoCur;
        switch(GET_FLAGS(moveList[i])){
            case QUEEN_PROMO_CAPTURE:
            case QUEEN_PROMOTION:
                promoCur = 'q';
                break;
            case ROOK_PROMO_CAPTURE:
            case ROOK_PROMOTION:
                promoCur = 'r';
                break;
            case BISHOP_PROMO_CAPTURE:
            case BISHOP_PROMOTION:
                promoCur = 'b';
                break;
            case KNIGHT_PROMO_CAPTURE:
            case KNIGHT_PROMOTION:
                promoCur = 'n';
                break;
            default:
                promoCur = '\0';
                break;
        }
        if(fromCur == from && toCur == to && promo == promoCur){
            return moveList[i];
        }
    }
    printf("info warning move not found");
    return NO_MOVE;
}

void printPV(Move *pvArray, i32 depth) {
    for (i32 i = 0; i < depth; i++) {
        if(pvArray[i] != NO_MOVE){ 
            printMoveShort(pvArray[i]);
            printf(" ");
        }
    }
}

void printPVInfo(SearchData data){
    printf("info ");
    printf("depth %d ", data.depth);

    i32 score = data.eval;
    if(abs(score) < CHECKMATE_VALUE - MAX_MOVES){
        printf("score cp %d ", score/10);
    }
    else{
        i32 mate = CHECKMATE_VALUE - abs(score);
        mate = (mate + 1) / 2;
        if(score < 0) mate = -mate;
        printf("score mate %d ", mate);
    }

    printf("time %d ", (int)(data.stats.elap_time * 1000));
    printf("nodes %lld ", (long long)data.stats.node_count);
    printf("nps %lld ", (long long)((double)data.stats.node_count / data.stats.elap_time));
    printf("pv ");
    printPV(data.PVArray, data.depth);
    printf("\n");
    fflush(stdout);
}

Stage calculateStage(Position pos){
    Stage stage = MID_GAME;
    if(pos.fullmove_number < OPN_GAME_MOVES) stage = OPN_GAME; 
    if(count_bits(pos.color[0] | pos.color[1]) <= END_GAME_PIECES) stage = END_GAME;
    return stage;
}

/*
*
* Hash Stack Stuff
*
*/

HashStack createHashStack(void){
    u64 *ptr = malloc(sizeof(u64) * GAME_MOVES);
    if (ptr == NULL) {
        printf("info Warning: Failed to allocate space for the Hash Stack.\n");
    }
    HashStack hashStack = {ptr, GAME_MOVES, 0, 0};
    return hashStack;
}

void doubleHashStack(HashStack *hs){
    hs->size *= 2;
    hs->ptr = realloc(hs->ptr, sizeof(u64) * hs->size);
    printf("info string growing hash stack\n");
    if (hs->ptr == NULL) {
        printf("info string Warning: Failed to allocate space for the Hash Stack.\n");
        return;
    }
}

i32 removeHashStack(HashStack *hashStack){
    if (hashStack && hashStack->ptr) {
        free(hashStack->ptr);
        hashStack->ptr = NULL;
    }
    return 0;
}

#ifdef PYTHON
i32 python_init() {
    system("rm /tmp/chess_fifo_in");
    system("rm /tmp/chess_fifo_out");
    system("mkfifo /tmp/chess_fifo_in");
    system("mkfifo /tmp/chess_fifo_out");

    pid = fork();
    if (pid == -1) {
        perror("fork failed");
        return -1;
    }

    if (pid == 0) {
        execlp("python3", "python3", "chessmoves.py", NULL);
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }

    fifo_fd_in = open("/tmp/chess_fifo_in", O_WRONLY);
    fifo_fd_out = open("/tmp/chess_fifo_out", O_RDONLY);
    if (fifo_fd_in == -1 || fifo_fd_out == -1) {
        perror("Error opening FIFO");
        return -1;
    }

    printf("Python running and connected through FIFO\n");

    return 0;
}


static i32 python_movecount(char* fen) {
    char buffer[256];
    ssize_t bytes_read;
    size_t total_bytes_read = 0;

    write(fifo_fd_in, fen, strlen(fen));
    write(fifo_fd_in, "\n", 1);

    memset(buffer, 0, sizeof(buffer));

    do {
        bytes_read = read(fifo_fd_out, buffer + total_bytes_read, sizeof(buffer) - total_bytes_read - 1);
        if (bytes_read == -1) {
            perror("Error reading from FIFO");
            return -1;
        }
        total_bytes_read += bytes_read;
    } while (bytes_read > 0 && buffer[total_bytes_read - 1] != '\n');

    buffer[total_bytes_read] = '\0';

    i32 move_count = atoi(buffer);
    return move_count;
}

i32 python_close() {
    close(fifo_fd_in);
    close(fifo_fd_out);
    kill(pid, SIGKILL);
    unlink("/tmp/chess_fifo_in");
    unlink("/tmp/chess_fifo_out");
    return 0;
}

i32 checkMoveCount(Position pos){
    Move moveList[MAX_MOVES];
    i32 num_moves = generateLegalMoves(pos, moveList);
    char fen[128];
    PositionToFen(pos, fen);
    i32 correct_num_moves = python_movecount(fen);
    if(num_moves != correct_num_moves){
        printf("Incorrect amount of moves found (%d/%d) at pos:\n", num_moves, correct_num_moves);
        printPosition(pos, TRUE);
        for (i32 i = 0; i < num_moves; i++) {
            printMove(moveList[i]);
        }
        return -1;
    }
    return 0;
}

#endif