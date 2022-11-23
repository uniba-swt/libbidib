cmake_minimum_required(VERSION 3.6)
project(libbidib-system-test)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include(FindPkgConfig)
pkg_check_modules(GLIB glib-2.0 REQUIRED)

file(GLOB SRCFILES "src/*.c")

include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

# Include BiDiB library that have been installed
pkg_check_modules(BIDIB bidib REQUIRED)
include_directories(${BIDIB_INCLUDE_DIRS})
link_directories(${BIDIB_LIBRARY_DIRS})

add_executable(bidib_system_test src src/bidib_system_test.c)
target_link_libraries(bidib_system_test glib-2.0 pthread yaml bidib)

add_executable(bidib_system_observe src src/bidib_system_observe.c)
target_link_libraries(bidib_system_observe glib-2.0 pthread yaml bidib)
