cmake_minimum_required(VERSION 3.20)
project(chesspp)

set(CMAKE_CXX_STANDARD 17)

add_executable(chesspp
        main.cpp chesspp.cpp chesspp.h)
