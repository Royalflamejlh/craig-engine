#include "tree.h"
#include "movement.h"
#include <ctype.h>

static void getLegalPawnMoves(struct Node *node, char x, char y);
static void getLegalBishopMoves(struct Node *node, char x, char y);
static void getLegalKnightMoves(struct Node *node, char x, char y);
static void getLegalRookMoves(struct Node *node, char x, char y);
static void getLegalKingMoves(struct Node *node, char x, char y);

/*
* Takes in a node and generates all legal moves for that node
*/
void buildLegalMoves(struct Node *node){
    for (char y = 0; y < 8; y++) {
        for (char x = 0; x < 8; x++) {
            char piece = node->board[y][x];
            if (piece == ' ' || (node->color == 'B' && islower(piece)) || (node->color == 'W' && isupper(piece))) {
                continue;
            }
            switch (piece) {
                case 'P':
                case 'p':
                    getLegalPawnMoves(node, x, y);
                    break;
                case 'B': 
                case 'b':
                    getLegalBishopMoves(node, x, y);
                    break;
                case 'N': 
                case 'n':
                    getLegalKnightMoves(node, x, y);
                    break;
                case 'R': 
                case 'r':
                    getLegalRookMoves(node, x, y);
                    break;
                case 'Q': 
                case 'q':
                    getLegalBishopMoves(node, x, y);
                    getLegalRookMoves(node, x, y);
                    break;
                case 'K': 
                case 'k':
                    getLegalKingMoves(node, x, y);
                    break;
                default:
                    break;
            }
        }
    }
}

static int64_t makeMove(char from_x, char from_y, char to_x, char to_y){
    int64_t result = 0;
    result = from_x;
    result = (result << 8) | from_y;
    result = (result << 8) | to_x;
    result = (result << 8) | to_y;
    return result;
}


static void addMoveIfValid(struct Node *node, char from_x, char from_y, char to_x, char to_y) {
    //Make sure the not mate function here
    int64_t move = makeMove(from_x, from_y, to_x, to_y);
    addTreeNode(node, move, STATUS_PREDICTED, 0);
}


static void getLegalPawnMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];
    char direction = isupper(piece) ? 1 : -1;

    char forward_y = y + direction;
    if (node->board[forward_y][x] == ' ') {
        addMoveIfValid(node, x, y, x, forward_y);

        if ((isupper(piece) && y == 1) || (islower(piece) && y == 6)) {
            char twoForward_y = y + 2 * direction;
            if (node->board[twoForward_y][x] == ' ') {
                addMoveIfValid(node, x, y, x, twoForward_y);
            }
        }
    }

    for (int dx = -1; dx <= 1; dx += 2) { 
        char capture_x = x + dx;
        char capture_y = y + direction;

        if (capture_x >= 0 && capture_x < 8 && capture_y >= 0 && capture_y < 8) {
            char target = node->board[capture_y][capture_x];
            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                addMoveIfValid(node, x, y, capture_x, capture_y);
            }
        }
    }
}

static void getLegalRookMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];

    int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}}; 

    for (int i = 0; i < 4; i++) {
        char dx = directions[i][1];
        char dy = directions[i][0];

        char new_x = x + dx;
        char new_y = y + dy;

        while (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if ((isupper(piece) && isupper(target)) || (islower(piece) && islower(target))) {
                break;
            }

            addMoveIfValid(node, x, y, new_x, new_y);

            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                break;
            }

            new_x += dx;
            new_y += dy;
        }
    }
}


static void getLegalBishopMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];

    int directions[4][2] = {{-1, 1}, {-1, -1}, {1, 1}, {1, -1}}; 

    for (int i = 0; i < 4; i++) {
        char dx = directions[i][1];
        char dy = directions[i][0];

        char new_x = x + dx;
        char new_y = y + dy;

        while (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if ((isupper(piece) && isupper(target)) || (islower(piece) && islower(target))) {
                break;
            }

            addMoveIfValid(node, x, y, new_x, new_y);

            if ((isupper(piece) && islower(target)) || (islower(piece) && isupper(target))) {
                break;
            }

            new_x += dx;
            new_y += dy;
        }
    }
}


static void getLegalKnightMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];
\
    int moves[8][2] = {{-2, 1}, {-1, 2}, {1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}};

    for (int i = 0; i < 8; i++) {
        char new_x = x + moves[i][1];
        char new_y = y + moves[i][0];

        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if (!((isupper(piece) && islower(target)) || (islower(piece) && isupper(target)) || target == ' ')) {
                continue;
            }

            addMoveIfValid(node, x, y, new_x, new_y);
        }
    }
}


static void getLegalKingMoves(struct Node *node, char x, char y) {
    char piece = node->board[y][x];

    int moves[8][2] = {{-1, 0}, {-1, 1}, {0, 1}, {1, 1},
                       {1, 0}, {1, -1}, {0, -1}, {-1, -1}};

    for (int i = 0; i < 8; i++) {
        char new_x = x + moves[i][1];
        char new_y = y + moves[i][0];

        if (new_x >= 0 && new_x < 8 && new_y >= 0 && new_y < 8) {
            char target = node->board[new_y][new_x];

            if (!((isupper(piece) && isupper(target)) || (islower(piece) && islower(target)))) {
                addMoveIfValid(node, x, y, new_x, new_y);
            }
        }
    }
}
