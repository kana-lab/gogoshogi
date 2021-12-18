cmake_minimum_required(VERSION 3.20)
project(C)

set(CMAKE_C_STANDARD 11)

add_executable(
        main
        main.c
        Action.c
        Action.h
        Board.c
        Board.h
        Game.c
        Game.h
        gamedef.c
        gamedef.h)
