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


#define SIGNAL_WAITING_TIME 3	   // in seconds
#define POINT_WAITING_TIME	3	   // in seconds
#define TRAIN_WAITING_TIME	250000 // in microseconds

t_bidib_id_list_query points;
t_bidib_id_list_query signals;

char *trainName = NULL;


void testsuite_setTrainName(const char *name) {
	trainName = name;
}

// This initialisation function is specific to SWTbahn Standard!
t_testsuite_test_result *testsuite_initTestSuite() {
	points = bidib_get_connected_points();

	// Accessories that are not signals
	t_testsuite_ids filterOutIds;
	char *excludedSignalAccessories[1] = {"platformlights"};
	filterOutIds.ids = excludedSignalAccessories;
	filterOutIds.length = 1;
	signals = testsuite_filterOutIds(bidib_get_connected_signals(), filterOutIds);

	t_testsuite_test_result *result = malloc(sizeof(t_testsuite_test_result));
	result->points = malloc(points.length * sizeof(t_testsuite_point_result));

	for (size_t i = 0; i < points.length; i++) {
		result->points[i].stateReachedVerified = 0;
		result->points[i].stateReached = 0;
		result->points[i].stateNotReachedVerified = 0;
		result->points[i].stateNotReached = 0;
		result->points[i].stateError = 0;
		result->points[i].unknownState = 0;
	}
	return result;
}

void testsuite_stopBidib(void) {
	bidib_free_id_list_query(points);
	bidib_free_id_list_query(signals);
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
	outputIdQuery.ids = malloc(sizeof(char *) * count);

	int isFilteredOut = 0;

	for (size_t i = 0; i < inputIdQuery.length; i++) {
		isFilteredOut = 0;
		for (size_t j = 0; j < filterOutIds.length; j++) {
			if (!strcmp(inputIdQuery.ids[i], filterOutIds.ids[j])) {
				isFilteredOut = 1;
				break;
			}
		}

		if (!isFilteredOut) {
			outputIdQuery.ids[outputIdQuery.length] = malloc(strlen(inputIdQuery.ids[i]) * sizeof(char) + 1) ;
			memcpy(outputIdQuery.ids[outputIdQuery.length], inputIdQuery.ids[i], strlen(inputIdQuery.ids[i]) * sizeof(char) + 1);
			outputIdQuery.length++;
		}
	}

	if (outputIdQuery.length != count) {
		printf("testsuite: Error: %zu IDs were to be filtered, but %d IDs filtered instead\n", filterOutIds.length, (int)inputIdQuery.length - (int)outputIdQuery.length);
	}

	return outputIdQuery;
}

void testsuite_logTestResult(t_testsuite_test_result *result, t_bidib_unified_accessory_state_query state, int accessory_index) {
	if (state.known) {
		switch (state.board_accessory_state.execution_state) {
			case BIDIB_EXEC_STATE_ERROR:
				result->points[accessory_index].stateError++;
				break;
			case BIDIB_EXEC_STATE_NOTREACHED:
				result->points[accessory_index].stateNotReached++;
				break;
			case BIDIB_EXEC_STATE_NOTREACHED_VERIFIED:
				result->points[accessory_index].stateNotReachedVerified++;
				break;
			case BIDIB_EXEC_STATE_REACHED:
				result->points[accessory_index].stateReached++;
				break;
			case BIDIB_EXEC_STATE_REACHED_VERIFIED:
				result->points[accessory_index].stateReachedVerified++;
				break;
			default:
				break;
		}
	} else {
		result->points[accessory_index].unknownState++;
	}
}

void testsuite_printTestResults(t_testsuite_test_result *result) {
	for (size_t i = 0; i < points.length; i++) {
		printf("\n\n%s\n", points.ids[i]);
		printf("  -> stateReachedVerified: %d \n", result->points[i].stateReachedVerified);
		printf("  -> stateReached: %d \n", result->points[i].stateReached);
		printf("  -> stateNotReachedVerified: %d \n", result->points[i].stateNotReachedVerified);
		printf("  -> stateNotReached: %d \n", result->points[i].stateNotReached);
		printf("  -> stateError: %d \n", result->points[i].stateError);
		printf("  -> unknownState: %d \n", result->points[i].unknownState);
	}
}

