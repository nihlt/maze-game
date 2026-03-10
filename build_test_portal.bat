@echo off
if not exist bin mkdir bin
echo Compiling Portal Test...
g++ src-2/tests/test_portal.cpp src-2/logic/GameLogic.cpp src-2/logic/EnemyLogic.cpp src-2/render/Renderer.cpp src-2/render/Renderer3D.cpp src-2/render/RendererUI.cpp src-2/render/RendererArchitect.cpp src-2/logic/MazeGenerator.cpp -Isrc-2/core -Isrc-2/logic -Isrc-2/render -o bin/test_portal.exe -lgdi32 -mwindows
if %ERRORLEVEL% EQU 0 (
    echo Build successful: bin/test_portal.exe
    echo Running Portal Test...
    start bin\test_portal.exe
) else (
    echo Build failed
    pause
)
