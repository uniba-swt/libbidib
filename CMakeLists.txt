cmake_minimum_required(VERSION 3.6)
project(libbidib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include(FindPkgConfig)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
include_directories(${GLIB_INCLUDE_DIRS})

file(GLOB SRCFILES
        "src/*/*.c"
		)


# - - - - - - - - - - - - -
# ADJUSTMENT SECTION BEGIN
# - - - - - - - - - - - - -


# If you have cmocka installed system wide, comment out the following six lines.
# Otherwise adjust the paths according to your installation.
#set(CMOCKA_DIR /home/nicolas/git/cmocka)
#add_library(cmocka SHARED IMPORTED)
#set_target_properties(cmocka PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
#	${CMOCKA_DIR}/include)
#set_target_properties(cmocka PROPERTIES IMPORTED_LOCATION
#	${CMOCKA_DIR}/bin/src/libcmocka.so.0.4.1)

# You can add all other libraries which you have not installed system wide by the
# above pattern.


# - - - - - - - - - - - - -
# ADJUSTMENT SECTION END
# - - - - - - - - - - - - -


enable_testing()
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)


add_library(bidib_static STATIC include src ${SRCFILES})

add_library(bidib SHARED include src ${SRCFILES})

# Unit tests

add_executable(bidib_send_tests test test/unit/bidib_send_tests.c)
target_link_libraries(bidib_send_tests glib-2.0 cmocka pthread yaml bidib_static)
add_test(bidib_send_tests bidib_send_tests)

add_executable(bidib_receive_tests test test/unit/bidib_receive_tests.c)
target_link_libraries(bidib_receive_tests glib-2.0 cmocka pthread yaml bidib_static)
add_test(bidib_receive_tests bidib_receive_tests)

add_executable(bidib_lowlevel_message_tests test test/unit/bidib_lowlevel_message_tests.c)
target_link_libraries(bidib_lowlevel_message_tests glib-2.0 cmocka pthread yaml bidib_static)
add_test(bidib_lowlevel_message_tests bidib_lowlevel_message_tests)

add_executable(bidib_config_parser_tests test test/unit/bidib_config_parser_tests.c)
target_link_libraries(bidib_config_parser_tests glib-2.0 cmocka pthread yaml bidib_static)
add_test(bidib_config_parser_tests bidib_config_parser_tests)

add_executable(bidib_highlevel_message_tests test test/unit/bidib_highlevel_message_tests.c)
target_link_libraries(bidib_highlevel_message_tests glib-2.0 cmocka pthread yaml bidib_static)
add_test(bidib_highlevel_message_tests bidib_highlevel_message_tests)

add_executable(bidib_state_tests test test/unit/bidib_state_tests.c)
target_link_libraries(bidib_state_tests glib-2.0 cmocka pthread yaml bidib_static)
add_test(bidib_state_tests bidib_state_tests)

# Physical test

add_executable(testsuite test test/physical/main.c test/physical/testsuite.c test/physical/testsuite.h)
target_link_libraries(testsuite glib-2.0 pthread yaml bidib_static)

# Code coverage

if (CMAKE_BUILD_TYPE MATCHES "Debug")
	include(CodeCoverage)
	APPEND_COVERAGE_COMPILER_FLAGS()
	set(COVERAGE_EXCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/test/*)
	SETUP_TARGET_FOR_COVERAGE(
		NAME coverage_test
		EXECUTABLE ctest
		DEPENDENCIES bidib_send_tests bidib_receive_tests
		             bidib_lowlevel_message_tests bidib_config_parser_tests
		             bidib_highlevel_message_tests bidib_state_tests
		)
	unset(CMAKE_BUILD_TYPE CACHE)
endif (CMAKE_BUILD_TYPE MATCHES "Debug")
