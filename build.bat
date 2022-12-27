@echo off
set CLANG=clang++
set FLAGS=-O0 -D _DEBUG -g -gcodeview -Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-missing-braces -Wno-c++20-designator -Wno-c++17-extensions -Wno-variadic-macros -Wno-gnu-zero-variadic-macro-arguments

set WINSOCKLIB=ws2_32


if not exist build mkdir build



cd ./build

%CLANG% ../src/main.cpp -l %WINSOCKLIB% %FLAGS% -o chat.exe

cd ..
