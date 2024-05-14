from chester.timecontrol import TimeControl
from chester.tournament import play_tournament

players = ["./bin/chess_db", "stockfish"]

time_control = TimeControl(initial_time=3, increment=0)

n_games = 1

for pgn in play_tournament(
    players,
    time_control,
    n_games=n_games,
):
    print(pgn, "\n")