import chess
import chess.engine
import logging

# Configuration
craig_time = 1  # Time in seconds Craig has for each move
fish_time = 1  # Time in seconds Stockfish has for each move
num_games = 10  # Number of games to be played for the calculation

#logging.basicConfig(level=logging.DEBUG)


def play_game(white_engine, black_engine, white_time, black_time, white_depth, engine, turn):
    board = chess.Board()
    while not board.is_game_over():
        if board.turn == chess.WHITE:
            try:
                result = white_engine.play(board, chess.engine.Limit(time=white_time, depth=white_depth))
            except TimeoutError:
                print("White to move:")
                print(board)
                print(board.fen())
                print(board.epd())
                return TimeoutError
        else:
            try:
                result = black_engine.play(board, chess.engine.Limit(time=black_time, depth=white_depth))
            except TimeoutError:
                print("Black to move:")
                print(board)
                print(board.fen())
                print(board.epd())
                return TimeoutError
        board.push(result.move)
    return board.result()

def calculate_elo_difference(games_won, games_lost, games_drawn):
    """ Calculate the approximate Elo difference using a simple model. """
    total_games = games_won + games_lost + games_drawn
    if total_games == 0:
        return 0  # Avoid division by zero if no games are played
    
    score = games_won + 0.5 * games_drawn
    win_rate = score / total_games

    if win_rate == 1:
        return float('inf')  # Infinity, cannot calculate log10 of infinity
    elif win_rate == 0:
        return float('-inf')  # Negative infinity for complete loss
    else:
        return 400 * math.log10(win_rate / (1 - win_rate))


def main(db_engine_path, engine_path):
    craig_wins = 0
    fish_wins = 0
    draws = 0

    for i in range(num_games):
        # Initialize engines
        craig = chess.engine.SimpleEngine.popen_uci(db_engine_path)
        fish = chess.engine.SimpleEngine.popen_uci(engine_path)
        
        # Alternate starting colors
        if i % 2 == 0:
            result = play_game(craig, fish, craig_time, fish_time, 6, fish, i)
        else:
            result = play_game(fish, craig, fish_time, craig_time, 6, fish, i)
        
        if result == "1-0":
            craig_wins += 1 if i % 2 == 0 else 0
            fish_wins += 1 if i % 2 != 0 else 0
        elif result == "0-1":
            fish_wins += 1 if i % 2 == 0 else 0
            craig_wins += 1 if i % 2 != 0 else 0
        else:
            draws += 1
        
        # Close the engines
        craig.quit()
        fish.quit()
        print(f"Game {i+1} completed. {craig_wins} wins for Craig, {fish_wins} wins for {engine2_name}")

    elo_diff = calculate_elo_difference(craig_wins, fish_wins, draws)
    print(f"Elo difference: {elo_diff} (positive means {engine1_name} is stronger)")
    print(f"Results: {craig_wins} wins for {engine1_name}, {fish_wins} wins for {engine2_name}, {draws} draws")

if __name__ == "__main__":
    import sys
    import math

    engine1_path = "./bin/chess"
    engine1_name = "Craig Testing"
    engine2_path = "./bin/chess"
    engine2_name = "CraigEngine V0.1"
    
    main(engine1_path, engine2_path)
