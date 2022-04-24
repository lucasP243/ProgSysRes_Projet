cmake_minimum_required(VERSION 3.21)
project (Partie_Client C)

set(CMAKE_C_STANDARD 99)

add_executable(Partie_Client main.c)
if(WIN32)
    target_link_libraries(Partie_Client wsock32 ws2_32)
endif()
