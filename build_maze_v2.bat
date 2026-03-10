@echo off
if not exist bin mkdir bin
g++ src-2/main.cpp src-2/logic/GameLogic.cpp src-2/logic/EnemyLogic.cpp src-2/render/Renderer.cpp src-2/render/Renderer3D.cpp src-2/render/RendererUI.cpp src-2/render/RendererArchitect.cpp src-2/logic/MazeGenerator.cpp -Isrc-2/core -Isrc-2/logic -Isrc-2/render -o bin/maze_v2.exe -lgdi32 -mwindows
if %ERRORLEVEL% EQU 0 (
    echo Build successful: bin/maze_v2.exe
    start bin\maze_v2.exe
) else (
    echo Build failed
)
