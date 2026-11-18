cmake_minimum_required(VERSION 3.16) 
project(archios_pingpong CXX)
set(CMAKE_CXX_STANDARD 17)

add_executable(server server/main.cpp)
add_executable(client client/main.cpp)

target_compile_options(server PRIVATE -Wall -Wextra)
target_compile_options(client PRIVATE -Wall -Wextra)