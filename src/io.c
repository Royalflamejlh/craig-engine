#include "io.h"
#include "bitboard/bbutils.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "util.h"
#include "globals.h"
#include "movement.h"
#include "search.h"

#ifdef DEBUG
#include "evaluator.h"
#endif

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
        Position cur = get_global_position();
        makeMove(&cur, moveStrToType(cur, moveStr));
        set_global_position(cur);
get_next_token:
        pch = strtok_r(NULL, " ", &rest);
    }
}

void processGoCommand(char* input) {
    char* token;
    char* saveptr;
    u32 wtime = 0; // Initialize with default values
    u32 winc = 0;
    u32 btime = 0;
    u32 binc = 0;
    u32 movetime = 0;

    u32 time = 0;
    u32 depth = MAX_DEPTH;

    token = strtok_r(input, " ", &saveptr);
    while (token != NULL) {
        if (strcmp(token, "infinite") == 0) {
            time = 0;
        } else if (strcmp(token, "wtime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                wtime = atol(token);
            }
        } else if (strcmp(token, "winc") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                winc = atol(token);
            }
        } else if (strcmp(token, "btime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                btime = atol(token);
            }
        } else if (strcmp(token, "binc") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                binc = atol(token);
            }
        } else if (strcmp(token, "movetime") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                movetime = atol(token);
            }
        } else if (strcmp(token, "depth") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                depth = atol(token);
            }
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    if(movetime != 0) time = movetime;
    else{ 
        time = calculate_search_time(wtime, winc, btime, binc, get_global_position().flags & WHITE_TURN);
    }
    
    startSearch(time, depth);
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
        stopSearch();
        if (strncmp(input, "startpos", 8) == 0) {
            input += 9;
            set_global_position(fenToPosition(START_FEN));
        }
        else if (strncmp(input, "fen", 3) == 0) {
            input += 4;
            set_global_position(fenToPosition(input));
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
        print_best_move = TRUE;
        stopSearch();
    }
    else if (strncmp(input, "quit", 4) == 0){
        printf("info string Closing Engine\n");
        fflush(stdout);
        stopSearch();
        run_program = FALSE;
        return 0;
    }
    #ifdef DEBUG
    else if (strncmp(input, "debug", 5) == 0){
        input += 6;
        if (strncmp(input, "pos", 3) == 0) {
            printPosition(get_global_position(), TRUE);
        }
        else if (strncmp(input, "bestmove", 8) == 0) {
            printf("Current bestmove is: ");
            printMove(get_global_best_move());
            printf("\n");
        }
        else if (strncmp(input, "list moves", 10) == 0) {
            Move debug_moves[MAX_MOVES];
            u32 size = generateLegalMoves(get_global_position(), debug_moves);
            printf("Moves: \n");
            for(u32 i = 0; i < size; i++){
                printMove(debug_moves[i]);
                printf("\n");
            }
        }
        else if (strncmp(input, "eval", 4) == 0){
            printf("Eval: %d\n", evaluate(get_global_position(), TRUE));
        }
        else if (strncmp(input, "play move", 4) == 0){
            printf("Making move: ");
            printMove(get_global_best_move());
            printf("\n");
            Position tempPos = get_global_position();
            makeMove(&tempPos, get_global_best_move());
            set_global_position(tempPos);
        }

    }
    #endif
    return 0;
}

i32 inputLoop(){
    char input[4096];
    input[4095] = '\0';

    while (run_program) {
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }
        if(processInput(input)){
            break;
        }
    }
    return 0;
}

i32 outputLoop(){
    while(run_program){
        if(print_pv_info){
            SearchData data = get_global_pv_data();
            printPVInfo(data);
            print_pv_info = FALSE;
            free(data.PVArray);
        }
        if(print_best_move){
            Move move = get_global_best_move();
            if(move != NO_MOVE) printBestMove(move);
            print_best_move = FALSE;
        }
    }
    return 0;
}