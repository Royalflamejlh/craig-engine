uint64_t zobristTable[64][12];

void initZobrist() {
    for (int square = 0; square < 64; square++) {
        for (int piece = 0; piece < 12; piece++) {
            zobristTable[square][piece] = random64bitNumber();
        }
    }
}

uint64_t hashBoard(char board[8][8]) {
    uint64_t hash = 0;
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (board[y][x] != ' ') { 
                int piece = convertPieceToIndex(board[y][x]);
                int square = y * 8 + x;
                hash ^= zobristTable[square][piece];
            }
        }
    }
    return hash;
}

int convertPieceToIndex(char piece) {
    switch (piece) {
        case 'P': return 0;  // White Pawn
        case 'N': return 1;  // White Knight
        case 'B': return 2;  // White Bishop
        case 'R': return 3;  // White Rook
        case 'Q': return 4;  // White Queen
        case 'K': return 5;  // White King
        case 'p': return 6;  // Black Pawn
        case 'n': return 7;  // Black Knight
        case 'b': return 8;  // Black Bishop
        case 'r': return 9;  // Black Rook
        case 'q': return 10; // Black Queen
        case 'k': return 11; // Black King
        default:  return -1; // Invalid piece
    }
}
