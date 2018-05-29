# Network StateMachine Library
# @author Haoran Luo

cmake_minimum_required(VERSION 3.5)
project(ntstm)

include_directories(include)
add_library(ntstm STATIC src/io.cpp)
