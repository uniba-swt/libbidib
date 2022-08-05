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
 * - Eugene Yip <https://github.com/eyip002>
 *
 */

#ifndef TESTSUITE_H
#define TESTSUITE_H


#include "../../../include/bidib.h"


typedef struct {
	int stateError;
	int stateNotReached;
	int stateNotReachedVerified;
	int stateReached;
	int stateReachedVerified;
	int unknownState;
}
t_testsuite_point_result;

typedef struct {
	t_testsuite_point_result * points;
}
t_testsuite_test_result;

typedef struct {
	char ** ids;
	size_t length;
}
t_testsuite_ids;


// Setup
t_testsuite_test_result * testsuite_initTestSuite();
t_bidib_id_list_query testsuite_filterOutIds(t_bidib_id_list_query inputIdQuery, t_testsuite_ids filterOutIds);

// Teardown
void testsuite_stopBidib(void);
void testsuite_signal_callback_handler(int signum);

// Logging
void testsuite_logTestResult(t_testsuite_test_result * result, t_bidib_unified_accessory_state_query state, int accessory_index);
void testsuite_printTestResults(t_testsuite_test_result * result);

// Driving
bool testsuite_trainReady(char * train, char * segment);
void testsuite_driveTo(char * segment, int speed, char * train);
void testsuite_driveToStop(char * segment, int speed, char * train);

// Test cases
void testsuite_case_signal();
void testsuite_case_pointParallel(t_testsuite_test_result * result);
void testsuite_case_pointSerial(t_testsuite_test_result * result);
void testsuite_case_swtbahnFullTrackCoverage(char * train);
void testsuite_case_swtbahnFullMultipleTrains(char * train1, char * train2);
void testsuite_case_various_performance();

#endif
