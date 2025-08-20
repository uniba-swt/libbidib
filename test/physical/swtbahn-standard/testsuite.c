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
 * - Bernhard Luedtke <https://github.com/BLuedtke>
 * - Eugene Yip <https://github.com/eyip002>
 *
 */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "testsuite.h"

// This initialisation function is specific to SWTbahn Standard.
t_testsuite_test_result *testsuite_initTestSuite() {
	char *excludedSignalAccessories[1] = {"platformlights"};
	return testsuite_initTestSuite_common(excludedSignalAccessories, 1);
}

void testsuite_case_signal() {
	char *signalAspects[3] = {"aspect_caution", "aspect_go", "aspect_stop"};
	testsuite_case_signal_common(signalAspects, 3);
}

void testsuite_case_pointParallel(t_testsuite_test_result *result) {
	testsuite_case_pointParallel_common(result);
}

void testsuite_case_pointSerial(t_testsuite_test_result *result) {
	testsuite_case_pointSerial_common(result);
}

void testsuite_case_reverser(void) {
	const int max_retries = 2;
	
	t_bidib_id_list_query rev_query = bidib_get_connected_reversers();
	for (size_t i = 0; i < rev_query.length; i++) {
		const char *reverser_id = rev_query.ids[i];
		const char *reverser_board = "master";
		int err = bidib_request_reverser_state(reverser_id, reverser_board);
		bidib_flush();
		if (err) {
			continue;
		}
		
		bool state_unknown = true;
		for (int retry = 0; retry < max_retries && state_unknown; retry++) {
			t_bidib_reverser_state_query rev_state_query =
					bidib_get_reverser_state(reverser_id);
			state_unknown = !rev_state_query.available 
			                || rev_state_query.data.state_value == BIDIB_REV_EXEC_STATE_UNKNOWN;
			if (!state_unknown) {
				char *state_value_str = "unknown";
				switch (rev_state_query.data.state_value) {
					case BIDIB_REV_EXEC_STATE_OFF: 
						state_value_str = "off";
						break;
					case BIDIB_REV_EXEC_STATE_ON: 
						state_value_str = "on";
						break;
					default:
						state_value_str = "unknown";
						break;
				}
				printf("Reverser %s is %s\n", reverser_id, state_value_str);
				bidib_free_reverser_state_query(rev_state_query);
				break;
			}
			usleep(50000);   // 0.05s
		}
		
		if (state_unknown) {
			printf("Reverser %s state could not be retrieved\n", reverser_id);
		}
	}

	bidib_free_id_list_query(rev_query);
}

void testsuite_case_swtbahnStandardTrackCoverage(const char *train) {
	if (!testsuite_trainReady(train, "seg1")) {
		return;
	}
	
	const char *points_normal_c1[3] = { "point1", "point2", "point3" };
	if (!testsuite_set_and_check_points(points_normal_c1, 3, NULL, 0)) {
		printf("testsuite: standard track coverage - "
		       "one or more points are not in expected aspect (1st check).\n");
		return;
	}
	
	testsuite_driveToStop("seg12", 30, train);
	
	const char *points_normal_c2[1] = { "point12" };
	const char *points_reverse_c2[9] = { 
		"point6", "point8", "point2", "point3", "point4", "point5", "point10", "point9", "point11"
	};
	if (!testsuite_set_and_check_points(points_normal_c2, 1, points_reverse_c2, 9)) {
		printf("testsuite: standard track coverage - "
		       "one or more points are not in expected aspect (2nd check).\n");
		return;
	}
	
	testsuite_driveToStop("seg37", 30, train);
	
	const char *points_reverse_c3[1] = { "point12" };
	if (!testsuite_set_and_check_points(NULL, 0, points_reverse_c3, 1)) {
		printf("testsuite: standard track coverage - point12 not in expected aspect (3rd check).\n");
		return;
	}
	
	testsuite_driveToStop("seg40", -30, train);
	
	const char *points_normal_c4[3] = { "point12", "point11", "point10" };
	if (!testsuite_set_and_check_points(points_normal_c4, 3, NULL, 0)) {
		printf("testsuite: standard track coverage - "
		       "one or more points are not in expected aspect (4th check).\n");
		return;
	}
	
	testsuite_driveToStop("seg28", 30, train);
	
	const char *points_normal_c5[3] = { "point7", "point4", "point9" };
	if (!testsuite_set_and_check_points(points_normal_c5, 3, NULL, 0)) {
		printf("testsuite: standard track coverage - "
		       "one or more points are not in expected aspect (5th check).\n");
		return;
	}
	
	testsuite_driveToStop("seg21", 30, train);
	
	const char *points_normal_c6[1] = { "point5"};
	if (!testsuite_set_and_check_points(points_normal_c6, 1, NULL, 0)) {
		printf("testsuite: standard track coverage - point5 not in expected aspect (6th check).\n");
		return;
	}
	
	testsuite_driveTo("seg28", 30, train);
	
	const char *points_normal_c7[3] = { "point8", "point3", "point6" };
	const char *points_reverse_c7[3] = { "point7", "point2", "point1" };
	if (!testsuite_set_and_check_points(points_normal_c7, 3, points_reverse_c7, 3)) {
		printf("testsuite: standard track coverage - "
		       "one or more points are not in expected aspect (7th check).\n");
		return;
	}
	
	testsuite_driveToStop("seg4", 30, train);
	
	const char *points_normal_c8[1] = { "point1"};
	if (!testsuite_set_and_check_points(points_normal_c8, 1, NULL, 0)) {
		printf("testsuite: standard track coverage - point1 not in expected aspect (8th check).\n");
		return;
	}
	
	testsuite_driveTo("seg1", -20, train);
	sleep(1);
	testsuite_driveToStop("seg1", 0, train);
}
