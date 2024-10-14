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
        ThreadData td = copy_global_td();
        make_move(&td, moveStrToType(&td.pos, moveStr));
        set_global_td(td);
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
    u32 movestogo = 0;
    u8 infinite = FALSE;

    SearchParameters params;
    params.depth = MAX_DEPTH - 1;

    token = strtok_r(input, " ", &saveptr);
    if(token == NULL) infinite = TRUE; // If the user only said "go" then we want to run infinite
    while (token != NULL) {
        if (strncmp(token, "infinite", 8) == 0) {
            infinite = TRUE;
            break;
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
        } else if (strcmp(token, "movestogo") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                movestogo = atol(token);
            }
        }else if (strcmp(token, "depth") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                params.depth = atol(token);
            }
        }else if (strcmp(token, "perft") == 0) {
            token = strtok_r(NULL, " ", &saveptr);
            if (token != NULL) {
                ThreadData temp_td = {0};
                temp_td.pos = copy_global_position();
                perft(&temp_td, atol(token), TRUE);
            }
            return;
        } else if (strncmp(token, "perft", 5) == 0) {
            ThreadData temp_td = {0};
            temp_td.pos = copy_global_position();
            perft(&temp_td, MAX_DEPTH, TRUE);
            return;
        }
        token = strtok_r(NULL, " ", &saveptr);
    }

    if(movetime != 0){
        params.max_time = movetime;
        params.rec_time = movetime;
        params.can_shorten = FALSE;
    } else if(infinite == TRUE){
        params.rec_time = 0;
        params.max_time = 0;
        params.can_shorten = FALSE;
    } else{ 
        params.max_time = calculate_max_search_time(wtime, winc, btime, binc, movestogo, copy_global_position().flags & WHITE_TURN);
        params.rec_time = calculate_rec_search_time(wtime, winc, btime, binc, movestogo, copy_global_position().flags & WHITE_TURN);
        params.can_shorten = TRUE;
    }
    
    start_search(params);
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
            set_global_position(fen_to_position(START_FEN));
        }
        else if (strncmp(input, "fen", 3) == 0) {
            input += 4;
            set_global_position(fen_to_position(input));
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
        #ifdef DEBUG_PRINT
        printf("info string Stopping\n");
        #endif
        stopSearch();
        print_best_move = TRUE;
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
            printPosition(copy_global_position(), TRUE);
        }
        else if (strncmp(input, "bestmove", 8) == 0) {
            printf("Current bestmove is: ");
            printMove(get_global_best_move());
            printf("\n");
        }
        else if (strncmp(input, "list moves", 10) == 0) {
            Move debug_moves[MAX_MOVES];
            Position tempPos = copy_global_position();
            u32 size = generateLegalMoves(&tempPos, debug_moves);
            printf("Moves: \n");
            for(u32 i = 0; i < size; i++){
                printMove(debug_moves[i]);
                printf("\n");
            }
        }
        else if (strncmp(input, "eval", 4) == 0){
            Position tempPos = copy_global_position();
            printf("Eval: %d\n", eval_position(&tempPos));
        }
        else if (strncmp(input, "play move", 4) == 0){
            printf("Making move: ");
            printMove(get_global_best_move());
            printf("\n");
            ThreadData td = copy_global_td();
            make_move(&td, get_global_best_move());
            set_global_td(td);
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
            free(data.pv_array);
        }
        if(print_best_move){
            Move move = get_global_best_move();
            if(move != NO_MOVE) printBestMove(move);
            #ifdef DEBUG
            else printf("NO BEST MOVE FOUND!");
            #endif
            print_best_move = FALSE;
        }
    }
    return 0;
}