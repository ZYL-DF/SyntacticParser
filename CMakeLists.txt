cmake_minimum_required(VERSION 3.25)
project(SyntacticParser)

set(CMAKE_CXX_STANDARD 20)

add_executable(SyntacticParser LR1_Parser.cpp)
