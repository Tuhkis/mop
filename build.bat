:: MINGW build script.
@echo off

set CC=gcc
set INC=-Iexternal -Iexternal\winlib\SDL2-2.0.10\x86_64-w64-mingw32\include
set CFLAGS=-pipe -O2 -Wall -Wextra -Werror -Wno-comment -Wno-unused-function -std=gnu11 %INC% -Isrc
set LFLAGS=-lmingw32 -lm -lSDL2main -lSDL2 -Lexternal\winlib\SDL2-2.0.10\x86_64-w64-mingw32\lib -mwindows

if not exist bin call mkdir bin

call %CC% -pipe -O3 -c src\external_impl.c -o bin\external_impl.o
call %CC% %CFLAGS% -c src\unity.c -o bin\unity.o

call %CC% bin\*.o %LFLAGS% -o bin\mop.exe
echo done.

