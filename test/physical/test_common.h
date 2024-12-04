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


#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include "../../include/bidib.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
	int stateError;
	int stateNotReached;
	int stateNotReachedVerified;
	int stateReached;
	int stateReachedVerified;
	int unknownState;
} t_testsuite_point_result;

typedef struct {
	t_testsuite_point_result *points;
} t_testsuite_test_result;

typedef struct {
	char **ids;
	size_t length;
} t_testsuite_ids;

extern t_bidib_id_list_query points;
extern t_bidib_id_list_query signals;

// Setup
t_testsuite_test_result *testsuite_initTestSuite_common(char **excludedSignalAccessories, size_t excludedSignalAccessories_len);
t_bidib_id_list_query testsuite_filterOutIds(t_bidib_id_list_query inputIdQuery, t_testsuite_ids filterOutIds);

// Teardown
void testsuite_stopBidib(void);
void testsuite_signal_callback_handler(int signum);

// Logging
void testsuite_recordTestResult(t_testsuite_test_result *result, t_bidib_unified_accessory_state_query state, int accessory_index);
void testsuite_printTestResults(t_testsuite_test_result *result);
void testsuite_logAllTrackOutputStates();
void testsuite_logAllBoosterPowerStates();

// Driving
bool testsuite_trainReady(const char *train, const char *segment);
void testsuite_driveTo(const char *segment, int speed, const char *train);
void testsuite_driveToStop(const char *segment, int speed, const char *train);

// Accessories
void testsuite_set_signal(const char *signal, const char *aspect);
void testsuite_switch_point(const char *point, const char *aspect);
// Returns true if the point's aspect matches a certain value/aspect. Otherwise returns false.
bool testsuite_check_point_aspect(const char *point, const char *aspect);

// Common test base
void testsuite_case_signal_common(char **aspects, size_t aspects_len);
void testsuite_case_pointParallel_common(t_testsuite_test_result *result);
void testsuite_case_pointSerial_common(t_testsuite_test_result *result);

#endif
