cmake_minimum_required(VERSION 3.15)
project(physical_test)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(physical_test
        CMakeLists.txt
        main.c
        test-suite.c
        test-suite.h)
