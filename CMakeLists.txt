cmake_minimum_required(VERSION 3.10)
project(lispterp CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(lispterp src/main.cpp)
target_compile_options(lispterp PRIVATE -Wall -Wextra)
