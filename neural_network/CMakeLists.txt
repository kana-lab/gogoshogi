cmake_minimum_required(VERSION 3.16)
project(nn_main)

set(CMAKE_C_STANDARD 11)

add_executable(
        nn_main
        train.c
        ../Action.c
        ../Board.c
        ../Game.c
        ../gamedef.c
        ../Hash.c)

target_link_libraries(nn_main PRIVATE m)
