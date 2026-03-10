@echo off
g++ -O3 src-2/test_keys.cpp src-2/GameLogic.cpp src-2/EnemyLogic.cpp src-2/Renderer.cpp -o bin/test_keys.exe -lgdi32 -mwindows
if %ERRORLEVEL% EQU 0 (
    echo Build successful: bin/test_keys.exe
) else (
    echo Build failed
)
