cmake_minimum_required(VERSION 3.21)
project(Partie_Centralisee C)

set(CMAKE_C_STANDARD 99)

add_executable(Partie_Centralisee main.c user_database_handler.c server_handler.c)
if(WIN32)
    target_link_libraries(Partie_Centralisee wsock32 ws2_32)
endif()
if(UNIX)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(Partie_Centralisee PRIVATE Threads::Threads)
endif()
