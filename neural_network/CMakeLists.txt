cmake_minimum_required(VERSION 3.16)
project(NeuralNetwork)

set(CMAKE_C_STANDARD 11)

add_executable(
        NeuralNetwork
        train.c
        ../Action.c
        ../Board.c
        ../Game.c
        ../gamedef.c)

target_link_libraries(NeuralNetwork PRIVATE m)