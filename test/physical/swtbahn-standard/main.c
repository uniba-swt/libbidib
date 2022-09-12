/*
 *
 * Copyright (C) 2020 University of Bamberg, Software Technologies Research Group
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
 * - Eugene Yip <https://github.com/eyip002>
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "testsuite.h"


int argumentsValid(int argc, char **argv);
void printWelcome();


int main(int argc, char **argv) {
	printWelcome();

	signal(SIGINT, testsuite_signal_callback_handler);

	if (!argumentsValid(argc, argv)) {
		printf("Usage: testsuite <testCaseNumber> <repetitions> [trainName] \n");
		printf("  Test cases: \n");
		printf("  1 - Points (parallel switching) \n");
		printf("  2 - Points (serial switching) \n");
		printf("  3 - Signals \n");
		printf("  4 - Track coverage with one train (specified a trainName) \n");
		printf("  5 - Reverser (runs continuoslz) \n");
		printf("\n");

		return 0;
	}

	if (bidib_start_serial("/dev/ttyUSB0", "../../swtbahn-cli/configurations/swtbahn-standard", 200)) {
		printf("testsuite: libbidib failed to start\n");
		return 0;
	}

	printf("testsuite: Test case %d\n", atoi(argv[1]));
	t_testsuite_test_result *result = testsuite_initTestSuite();

	const int repetitions = atoi(argv[2]);
	switch (atoi(argv[1])) {
		case 1:
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_pointParallel(result);
			}
			testsuite_printTestResults(result);
			break;
		case 2:
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_pointSerial(result);
			}
			testsuite_printTestResults(result);
			break;
		case 3:
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_signal();
			}
			break;
		case 4:
			for (int i = 0; i < repetitions; i++) {
				testsuite_case_swtbahnStandardTrackCoverage(argv[3]);
			}
			break;
		case 5:
			while (true) {
				testsuite_case_reverser();
				sleep(1);
			}
			break;
		default:
			break;
	}

	testsuite_stopBidib();
	free(result);

	return 0;
}

int argumentsValid(int argc, char **argv) {
	if (argc < 3) {
		return 0;
	} else if ((argc > 3) && !(atoi(argv[1]) == 4)) {
		return 0;
	} else if ((argc != 4) && (atoi(argv[1]) == 4)) {
		return 0;
	} else if (!(atoi(argv[1]) < 6)) {
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
		"*    UniBa-SWT-2022    *",
		"************************",
		""
	};

	for (size_t i = 0; i < 8; i++) {
		printf("%s\n", message[i]);
	}
}
