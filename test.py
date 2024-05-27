import chess.engine
import logging
                    
def play_chess(db_engine_path, engine_path, time_limit=0.1):
    # Initialize the engines
    engine1 = chess.engine.SimpleEngine.popen_uci(db_engine_path)
    engine2 = chess.engine.SimpleEngine.popen_uci(engine_path)

    board = chess.Board()
    print(board)
    
    while not board.is_game_over():
        if board.turn == chess.WHITE:
            result = engine1.play(board, chess.engine.Limit(time=time_limit))
            print(engine1.id.get('name') + " found move " + str(result.move))
        else:
            result = engine2.play(board, chess.engine.Limit(time=time_limit))
            print(engine2.id.get('name') + " found move " + str(result.move))
        
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
    engine1.quit()
    engine2.quit()

if __name__ == "__main__":
    engine1_path = "./bin/chess_db"
    engine2_path = "stockfish"
    
    play_chess(engine1_path, engine2_path)
