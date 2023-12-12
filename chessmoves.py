import chess
import sys

def main():
    if len(sys.argv) != 2:
        print("Usage: chessmoves.py 'fen_string'")
        return

    fen = sys.argv[1]
    board = chess.Board(fen)
    moves = list(board.legal_moves)
    print(len(moves))

if __name__ == "__main__":
    main()