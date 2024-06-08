#include "io.h"
#include "bitboard/bbutils.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "util.h"
#include "globals.h"
#include "movement.h"
#include "search.h"

static char isNullMove(char* moveStr){
    if(moveStr == NULL || strlen(moveStr) < 4) return 0;
    for(i32 i = 0; i < 4; i++){
        if(moveStr[i] != '0') return 0;
    }
    return 1;
}

static char* trimWhitespace(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return str;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
    return str;
}

static void processUCI(void) {
    printf("id name CraigEngine\r\n");
    printf("id author John\r\n");
    printf("uciok\r\n");
}

static void processIsReady(void) {
    printf("readyok\r\n");
    fflush(stdout);
}

static void processMoves(char* str) {
    char* pch;
    char* rest = str; 
    pch = strtok_r(str, " ", &rest);
    
    while (pch != NULL) {
        char* moveStr = trimWhitespace(pch);
        if(isNullMove(moveStr)){
            goto get_next_token;
        }
        //printf("Move String found: %s", moveStr);
        makeMove(&global_position, moveStrToType(global_position, moveStr));
get_next_token:
        pch = strtok_r(NULL, " ", &rest);
    }
}

void processGoCommand(char* input) {
    char* token;
    char* saveptr;
    u32 wtime = 0; // Initialize with default values
    u32 btime = 0;
    u32 time = 0;

    token = strtok_r(input, " ", &saveptr);
    while (token != NULL) {
        if (strcmp(token, "infinite") == 0) {
            time = 0;
        } else if (strcmp(token, "wtime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                wtime = atol(token);
            }
        } else if (strcmp(token, "btime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                btime = atol(token);
            }
        } else if (strcmp(token, "movetime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                time = atol(token);
            }
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    if(global_position.flags & WHITE_TURN){
        time += (wtime / 20);
    }
    else{
        time += (btime / 20);
    }

    startSearch(time);
}

static i32 processInput(char* input){
    if (strncmp(input, "uci", 3) == 0) {
        input += 3;
        if(strncmp(input, "newgame", 7) == 0) return 0;
        processUCI();
        fflush(stdout);
        return 0;
    } 
    else if (strncmp(input, "isready", 7) == 0) {
        processIsReady();
        fflush(stdout);
        return 0;
    }
    else if (strncmp(input, "position", 8) == 0) {
        input += 9;
        global_best_move = NO_MOVE; // Reset best move for current position
        stopSearch();
        if (strncmp(input, "startpos", 8) == 0) {
            input += 9;
            global_position = fenToPosition(START_FEN);
        }
        else if (strncmp(input, "fen", 3) == 0) {
            input += 4;
            global_position = fenToPosition(input);
            while (*input != 'm' && *input != '\n' && *input != '\0') {
                input++;
            }
        }
        if (strncmp(input, "moves", 5) == 0) {
            input += 6;
            processMoves(input);
        }
        fflush(stdout);
    }
    else if (strncmp(input, "go", 2) == 0) {
        processGoCommand(input + 3);
    }
    else if (strncmp(input, "stop", 4) == 0){
        #ifdef DEBUG
        printf("info string Stopping\n");
        #endif
        //printBestMove();
        stopSearch();
    }
    #ifdef DEBUG
    else if (strncmp(input, "debug", 5) == 0){
        input += 6;
        if (strncmp(input, "pos", 3) == 0) {
            printPosition(global_position, TRUE);
        }
        else if (strncmp(input, "bestmove", 8) == 0) {
            printf("Current bestmove is: ");
            printMove(global_best_move);
            printf("\n");
        }
        else if (strncmp(input, "list moves", 10) == 0) {
            Move debug_moves[MAX_MOVES];
            u32 size = generateLegalMoves(global_position, debug_moves);
            printf("Moves: \n");
            for(u32 i = 0; i < size; i++){
                printMove(debug_moves[i]);
                printf("\n");
            }
        }
        else if (strncmp(input, "eval", 4) == 0){
            printf("Eval: %d\n", evaluate(global_position, TRUE));
        }
        else if (strncmp(input, "play move", 4) == 0){
            printf("Making move: ");
            printMove(global_best_move);
            printf("\n");
            makeMove(&global_position, global_best_move);
        }

    }
    #endif
    else if (strncmp(input, "quit", 4) == 0) {
        return -1; 
    }
    return 0;
}

i32 ioLoop(){
    char input[4096];
    input[4095] = '\0';

    while (true) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }
        if(processInput(input)){
            break;
        }
    }
    return 0;
}