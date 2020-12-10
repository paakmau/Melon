cmake_minimum_required(VERSION 3.18.2)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_CXX_STANDARD 20)

project(Melon)

find_package(Threads REQUIRED)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)
include(MelonUtils)

add_subdirectory(third_party)
add_subdirectory(libs)
add_subdirectory(examples)
