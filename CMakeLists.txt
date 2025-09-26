cmake_minimum_required(VERSION 4.1)

set(CMAKE_C_STANDARD 11)

project("GameOfLife")

add_compile_options(-Wall -Wextra -Wpedantic)
add_executable("GameOfLife" "src/gameOfLife.c")
