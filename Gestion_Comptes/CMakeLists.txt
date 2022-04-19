cmake_minimum_required(VERSION 3.21)
project(Gestion_Comptes C)

set(CMAKE_C_STANDARD 99)

add_executable(Gestion_Comptes main.c user_database_engine.h user_database_engine.c)
if(WIN32)
    target_link_libraries(Gestion_Comptes wsock32 ws2_32)
endif()