bool testsuite_trainReady(const char *train) {
	const char *segment = "seg1";
	if (bidib_get_train_on_track(train)) {
		t_bidib_train_position_query train_position_query = bidib_get_train_position(train);
		if (train_position_query.length > 0) {
			for (size_t i = 0; i < train_position_query.length; i++) {
				if (strcmp(segment, train_position_query.segments[i]) == 0) {
					printf("testsuite: %s train ready on %s \n", train, segment);
					bidib_free_train_position_query(train_position_query);
					return true;
				}
			}
		}
		
		printf("testsuite: %s train not on track segment %s \n", train, segment);
		bidib_free_train_position_query(train_position_query);
		return false;
	} else {
		printf("testsuite: %s train not detected on any track \n", train);
		return false;
	}
}

void testsuite_driveTo(const char *segment, int speed, const char *train) {
	bidib_set_train_speed(train, speed, "master");
	bidib_flush();

	while (1) {
		t_bidib_train_position_query trainPosition = bidib_get_train_position(train);

		for (size_t i = 0; i < trainPosition.length; i++) {
			if (!strcmp(segment, trainPosition.segments[i])) {
				bidib_free_train_position_query(trainPosition);
				return;
			}
		}
		bidib_free_train_position_query(trainPosition);
		usleep(TRAIN_WAITING_TIME);
	}
}

void testsuite_driveToStop(const char *segment, int speed, const char *train) {
	testsuite_driveTo(segment, speed, train);
	bidib_set_train_speed(train, 0, "master");
	bidib_flush();
}

void set_signal(const char *signal, const char *aspect) {
	bidib_set_signal(signal, aspect);
	bidib_flush();
}

void switch_point(const char *point, const char *aspect) {
	bidib_switch_point(point, aspect);
	bidib_flush();
}

void testsuite_case_signal() {
	for (size_t i = 0; i < signals.length; i++) {
		set_signal(signals.ids[i], "aspect_caution");
	}
	sleep(SIGNAL_WAITING_TIME);

	for (size_t i = 0; i < signals.length; i++) {
		set_signal(signals.ids[i], "aspect_go");
	}
	sleep(SIGNAL_WAITING_TIME);

	for (size_t i = 0; i < signals.length; i++) {
		set_signal(signals.ids[i], "aspect_stop");
	}
	sleep(SIGNAL_WAITING_TIME);

}

void testsuite_case_pointParallel(t_testsuite_test_result *result) {
	for (size_t i = 0; i < points.length; i++) {
		switch_point(points.ids[i], "reverse");
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
	}

	sleep(POINT_WAITING_TIME);

	for (size_t i = 0; i < points.length; i++) {
		switch_point(points.ids[i], "normal");
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
	}

	sleep(POINT_WAITING_TIME);
}

void testsuite_case_pointSerial(t_testsuite_test_result *result) {
	for (size_t i = 0; i < points.length; i++) {
		switch_point(points.ids[i], "reverse");
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		sleep(POINT_WAITING_TIME);
		bidib_free_unified_accessory_state_query(state);
		switch_point(points.ids[i], "normal");
		state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
		sleep(POINT_WAITING_TIME);
	}
}

void testsuite_case_swtbahnStandardTrackCoverage(const char *train) {
	if (!testsuite_trainReady(train)) {
		return;
	}

	switch_point("point1", "normal");
	switch_point("point2", "normal");
	switch_point("point3", "normal");

	testsuite_driveTo("seg12", 80, train);

	switch_point("point6", "reverse");
	switch_point("point8", "reverse");
	switch_point("point2", "reverse");
	switch_point("point3", "reverse");
	switch_point("point4", "reverse");
	switch_point("point5", "reverse");
	switch_point("point12", "normal");
	switch_point("point10", "reverse");
	switch_point("point9", "reverse");
	switch_point("point11", "reverse");
	
	testsuite_driveToStop("seg37", 80, train);
	
	switch_point("point12", "reverse");

	testsuite_driveToStop("seg40", -80, train);

	switch_point("point12", "normal");
	switch_point("point11", "normal");
	switch_point("point10", "normal");

	testsuite_driveTo("seg28", 50, train);

	switch_point("point7", "normal");
	switch_point("point4", "normal");
	switch_point("point9", "normal");

	testsuite_driveTo("seg21", 80, train);

	switch_point("point5", "normal");

	testsuite_driveTo("seg28", 80, train);

	switch_point("point7", "reverse");
	switch_point("point8", "normal");
	switch_point("point2", "reverse");
	switch_point("point3", "normal");
	switch_point("point6", "normal");
	switch_point("point1", "reverse");

	testsuite_driveToStop("seg4", 80, train);

	switch_point("point1", "normal");

	testsuite_driveTo("seg1", -20, train);
	sleep(1);
	testsuite_driveToStop("seg1", 0, train);	
}
