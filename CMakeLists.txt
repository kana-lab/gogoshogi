cmake_minimum_required(VERSION 3.19)
project(gogoshogi C)

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
        gamedef.h
        Hash.c
        Hash.h
        MultiThread.c
        MultiThread.h
        neural_network/minimax.c
        neural_network/neural_network.h)


set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(main PRIVATE m Threads::Threads)