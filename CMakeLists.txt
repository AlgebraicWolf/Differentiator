cmake_minimum_required(VERSION 3.15)
project(Differentiator)

set(CMAKE_CXX_STANDARD 14)

add_executable(Differentiator main.cpp)

add_library(Tree ../Tree/Tree.cpp)

target_link_libraries(Differentiator Tree)