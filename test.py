import os
import sys
import subprocess
import urllib.request
import zipfile
import multiprocessing

def main():
    if len(sys.argv) != 3:
        print("Usage: python test.py engine1.exe engine2.exe")
        sys.exit(1)

    engine1 = sys.argv[1]
    engine2 = sys.argv[2]

    engine1_path = os.path.abspath(engine1)
    engine2_path = os.path.abspath(engine2)

    engine1_name =  engine1.rsplit('.')[0].rsplit('/')[-1]
    engine2_name =  engine2.rsplit('.')[0].rsplit('/')[-1]

    test_dir = 'test'
    if not os.path.exists(test_dir):
        os.makedirs(test_dir)

    # Download and unzip the opening book
    pgn_zip_url = 'https://github.com/official-stockfish/books/raw/master/8moves_v3.pgn.zip'
    pgn_zip_path = os.path.join(test_dir, '8moves_v3.pgn.zip')
    pgn_file_path = os.path.join(test_dir, '8moves_v3.pgn')

    if not os.path.exists(pgn_file_path):
        if not os.path.exists(pgn_zip_path):
            print("Downloading 8moves_v3.pgn.zip...")
            urllib.request.urlretrieve(pgn_zip_url, pgn_zip_path)
        print("Unzipping 8moves_v3.pgn.zip...")
        with zipfile.ZipFile(pgn_zip_path, 'r') as zip_ref:
            zip_ref.extractall(test_dir)

    # Download and build Fastchess
    fastchess_dir = os.path.join(test_dir, 'fastchess')
    fastchess_exe = os.path.join(fastchess_dir, 'fastchess')

    if not os.path.exists(fastchess_exe):
        if not os.path.exists(fastchess_dir):
            print("Cloning Fastchess repository...")
            subprocess.run(['git', 'clone', 'https://github.com/Disservin/fastchess.git', fastchess_dir])
        print("Building Fastchess...")
        cwd = os.getcwd()
        os.chdir(fastchess_dir)
        subprocess.run(['make', '-j'])
        os.chdir(cwd)

    threads = multiprocessing.cpu_count()

    command = [
        fastchess_exe,
        '-engine', f'cmd={engine1_path}', f'name={engine1_name}',
        '-engine', f'cmd={engine2_path}', f'name={engine2_name}',
        '-each', 'tc=8+0.08',
        '-rounds', '15000',
        '-repeat',
        '-concurrency', str(threads),
        '-recover',
        '-openings', f'file={pgn_file_path}', 'format=pgn',
        '-sprt', 'elo0=0', 'elo1=5', 'alpha=0.05', 'beta=0.05'
    ]

    print("Running Fastchess tournament...")
    subprocess.run(command)

if __name__ == "__main__":
    main()
