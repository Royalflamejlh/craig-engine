@echo off
REM Batch file to compile a C program with cl

REM Check if main.exe exists and delete it if it does
if exist main.exe (
    echo Deleting old main.exe...
    del main.exe
)

REM Compile the program
echo Compiling the program...
cl main.c tree.c util.c evaluator.c board.c

REM Check if the compilation was successful
if exist main.exe (
    echo Compilation successful.
) else (
    echo Compilation failed.
)
