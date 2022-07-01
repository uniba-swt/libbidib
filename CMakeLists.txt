cmake_minimum_required(VERSION 3.6)
project(libbidib)

SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_FLAGS "-Wall -Wextra")
SET(CMAKE_EXPORT_COMPILE_COMMANDS on)

SET(BIDIB_USE_TESTS true CACHE BOOL "Compile the tests")

SET(CMAKE_INSTALL_INCLUDEDIR include/bidib)


INCLUDE(FindPkgConfig)
pkg_check_modules(GLIB glib-2.0 REQUIRED)
INCLUDE_DIRECTORIES(${GLIB_INCLUDE_DIRS})


# SRC files
FILE(GLOB SRCFILES "src/*/*.c")

# - - - - - - - - - - - - -
# INCLUDE FILES
# - - - - - - - - - - - - -

# Collecting the headers for each folder, needed for the include targets later
FILE(GLOB INCLUDES_MAIN "include/*.h")
FILE(GLOB INCLUDES_DEFINITIONS "include/definitions/*.h")
FILE(GLOB INCLUDES_HIGHLEVEL "include/highlevel/*.h")
FILE(GLOB INCLUDES_LOWLEVEL "include/lowlevel/*.h")


# - - - - - - - - - - - - -
# LIBRARY DEFINITION
# - - - - - - - - - - - - -

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)

ADD_LIBRARY(bidib_static STATIC include src ${SRCFILES})
ADD_LIBRARY(bidib SHARED include src ${SRCFILES})



# - - - - - - - - - - - - -
# INSTALL DEFINITION
# - - - - - - - - - - - - -

INCLUDE(GNUInstallDirs)

# Configuring and install target pkgconfig files
CONFIGURE_FILE("bidib.pc.in" "bidib.pc" @ONLY)
CONFIGURE_FILE("bidib_static.pc.in" "bidib_static.pc" @ONLY)
INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/bidib.pc" "${CMAKE_CURRENT_BINARY_DIR}/bidib_static.pc"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# Install targets for headers
INSTALL(FILES ${INCLUDES_MAIN} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
INSTALL(FILES ${INCLUDES_DEFINITIONS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/definitions)
INSTALL(FILES ${INCLUDES_HIGHLEVEL} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/highlevel)
INSTALL(FILES ${INCLUDES_LOWLEVEL} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/lowlevel)
# Install Target for libs
INSTALL(TARGETS bidib bidib_static)





# - - - - - - - - - - - - -
# TESTS DEFINITION
# - - - - - - - - - - - - -


IF (${BIDIB_USE_TESTS})
	ENABLE_TESTING()
	# Unit tests

	SET(UNIT_TESTS bidib_send_tests bidib_receive_tests
	               bidib_lowlevel_message_tests bidib_config_parser_tests
	               bidib_highlevel_message_tests bidib_state_tests)

	FOREACH(UNIT_TEST ${UNIT_TESTS})
		ADD_EXECUTABLE(${UNIT_TEST} test test/unit/${UNIT_TEST}.c)
		TARGET_LINK_LIBRARIES(${UNIT_TEST} glib-2.0 cmocka pthread yaml bidib_static)
		ADD_TEST(${UNIT_TEST} ${UNIT_TEST})
	ENDFOREACH()


	# Code coverage

	IF (CMAKE_BUILD_TYPE MATCHES "Debug")
		SET(COVERAGE_TEST_DIR coverage_test)
		SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-instr-generate -fcoverage-mapping")

		ADD_CUSTOM_TARGET(coverage_test
			COMMAND llvm-profdata merge ${COVERAGE_TEST_DIR}/*.profraw 
				    -o ${COVERAGE_TEST_DIR}/coverage_test.profdata
			COMMAND llvm-cov show -format=html -output-dir=${COVERAGE_TEST_DIR} ${UNIT_TESTS}
				    -instr-profile=${COVERAGE_TEST_DIR}/coverage_test.profdata -show-line-counts-or-regions
			DEPENDS ${UNIT_TESTS}
		)

		FOREACH(UNIT_TEST ${UNIT_TESTS})
			ADD_CUSTOM_TARGET(${UNIT_TEST}_coverage_prepare
				COMMAND LLVM_PROFILE_FILE=${COVERAGE_TEST_DIR}/${UNIT_TEST}.profraw $<TARGET_FILE:${UNIT_TEST}>
				DEPENDS ${UNIT_TESTS}
			)
			ADD_DEPENDENCIES(coverage_test ${UNIT_TEST}_coverage_prepare)
		ENDFOREACH()

		UNSET(CMAKE_BUILD_TYPE CACHE)
	ENDIF (CMAKE_BUILD_TYPE MATCHES "Debug")


	# Physical test

	ADD_EXECUTABLE(swtbahn-standard-testsuite test test/physical/swtbahn-standard/main.c test/physical/swtbahn-standard/testsuite.c)
	TARGET_LINK_LIBRARIES(swtbahn-standard-testsuite glib-2.0 pthread yaml bidib_static)

	ADD_EXECUTABLE(swtbahn-full-testsuite test test/physical/swtbahn-full/main.c test/physical/swtbahn-full/testsuite.c)
	TARGET_LINK_LIBRARIES(swtbahn-full-testsuite glib-2.0 pthread yaml bidib_static)

ENDIF(${BIDIB_USE_TESTS})
