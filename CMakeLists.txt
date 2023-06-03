cmake_minimum_required(VERSION 3.4)
project(demangle VERSION 1.0)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})


add_executable(demangle main.cpp)
target_compile_features(demangle PUBLIC cxx_std_20)
