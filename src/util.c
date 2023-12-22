#include "util.h"
#include "./bitboard/bbutils.h"
#include "movement.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#ifdef PYTHON
static int fifo_fd_in, fifo_fd_out;
static pid_t pid;
#endif

void printMove(Move move){
    int from = GET_FROM(move);
    int to = GET_TO(move);

    int rank_from = from / 8;
    int file_from = from % 8;

    int rank_to = to / 8;
    int file_to = to % 8;

    char file_char_from = 'a' + file_from;
    char file_char_to = 'a' + file_to;

    int rank_num_from = rank_from + 1;
    int rank_num_to = rank_to + 1;

    printf("%c%d%c%d", file_char_from, rank_num_from, file_char_to, rank_num_to);

    switch(GET_FLAGS(move)){
        case DOUBLE_PAWN_PUSH:
            printf(" (DPP)");
            break;
        case KING_CASTLE:
            printf(" (KCstl)");
            break;
        case QUEEN_CASTLE:
            printf(" (QCstl)");
            break;
        case CAPTURE:
            printf(" (Cap)");
            break;
        case EP_CAPTURE:
            printf(" (EPCap)");
            break;
        case KNIGHT_PROMOTION:
            printf(" (NPro)");
            break;
        case BISHOP_PROMOTION:
            printf(" (BPro)");
            break;
        case ROOK_PROMOTION:
            printf(" (RPro)");
            break;
        case QUEEN_PROMOTION:
            printf(" (QPro)");
            break;
        case KNIGHT_PROMO_CAPTURE:
            printf(" (NProCap)");
            break;
        case BISHOP_PROMO_CAPTURE:
            printf(" (BProCap)");
            break;
        case ROOK_PROMO_CAPTURE:
            printf(" (RProCap)");
            break;
        case QUEEN_PROMO_CAPTURE:
            printf(" (QProCap)");
            break;
        case QUIET:
        default:
            break;
    }
}




uint64_t perft(int depth, Position pos){
  Move move_list[256];
  int n_moves, i;
  uint64_t nodes = 0;

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

char getPiece(Position pos, int square){
    return pos.charBoard[square];
}

Move moveStrToType(Position pos, char* str){
    Move moveList[MAX_MOVES] = {0};
    int size = generateLegalMoves(pos, moveList);

    int from  = (str[0] - 'a') + ((str[1] - '1') * 8);
    int to = (str[2] - 'a') + ((str[3] - '1') * 8);
    char promo = str[4];

    if(str[4] != '\0'){
        while(TRUE){
            printf("ERROR WILL SHIT SELF IMPLEMENT PROMO");
        }
    }

    for(int i = 0; i < size; i++){
        int fromCur = GET_FROM(moveList[i]);
        int toCur = GET_TO(moveList[i]);
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

void printPV(Move *pvArray, int depth) {
    printf("Principal Variation at depth %d: ", depth);
    for (int i = 0; i < depth; i++) {
        if(pvArray[i] != NO_MOVE){ 
            printMove(pvArray[i]);
            printf(" ");
        }
    }
}


#ifdef PYTHON
int python_init() {
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


static int python_movecount(char* fen) {
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

    int move_count = atoi(buffer);
    return move_count;
}

int python_close() {
    close(fifo_fd_in);
    close(fifo_fd_out);
    kill(pid, SIGKILL);
    unlink("/tmp/chess_fifo_in");
    unlink("/tmp/chess_fifo_out");
    return 0;
}

int checkMoveCount(Position pos){
    Move moveList[MAX_MOVES];
    int num_moves = generateLegalMoves(pos, moveList);
    char fen[128];
    PositionToFen(pos, fen);
    int correct_num_moves = python_movecount(fen);
    if(num_moves != correct_num_moves){
        printf("Incorrect amount of moves found (%d/%d) at pos:\n", num_moves, correct_num_moves);
        printPosition(pos, TRUE);
        for (int i = 0; i < num_moves; i++) {
            printMove(moveList[i]);
        }
        return -1;
    }
    return 0;
}

#endif