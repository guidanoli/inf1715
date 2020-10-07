cmake_minimum_required(VERSION 3.0)
project(monga)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(MONGA_TESTING "Allow unit tests" ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

if (MONGA_TESTING)
    find_program(BASH bash)
    enable_testing()
endif ()

add_subdirectory(src)
