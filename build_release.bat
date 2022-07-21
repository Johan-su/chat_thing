@echo off
set CLANG=clang++
set FLAGS=-O3 -Wall -Wpedantic -Wno-c++20-designator

set WINSOCKLIB=ws2_32


if not exist build mkdir build



cd ./build

%CLANG% ../src/*.cpp -l %WINSOCKLIB% %FLAGS% -o chat_0.0.1.exe

cd ..
