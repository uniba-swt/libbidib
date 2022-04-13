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
#include <string.h>

#include "testsuite.h"


#define SIGNAL_WAITING_TIME 3	   // in seconds
#define POINT_WAITING_TIME	3	   // in seconds
#define TRAIN_WAITING_TIME	250000 // in microseconds

t_bidib_id_list_query points;
t_bidib_id_list_query signals;
t_bidib_unified_accessory_state_query state;
t_bidib_train_position_query trainPosition;

char * trainName = NULL;


void testsuite_setTrainName(char * name) {
	trainName = name;
}

// This initialisation function is specific to SWTbahn Standard!
t_testsuite_test_result * testsuite_initTestSuite() {
	points = bidib_get_connected_points();

	// Accessories that are not signals
	t_testsuite_ids filterOutIds;
	char * excludedSignalAccessories[1] = {"platformlights"};
	filterOutIds.ids = excludedSignalAccessories;
	filterOutIds.length = 1;
	signals = testsuite_filterOutIds(bidib_get_connected_signals(), filterOutIds);

	t_testsuite_test_result * result = malloc(sizeof(t_testsuite_test_result));
	result -> points = malloc(points.length * sizeof(t_testsuite_point_result));

	for (size_t i = 0; i < points.length; i++) {
		result -> points[i].stateReachedVerified = 0;
		result -> points[i].stateReached = 0;
		result -> points[i].stateNotReachedVerified = 0;
		result -> points[i].stateNotReached = 0;
		result -> points[i].stateError = 0;
		result -> points[i].unknownState = 0;
	}
	return result;
}

void testsuite_stopBidib(void) {
	bidib_free_id_list_query(points);
	bidib_free_id_list_query(signals);
	bidib_free_unified_accessory_state_query(state);
	bidib_free_train_position_query(trainPosition);
	if (trainName != NULL) {
		bidib_set_train_speed(trainName, 0, "master");
	}
	bidib_stop();
}

void testsuite_signal_callback_handler(int signum) {
	testsuite_stopBidib();
	printf("testsuite: SIGINT - stopping libbidib \n");
	exit(signum);
}

t_bidib_id_list_query testsuite_filterOutIds(t_bidib_id_list_query inputIdQuery, t_testsuite_ids filterOutIds) {
	const size_t count = inputIdQuery.length - filterOutIds.length;

	if (count <= 0) {
		printf("testsuite: No IDs will be left after filtering\n");
	}

	t_bidib_id_list_query outputIdQuery;
	outputIdQuery.length = 0;
	outputIdQuery.ids = malloc(sizeof(char * ) * count);

	int isFilteredOut = 0;

	for (size_t i = 0; i < inputIdQuery.length; i++) {
		isFilteredOut = 0;
		for (size_t j = 0; j < filterOutIds.length; j++) {
			if (!strcmp(inputIdQuery.ids[i], filterOutIds.ids[j])) {
				free(inputIdQuery.ids[i]);
				isFilteredOut = 1;
			}
		}

		if (!isFilteredOut) {
			outputIdQuery.ids[outputIdQuery.length] = inputIdQuery.ids[i];
			outputIdQuery.length++;
		}
	}

	if (outputIdQuery.length != count) {
		printf("testsuite: Error: %zu IDs were to be filtered, but %d IDs filtered instead\n", filterOutIds.length, (int)inputIdQuery.length - (int)outputIdQuery.length);
	}

	return outputIdQuery;
}

void testsuite_logTestResult(t_testsuite_test_result * result, t_bidib_unified_accessory_state_query state, int accessory_index) {
	if (state.known) {
		switch (state.board_accessory_state.execution_state) {
			case BIDIB_EXEX_STATE_ERROR:
				result -> points[accessory_index].stateError++;
				break;
			case BIDIB_EXEC_STATE_NOTREACHED:
				result -> points[accessory_index].stateNotReached++;
				break;
			case BIDIB_EXEC_STATE_NOTREACHED_VERIFIED:
				result -> points[accessory_index].stateNotReachedVerified++;
				break;
			case BIDIB_EXEC_STATE_REACHED:
				result -> points[accessory_index].stateReached++;
				break;
			case BIDIB_EXEC_STATE_REACHED_VERIFIED:
				result -> points[accessory_index].stateReachedVerified++;
				break;
			default:
				break;
		}
	} else {
		result -> points[accessory_index].unknownState++;
	}
}

void testsuite_printTestResults(t_testsuite_test_result * result) {
	for (size_t i = 0; i < points.length; i++) {
		printf("\n\n%s\n", points.ids[i]);
		printf("  -> stateReachedVerified: %d \n", result -> points[i].stateReachedVerified);
		printf("  -> stateReached: %d \n", result -> points[i].stateReached);
		printf("  -> stateNotReachedVerified: %d \n", result -> points[i].stateNotReachedVerified);
		printf("  -> stateNotReached: %d \n", result -> points[i].stateNotReached);
		printf("  -> stateError: %d \n", result -> points[i].stateError);
		printf("  -> unknownState: %d \n", result -> points[i].unknownState);
	}
}

