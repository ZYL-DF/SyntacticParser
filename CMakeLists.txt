cmake_minimum_required(VERSION 3.25)
project(SyntacticParser)

set(CMAKE_CXX_STANDARD 11)

add_executable(SyntacticParser LL1_Parser.cpp)
