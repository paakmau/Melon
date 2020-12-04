cmake_minimum_required(VERSION 3.10)
project(vkmemalloc)

add_library(vkmemalloc INTERFACE)
target_include_directories(vkmemalloc INTERFACE ./src)
