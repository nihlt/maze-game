@echo off
setlocal
echo === Assembling game.asm ===
"C:\Program Files\NASM\nasm.exe" -f win64 src\game_v2.0.asm -o obj\game.obj
if %errorlevel% neq 0 exit /b %errorlevel%

echo === Linking game.obj + api.obj (GDI32 + WinHTTP) ===
"C:\msys64\mingw64\bin\gcc.exe" -nostdlib -m64 obj\game.obj obj\api.obj -o bin\game.exe -lkernel32 -luser32 -lgdi32 -lwinhttp -Wl,-e,mainCRTStartup -s -mwindows
if %errorlevel% neq 0 exit /b %errorlevel%

echo === Build OK! Starting bin\game.exe ===
bin\game.exe
