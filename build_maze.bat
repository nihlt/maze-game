@echo off
setlocal

:: Configuration
set COMPILER=C:\msys64\mingw64\bin\g++.exe
set SRC=src\maze.cpp
set OUT=bin\maze.exe
set LIBS=-lgdi32

:: Ensure bin directory exists
if not exist bin mkdir bin

echo Building %SRC%...

:: -static-libgcc -static-libstdc++ -static ensures no DLL dependencies for Mingw
"%COMPILER%" -O2 -mwindows -static -static-libgcc -static-libstdc++ "%SRC%" %LIBS% -o "%OUT%"

if %ERRORLEVEL% neq 0 (
    echo [ERROR] Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo [SUCCESS] Built to %OUT%
echo.
echo Running %OUT%...
"%OUT%"

endlocal
