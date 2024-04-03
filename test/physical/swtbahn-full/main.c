/*
 *
 * Copyright (C) 2022 University of Bamberg, Software Technologies Research Group
 * <https://www.uni-bamberg.de/>, <http://www.swt-bamberg.de/>
 *
 * This file is part of the BiDiB library (libbidib), used to communicate with
 * BiDiB <www.bidib.org> systems over a serial connection. This library was
 * developed as part of Nicolas Gross’ student project.
 *
 * libbidib is licensed under the GNU GENERAL PUBLIC LICENSE (Version 3), see
 * the LICENSE file at the project's top-level directory for details or consult
 * <http://www.gnu.org/licenses/>.
 *
 * libbidib is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or any later version.
 *
 * libbidib is a RESEARCH PROTOTYPE and distributed WITHOUT ANY WARRANTY, without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 *
 * The following people contributed to the conception and realization of the
 * present libbidib (in alphabetic order by surname):
 *
 * - Christof Lehanka <https://github.com/clehanka>
 * - Bernhard Luedtke <https://github.com/BLuedtke>
 * - Eugene Yip <https://github.com/eyip002>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "testsuite.h"
#include "kinderuni_suite.h"


int argumentsValid(int argc, char **argv);
void printWelcome();
void kinderuni_s1(const char* train1);
void kinderuni_s2_3_4(const char* train1, const char* train2, int scenario);
bool wait_for_console_input();

int main(int argc, char **argv) {
	printWelcome();

	signal(SIGINT, testsuite_signal_callback_handler);

	if (!argumentsValid(argc, argv)) {
		printf("Usage: testsuite <testCaseNumber> <repetitions> [trainName] [trainName] \n");
		printf("  Test cases: \n");
		printf("  1 - Points (parallel switching) \n");
		printf("  2 - Points (serial switching) \n");
		printf("  3 - Signals \n");
		printf("  4 - Track coverage with one train (specify a trainName) \n");
		printf("  5 - Track coverage with two trains (specify two trainNames) \n");
		printf("  6 - Drive short test route with one train (specify a trainName)\n");
		printf("  7 - Drive kinder uni scenario 1 (specify 1 trainName)\n");
		printf("  8 - Drive kinder uni scenario 2 (specify 2 trainNames)\n");
		printf("  9 - Drive kinder uni scenario 3 (specify 2 trainNames)\n");
		printf("  10 - Drive kinder uni scenario 4 (specify 2 trainNames)\n");
		printf("\n");

		return 0;
	}

	//if (bidib_start_serial("/dev/tty.usbserial-AK06U8H7", "../../swtbahn-cli/configurations/swtbahn-full/", 0)) {
	if (bidib_start_serial("/dev/ttyUSB0", "../../swtbahn-cli/configurations/swtbahn-full", 0)) {
		printf("testsuite: libbidib failed to start\n");
		return 0;
	} else {
		printf("testsuite: libbidib started\n");
	}
	sleep(2);	// Wait for the points to finish switching to their default positions.

	printf("testsuite: Test case %d\n", atoi(argv[1]));
	t_testsuite_test_result *result = testsuite_initTestSuite();

	const int repetitions = atoi(argv[2]);
	switch (atoi(argv[1])) {
		case 1:
			bidib_set_track_output_state_all(BIDIB_CS_OFF);
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_pointParallel(result);
			}
			testsuite_printTestResults(result);
			break;

		case 2:
			bidib_set_track_output_state_all(BIDIB_CS_OFF);
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_pointSerial(result);
			}
			testsuite_printTestResults(result);
			break;

		case 3:
			bidib_set_track_output_state_all(BIDIB_CS_OFF);
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_signal();
			}
			break;

		case 4:
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_swtbahnFullTrackCoverage(argv[3]);
			}
			break;

		case 5:
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_swtbahnFullMultipleTrains(argv[3], argv[4]);
			}
			break;
		case 6:
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_swtbahnFullShortRoute(argv[3]);
			}
			break;
		case 7:
			kinderuni_s1(argv[3]);
			break;
		case 8:
			kinderuni_s2_3_4(argv[3], argv[4], 2);
			break;
		case 9:
			kinderuni_s2_3_4(argv[3], argv[4], 3);
			break;
		case 10:
			kinderuni_s2_3_4(argv[3], argv[4], 4);
			break;
		default:
			break;
	}

	testsuite_stopBidib();
	free(result->points);
	free(result);

	return 0;
}

int argumentsValid(int argc, char **argv) {
	if (argc < 2) {
		return 0;
	}
	
	const int testCaseNumber = atoi(argv[1]);
	switch (testCaseNumber) {
		case 1:
		case 2:
		case 3:
			if (argc != 3) {
				return 0;
			}
			break;

		case 4:
			if (argc != 4) {
				return 0;
			}
			break;

		case 5:
			if (argc != 5) {
				return 0;
			}
			break;
		case 6:
			if (argc != 4) {
				return 0;
			}
			break;
		case 7:
			if (argc != 4) {
				return 0;
			}
			break;
		case 8:
		case 9:
		case 10:
			if (argc != 5) {
				return 0;
			}
			break;
		default:
			return 0;
	}

	return 1;
}

void printWelcome() {
	char *message[8] = {
		"************************",
		"*                      *",
		"*   SWTbahn-testsuite  *",
		"*                      *",
		"************************",
		"*    UniBa-SWT-2023    *",
		"************************",
		""
	};

	for (size_t i = 0; i < 8; i++) {
		printf("%s\n", message[i]);
	}
}

inline bool wait_for_console_input() {
	char *line = NULL;
    size_t len = 0;
    ssize_t read = 0;
	read = getline(&line, &len, stdin);
	free(line);
	return read >= 0;
}

inline void kinderuni_s1(const char* train1) {
	printf("Starting Kinder Uni Scenario 1.\n");
	if (train1 == NULL) {
		printf("Kinder Uni Scenario 1: Aborted, train1 is NULL.");
		return;
	}
	printf("Please ensure %s is on seg22.\n", train1);
	printf("Press enter once you want to start initialising.\n");
	if (!wait_for_console_input()) {
		printf("Aborting, faulty console read.\n");
		return;
	}
	bool init_result = ku_scenario1_initial(train1);
	if (!init_result) {
		printf("Aborting, init failed.\n");
		return;
	}
	
	printf("Press enter once you want to start the actions.\n");
	if (!wait_for_console_input()) {
		printf("Aborting, faulty console read.\n");
		return;
	}
	bool action_result = ku_scenario1_aktion(train1);
	if (!action_result) {
		printf("Aborting, action failed.\n");
		return;
	}
	
	printf("Press enter once you want to start the reset.\n");
	if (!wait_for_console_input()) {
		printf("Aborting, faulty console read.\n");
		return;
	}
	bool reset_result = ku_scenario1_reset(train1);
	if (!reset_result) {
		printf("Aborting, reset failed.\n");
		return;
	}
	printf("Scenario 1 completed.\n");
}

inline void kinderuni_s2_3_4(const char* train1, const char* train2, int scenario) {
	printf("Starting Kinder Uni Scenario %d.\n", scenario);
	if (train1 == NULL || train2 == NULL) {
		printf("Aborting, train1 or train2 is NULL.");
		return;
	}
	printf("Press enter once you want to start initialising.\n");
	if (!wait_for_console_input()) {
		printf("Aborting, faulty console read.\n");
		return;
	}
	bool init_result = false;
	switch (scenario) {
		case 2: init_result = ku_scenario2_initial(train1, train2); break;
		case 3: init_result = ku_scenario3_initial(train1, train2); break;
		case 4: init_result = ku_scenario4_initial(train1, train2); break;
		default:
			break;
	}
	if (!init_result) {
		printf("Aborting, init failed.\n");
		return;
	}
	
	printf("Press enter once you want to start the actions.\n");
	if (!wait_for_console_input()) {
		printf("Aborting, faulty console read.\n");
		return;
	}
	bool action_result = false;
	switch (scenario) {
		case 2: action_result = ku_scenario2_aktion(train1, train2); break;
		case 3: action_result = ku_scenario3_aktion(train1, train2); break;
		case 4: action_result = ku_scenario4_aktion(train1, train2); break;
		default:
			break;
	}
	if (!action_result) {
		printf("Aborting, action failed.\n");
		return;
	}
	
	printf("Press enter once you want to start the reset.\n");
	if (!wait_for_console_input()) {
		printf("Aborting, faulty console read.\n");
		return;
	}
	bool reset_result = false;
	switch (scenario) {
		case 2: reset_result = ku_scenario2_reset(train1, train2); break;
		case 3: reset_result = ku_scenario3_reset(train1, train2); break;
		case 4: reset_result = ku_scenario4_reset(train1, train2); break;
		default:
			break;
	}
	if (!reset_result) {
		printf("Aborting, reset failed.\n");
		return;
	}
	printf("Scenario %d completed.\n", scenario);
}
