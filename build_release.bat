@echo off
set CLANG=clang++
set FLAGS=-Os -Wall -Wpedantic -Wextra -Wconversion -Wshadow -Wno-missing-braces -Wno-c++20-designator

set WINSOCKLIB=ws2_32


if not exist build mkdir build



cd ./build

%CLANG% ../src/*.cpp -l %WINSOCKLIB% %FLAGS% -o chat.exe

cd ..
