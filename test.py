import chess.engine
import logging

craig_time = 10
fish_time = .05

#logging.basicConfig(level=logging.DEBUG)
         
def play_chess(db_engine_path, engine_path):
    # Initialize the engines
    craig = chess.engine.SimpleEngine.popen_uci(db_engine_path)
    fish = chess.engine.SimpleEngine.popen_uci(engine_path)

    board = chess.Board()
    print(board)
    
    while not board.is_game_over():
        if board.turn == chess.WHITE:
            result = craig.play(board, chess.engine.Limit(time=craig_time))
            print(craig.id.get('name') + " found move " + str(result.move))
        else:
            result = fish.play(board, chess.engine.Limit(time=fish_time))
            print(fish.id.get('name') + " found move " + str(result.move))
        
        print("Press enter to play move, or type in different move")
        move_str = input()
        
        move = result.move
        if(move_str != ""):
            move = chess.Move.from_uci(move_str)
        
        board.push(move)
        print(board)
        print("\n")

    result = board.result()
    fen = board.board_fen()
    print("Game Over. Result: " + result)
    print("Fen: " + fen)

    # Close the engines
    craig.quit()
    fish.quit()

if __name__ == "__main__":
    engine1_path = "./bin/chess"
    engine2_path = "stockfish"
    
    play_chess(engine1_path, engine2_path)
