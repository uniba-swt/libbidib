cmake_minimum_required(VERSION 3.6)
project(libbidib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include(FindPkgConfig)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
include_directories(${GLIB_INCLUDE_DIRS})

file(GLOB SRCFILES "src/*/*.c")


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

set(UNIT_TESTS bidib_send_tests bidib_receive_tests
               bidib_lowlevel_message_tests bidib_config_parser_tests
               bidib_highlevel_message_tests bidib_state_tests)

foreach(UNIT_TEST ${UNIT_TESTS})
	add_executable(${UNIT_TEST} test test/unit/${UNIT_TEST}.c)
	target_link_libraries(${UNIT_TEST} glib-2.0 cmocka pthread yaml bidib_static)
	add_test(${UNIT_TEST} ${UNIT_TEST})
endforeach()


# Code coverage

if (CMAKE_BUILD_TYPE MATCHES "Debug")
	set(COVERAGE_TEST_DIR coverage_test)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")

	add_custom_target(coverage_test
		COMMAND llvm-profdata merge ${COVERAGE_TEST_DIR}/*.profraw 
			    -o ${COVERAGE_TEST_DIR}/coverage_test.profdata
		COMMAND llvm-cov show -format=html -output-dir=${COVERAGE_TEST_DIR} ${UNIT_TESTS}
			    -instr-profile=${COVERAGE_TEST_DIR}/coverage_test.profdata -show-line-counts-or-regions
		DEPENDS ${UNIT_TESTS}
	)

	foreach(UNIT_TEST ${UNIT_TESTS})
		add_custom_target(${UNIT_TEST}_coverage_prepare
			COMMAND LLVM_PROFILE_FILE=${COVERAGE_TEST_DIR}/${UNIT_TEST}.profraw $<TARGET_FILE:${UNIT_TEST}>
			DEPENDS ${UNIT_TESTS}
		)
		add_dependencies(coverage_test ${UNIT_TEST}_coverage_prepare)
	endforeach()

	unset(CMAKE_BUILD_TYPE CACHE)
endif (CMAKE_BUILD_TYPE MATCHES "Debug")


# Physical test

add_executable(swtbahn-standard-testsuite test test/physical/swtbahn-standard/main.c test/physical/swtbahn-standard/testsuite.c)
target_link_libraries(swtbahn-standard-testsuite glib-2.0 pthread yaml bidib_static)

add_executable(swtbahn-full-testsuite test test/physical/swtbahn-full/main.c test/physical/swtbahn-full/testsuite.c)
target_link_libraries(swtbahn-full-testsuite glib-2.0 pthread yaml bidib_static)
