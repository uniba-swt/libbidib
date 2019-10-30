cmake_minimum_required(VERSION 3.6)
project(test-suite C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -Wall)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include(FindPkgConfig)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
include_directories(${GLIB_INCLUDE_DIRS})

file(GLOB SRCFILES "src/*.c")

set(LIBBIDIB_DIR /home/pi/SWTbahn/libbidib)

add_library(bidib_static STATIC IMPORTED)
set_target_properties(bidib_static PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
        ${LIBBIDIB_DIR}/include)
set_target_properties(bidib_static PROPERTIES IMPORTED_LOCATION
        ${LIBBIDIB_DIR}/bin/libbidib_static.a)

add_executable(test-suite main.c)
target_link_libraries(test-suite glib-2.0 pthread yaml bidib_static)

