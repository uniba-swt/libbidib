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
	t_testsuite_test_result *result = testsuite_initTestSuite_common(excludedSignalAccessories, 1);
	return result;
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

	testsuite_switch_point("point1", "normal");
	testsuite_switch_point("point2", "normal");
	testsuite_switch_point("point3", "normal");

	testsuite_driveTo("seg12", 80, train);

	testsuite_switch_point("point6", "reverse");
	testsuite_switch_point("point8", "reverse");
	testsuite_switch_point("point2", "reverse");
	testsuite_switch_point("point3", "reverse");
	testsuite_switch_point("point4", "reverse");
	testsuite_switch_point("point5", "reverse");
	testsuite_switch_point("point12", "normal");
	testsuite_switch_point("point10", "reverse");
	testsuite_switch_point("point9", "reverse");
	testsuite_switch_point("point11", "reverse");
	
	testsuite_driveToStop("seg37", 80, train);
	
	testsuite_switch_point("point12", "reverse");

	testsuite_driveToStop("seg40", -80, train);

	testsuite_switch_point("point12", "normal");
	testsuite_switch_point("point11", "normal");
	testsuite_switch_point("point10", "normal");

	testsuite_driveTo("seg28", 50, train);

	testsuite_switch_point("point7", "normal");
	testsuite_switch_point("point4", "normal");
	testsuite_switch_point("point9", "normal");

	testsuite_driveTo("seg21", 80, train);

	testsuite_switch_point("point5", "normal");

	testsuite_driveTo("seg28", 80, train);

	testsuite_switch_point("point7", "reverse");
	testsuite_switch_point("point8", "normal");
	testsuite_switch_point("point2", "reverse");
	testsuite_switch_point("point3", "normal");
	testsuite_switch_point("point6", "normal");
	testsuite_switch_point("point1", "reverse");

	testsuite_driveToStop("seg4", 80, train);

	testsuite_switch_point("point1", "normal");

	testsuite_driveTo("seg1", -20, train);
	sleep(1);
	testsuite_driveToStop("seg1", 0, train);	
}