bool testsuite_trainReady(char * train) {
	if (bidib_get_train_on_track(train)) {
		if (strcmp("seg1", bidib_get_train_position(train).segments[0])) {
			printf("testsuite: %s train not on track segment 1.\n", train);
			return false;
		}
		printf("testsuite: %s train ready.\n", train);
		return true;

	} else {
		printf("testsuite: %s train not on track.\n", train);
		return false;
	}
}

void testsuite_driveTo(char * segment, int speed, char * train) {
	bidib_set_train_speed(train, speed, "master");

	while (1) {
		trainPosition = bidib_get_train_position(train);

		for (size_t i = 0; i < trainPosition.length; i++) {
			if (!strcmp(segment, trainPosition.segments[i])) {
				return;
			}
		}

		usleep(TRAIN_WAITING_TIME);
	}
}

void testsuite_driveToStop(char * segment, int speed, char * train) {
	testsuite_driveTo(segment, speed, train);
	bidib_set_train_speed(train, 0, "master");
}

void testsuite_case_signal() {
	for (size_t i = 0; i < signals.length; i++) {
		bidib_set_signal(signals.ids[i], "aspect_caution");
	}
	sleep(SIGNAL_WAITING_TIME);

	for (size_t i = 0; i < signals.length; i++) {
		bidib_set_signal(signals.ids[i], "aspect_go");
	}
	sleep(SIGNAL_WAITING_TIME);

	for (size_t i = 0; i < signals.length; i++) {
		bidib_set_signal(signals.ids[i], "aspect_stop");
	}
	sleep(SIGNAL_WAITING_TIME);

}

void testsuite_case_pointParallel(t_testsuite_test_result * result) {
	for (size_t i = 0; i < points.length; i++) {
		bidib_switch_point(points.ids[i], "reverse");
		state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
	}

	sleep(POINT_WAITING_TIME);

	for (size_t i = 0; i < points.length; i++) {
		bidib_switch_point(points.ids[i], "normal");
		state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
	}

	sleep(POINT_WAITING_TIME);
}

void testsuite_case_pointSerial(t_testsuite_test_result * result) {
	for (size_t i = 0; i < points.length; i++) {
		bidib_switch_point(points.ids[i], "reverse");
		state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		sleep(POINT_WAITING_TIME);

		bidib_switch_point(points.ids[i], "normal");
		state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		sleep(POINT_WAITING_TIME);
	}

}

void testsuite_case_swtbahnStandardTrackCoverage(char * train) {
	if (!testsuite_trainReady(train)) {
		return;
	}

	bidib_switch_point("point1", "normal");

	testsuite_driveTo("seg4", 20, train);

	bidib_switch_point("point2", "normal");
	bidib_switch_point("point3", "normal");
	bidib_switch_point("point4", "normal");

	testsuite_driveTo("seg12", 50, train);

	bidib_switch_point("point6", "reverse");
	bidib_switch_point("point8", "reverse");
	bidib_switch_point("point2", "reverse");
	bidib_switch_point("point3", "reverse");
	bidib_switch_point("point4", "reverse");

	testsuite_driveTo("seg23", 30, train);

	bidib_switch_point("point5", "reverse");
	bidib_switch_point("point9", "reverse");
	bidib_switch_point("point10", "reverse");
	bidib_switch_point("point11", "reverse");
	bidib_switch_point("point12", "normal");

	testsuite_driveToStop("seg37", 30, train);

	bidib_switch_point("point12", "reverse");

	testsuite_driveToStop("seg40", -30, train);

	bidib_switch_point("point12", "normal");
	bidib_switch_point("point9", "normal");
	bidib_switch_point("point10", "normal");
	bidib_switch_point("point11", "normal");

	testsuite_driveToStop("seg28", 30, train);

	bidib_switch_point("point3", "normal");
	bidib_switch_point("point4", "normal");

	testsuite_driveTo("seg21", 30, train);

	bidib_switch_point("point5", "normal");
	bidib_switch_point("point7", "normal");
	bidib_switch_point("point6", "normal");

	testsuite_driveToStop("seg28", 30, train);

	bidib_switch_point("point7", "reverse");
	bidib_switch_point("point6", "normal");
	bidib_switch_point("point8", "normal");
	bidib_switch_point("point1", "reverse");

	testsuite_driveToStop("seg4", 50, train);

	bidib_switch_point("point1", "normal");
	bidib_switch_point("point7", "normal");

	testsuite_driveToStop("seg1", -20, train);
}
