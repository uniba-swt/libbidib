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

# Manually add the library directories for yaml
link_directories(/usr/local/Cellar/libyaml/0.2.1/lib)

set(LIBBIDIB_DIR /Users/eugeneyip/Documents/SWTbahn/Software/libbidib)

add_library(bidib SHARED IMPORTED)
set_target_properties(bidib PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
	${LIBBIDIB_DIR}/include)
set_target_properties(bidib PROPERTIES IMPORTED_LOCATION
	${LIBBIDIB_DIR}/build/libbidib.dylib)

add_executable(bidib_system_test src src/bidib_system_test.c)
target_link_libraries(bidib_system_test glib-2.0 pthread yaml bidib)

add_executable(bidib_system_observe src src/bidib_system_observe.c)
target_link_libraries(bidib_system_observe glib-2.0 pthread yaml bidib)
