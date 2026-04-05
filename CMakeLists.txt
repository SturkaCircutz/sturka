cmake_minimum_required(VERSION 3.16)
project(sturka LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sturka
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/interpreter.cpp
)
