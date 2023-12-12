#!/usr/bin/env python3
import chess
import os

fifo_in_path = '/tmp/chess_fifo_in'
fifo_out_path = '/tmp/chess_fifo_out'

def process_fen(fen):
    board = chess.Board(fen)
    moves = list(board.legal_moves)
    return len(moves)

def main():
    while True:
        with open(fifo_in_path, 'r') as fifo_in, open(fifo_out_path, 'w') as fifo_out:
            for line in fifo_in:
                fen = line.strip()
                move_count = process_fen(fen)
                fifo_out.write(f"{move_count}\n")
                fifo_out.flush()

if __name__ == "__main__":
    main()
