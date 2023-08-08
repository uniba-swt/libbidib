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
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "testsuite.h"


#define SIGNAL_WAITING_TIME 3	   // in seconds
#define POINT_WAITING_TIME	3	   // in seconds
#define TRAIN_WAITING_TIME	250000 // in microseconds

t_bidib_id_list_query points;
t_bidib_id_list_query signals;

// This initialisation function is specific to SWTbahn Full!
t_testsuite_test_result *testsuite_initTestSuite() {
	points = bidib_get_connected_points();

	// Accessories that are not signals
	t_testsuite_ids filterOutIds;
	char *excludedSignalAccessories[4] = {"platformlight1", "platformlight2", "platformlight4a", "platformlight4b"};
	filterOutIds.ids = excludedSignalAccessories;
	filterOutIds.length = 4;
	t_bidib_id_list_query signalsQuery = bidib_get_connected_signals();
	signals = testsuite_filterOutIds(signalsQuery, filterOutIds);
	bidib_free_id_list_query(signalsQuery);
	
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
			size_t len = strlen(inputIdQuery.ids[i]) + 1;
			outputIdQuery.ids[outputIdQuery.length] = malloc(sizeof(char) * len);
			memcpy(outputIdQuery.ids[outputIdQuery.length], inputIdQuery.ids[i], len);
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

bool testsuite_trainReady(const char *train, const char *segment) {
	if (bidib_get_train_on_track(train)) {
		t_bidib_train_position_query train_position_query = 
				bidib_get_train_position(train);
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

	for (size_t i = 0; i < signals.length; i++) {
		set_signal(signals.ids[i], "aspect_shunt");
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
		bidib_free_unified_accessory_state_query(state);
		sleep(POINT_WAITING_TIME);

		switch_point(points.ids[i], "normal");
		state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
		sleep(POINT_WAITING_TIME);
	}

}

bool route1(const char *train) {
	if (!testsuite_trainReady(train, "seg58")) {
		return false;
	}

	switch_point("point22", "reverse");
	switch_point("point23", "normal");
	switch_point("point24", "reverse");
	switch_point("point12", "reverse");
	switch_point("point13", "reverse");
	switch_point("point14", "reverse");
	switch_point("point15", "normal");
	switch_point("point16", "reverse");
	switch_point("point21", "reverse");
	switch_point("point20", "normal");
	switch_point("point19", "normal");
	switch_point("point18b", "reverse");

	set_signal("signal30", "aspect_go");
	set_signal("signal33", "aspect_go");
	set_signal("signal35a", "aspect_go");
	set_signal("signal35b", "aspect_go");
	set_signal("signal37", "aspect_go");

	testsuite_driveTo("seg57", 50, train);
	set_signal("signal30", "aspect_stop");

	testsuite_driveTo("seg64", 50, train);
	set_signal("signal33", "aspect_stop");
	set_signal("signal35a", "aspect_stop");
	set_signal("signal35b", "aspect_stop");

	testsuite_driveTo("seg69", 50, train);
	set_signal("signal37", "aspect_stop");

	testsuite_driveTo("seg46", 50, train);
	testsuite_driveToStop("seg47", 20, train);

	// Drive backwards through the route
	set_signal("signal26", "aspect_go");
	set_signal("signal38", "aspect_go");
	set_signal("signal36", "aspect_go");
	set_signal("signal34", "aspect_go");
	set_signal("signal32", "aspect_go");

	testsuite_driveTo("seg45", -50, train);
	set_signal("signal26", "aspect_stop");

	testsuite_driveTo("seg67", -50, train);
	set_signal("signal38", "aspect_stop");
	set_signal("signal36", "aspect_stop");

	testsuite_driveTo("seg62", -50, train);
	set_signal("signal34", "aspect_stop");
	set_signal("signal32", "aspect_stop");

	testsuite_driveTo("seg58", -50, train);
	testsuite_driveToStop("seg59", -20, train);
	return true;
}

bool route2(const char *train) {
	if (!testsuite_trainReady(train, "seg58")) {
		return false;
	}

	switch_point("point22", "normal");
	switch_point("point20", "reverse");
	switch_point("point21", "reverse");
	switch_point("point16", "reverse");
	switch_point("point15", "reverse");
	switch_point("point5", "reverse");
	switch_point("point4", "reverse");
	switch_point("point12", "reverse");
	switch_point("point11", "reverse");
	switch_point("point27", "reverse");
	switch_point("point29", "reverse");
	switch_point("point28", "reverse");
	switch_point("point26", "normal");
	switch_point("point9", "reverse");
	switch_point("point8", "reverse");
	switch_point("point1", "reverse");
	switch_point("point7", "normal");
	switch_point("point6", "reverse");
	switch_point("point17", "reverse");

	testsuite_driveTo("seg42b", 50, train);
	
	switch_point("point16", "normal");
	switch_point("point15", "normal");
	switch_point("point14", "reverse");
	switch_point("point13", "reverse");
	switch_point("point12", "reverse");
	switch_point("point24", "reverse");
	switch_point("point23", "reverse");
	switch_point("point19", "reverse");
	switch_point("point18b", "normal");
	switch_point("point18a", "reverse");
	switch_point("point8", "normal");
	switch_point("point9", "reverse");
	switch_point("point26", "reverse");
	switch_point("point27", "normal");
	switch_point("point11", "normal");
	switch_point("point3", "reverse");
	switch_point("point4", "normal");
	switch_point("point5", "normal");

	testsuite_driveTo("seg69", 50, train);
	switch_point("point6", "normal");
	switch_point("point7", "reverse");

	testsuite_driveTo("seg46", 50, train);
	testsuite_driveToStop("seg47", 20, train);

	return true;
}

bool route3(const char *train) {
	if (!testsuite_trainReady(train, "seg46")) {
		return false;
	}

	switch_point("point18b", "reverse");
	switch_point("point19", "reverse");
	switch_point("point23", "reverse");
	switch_point("point24", "reverse");
	switch_point("point12", "normal");
	switch_point("point4", "reverse");
	switch_point("point5", "reverse");
	switch_point("point15", "reverse");
	switch_point("point16", "normal");
	switch_point("point17", "reverse");
	switch_point("point6", "reverse");
	switch_point("point7", "normal");
	switch_point("point1", "reverse");
	switch_point("point8", "reverse");
	switch_point("point9", "normal");
	switch_point("point10", "reverse");

	testsuite_driveTo("seg29", -50, train);
	testsuite_driveToStop("seg78b", -20, train);

	return true;
}

bool route4(const char *train) {
	if (!testsuite_trainReady(train, "seg78a")) {
		return false;
	}

	switch_point("point10", "reverse");
	switch_point("point9", "normal");
	switch_point("point8", "reverse");
	switch_point("point1", "reverse");
	switch_point("point7", "normal");
	switch_point("point6", "reverse");
	switch_point("point17", "reverse");
	switch_point("point16", "normal");
	switch_point("point15", "normal");
	switch_point("point14", "reverse");
	switch_point("point13", "reverse");
	switch_point("point12", "reverse");
	switch_point("point24", "reverse");
	switch_point("point23", "normal");
	switch_point("point22", "reverse");

	testsuite_driveTo("seg58", 50, train);
	testsuite_driveToStop("seg59", 20, train);

	return true;
}

bool route5(const char *train) {
	if (!testsuite_trainReady(train, "seg58")) {
		return false;
	}
	
	switch_point("point22", "reverse");
	switch_point("point23", "normal");
	switch_point("point24", "reverse");
	switch_point("point12", "reverse");
	switch_point("point13", "reverse");
	switch_point("point14", "reverse");
	switch_point("point15", "normal");
	switch_point("point16", "reverse");
	switch_point("point21", "reverse");
	switch_point("point20", "reverse");
	
	testsuite_driveTo("seg64", -50, train);
	switch_point("point22", "normal");
	
	testsuite_driveTo("seg58", -50, train);
	testsuite_driveToStop("seg59", -20, train);
	
	return true;
}

void testsuite_case_swtbahnFullTrackCoverage(const char *train) {
	if (!route1(train)) {
		return;
	}

	if (!route2(train)) {
		return;
	}

	if (!route3(train)) {
		return;
	}

	if (!route4(train)) {
		return;
	}

	if (!route5(train)) {
		return;
	}
}

static void *route99(void *arg) {
	const char *train1 = arg;

	if (!testsuite_trainReady(train1, "seg58")) {
		pthread_exit(NULL);
	}

	while (true) {
		// train1: forwards
		switch_point("point22", "reverse");
		switch_point("point23", "normal");
		switch_point("point24", "reverse");
		switch_point("point12", "reverse");
		switch_point("point13", "reverse");
		switch_point("point14", "reverse");
		switch_point("point15", "normal");
		switch_point("point16", "reverse");
		switch_point("point21", "reverse");
		switch_point("point20", "normal");
		switch_point("point19", "normal");
		switch_point("point18b", "reverse");
		
		sleep(1);

		set_signal("signal30", "aspect_go");
		set_signal("signal33", "aspect_go");
		set_signal("signal35a", "aspect_go");
		set_signal("signal35b", "aspect_go");
		set_signal("signal37", "aspect_go");
		
		sleep(1);

		testsuite_driveTo("seg57", 50, train1);
		set_signal("signal30", "aspect_stop");

		testsuite_driveTo("seg64", 50, train1);
		set_signal("signal33", "aspect_stop");
		set_signal("signal35a", "aspect_stop");
		set_signal("signal35b", "aspect_stop");

		testsuite_driveTo("seg69", 50, train1);
		set_signal("signal37", "aspect_stop");

		testsuite_driveTo("seg46", 50, train1);
		sleep(1);
		testsuite_driveTo("seg46", 40, train1);
		sleep(1);
		testsuite_driveTo("seg46", 30, train1);
		sleep(1);
		testsuite_driveToStop("seg47", 20, train1);
		
		sleep(5);

		// train1: backwards
		set_signal("signal26", "aspect_go");
		set_signal("signal38", "aspect_go");
		set_signal("signal36", "aspect_go");
		set_signal("signal34", "aspect_go");
		set_signal("signal32", "aspect_go");

		sleep(1);

		testsuite_driveTo("seg45", -50, train1);
		set_signal("signal26", "aspect_stop");

		testsuite_driveTo("seg67", -50, train1);
		set_signal("signal38", "aspect_stop");
		set_signal("signal36", "aspect_stop");

		testsuite_driveTo("seg62", -50, train1);
		set_signal("signal34", "aspect_stop");
		set_signal("signal32", "aspect_stop");

		testsuite_driveTo("seg60", -50, train1);
		testsuite_driveTo("seg53", -40, train1);
		testsuite_driveTo("seg57", -30, train1);
		testsuite_driveTo("seg58", -20, train1);
		sleep(2);
		testsuite_driveToStop("seg58", -20, train1);
		
		sleep(5);
	}
}

static void *route100(void *arg) {
	const char *train2 = arg;

	if (!testsuite_trainReady(train2, "seg78a")) {
		pthread_exit(NULL);
	}
	
	while (true) {
		// train2: forwards
		switch_point("point10", "reverse");
		switch_point("point9", "normal");
		switch_point("point8", "reverse");
		switch_point("point1", "reverse");
		switch_point("point7", "normal");
		switch_point("point6", "normal");
		switch_point("point5", "normal");
		switch_point("point4", "normal");
		switch_point("point3", "reverse");
		switch_point("point11", "reverse");
		
		sleep(1);

		set_signal("signal43", "aspect_shunt");
		set_signal("signal19", "aspect_go");
		set_signal("signal3", "aspect_go");
		set_signal("signal1", "aspect_go");
		set_signal("signal13", "aspect_go");
		set_signal("signal11", "aspect_go");
		set_signal("signal10", "aspect_go");
		set_signal("signal8", "aspect_go");
		
		sleep(1);
		
		testsuite_driveTo("seg77", 126, train2);
		set_signal("signal43", "aspect_stop");
		
		testsuite_driveTo("seg26", 126, train2);
		set_signal("signal19", "aspect_stop");
		
		testsuite_driveTo("seg1", 126, train2);
		set_signal("signal3", "aspect_stop");
		set_signal("signal1", "aspect_stop");
		
		testsuite_driveTo("seg15", 126, train2);
		set_signal("signal13", "aspect_stop");
		set_signal("signal11", "aspect_stop");
		
		testsuite_driveTo("seg11", 126, train2);
		set_signal("signal10", "aspect_stop");
		set_signal("signal8", "aspect_stop");
		
		testsuite_driveTo("seg31b", 50, train2);
		sleep(1);
		testsuite_driveTo("seg31b", 40, train2);
		sleep(1);
		testsuite_driveTo("seg31a", 30, train2);
		sleep(1);
		testsuite_driveToStop("seg31a", 20, train2);
		
		sleep(5);

		// train2: backwards
		set_signal("signal22a", "aspect_go");
		set_signal("signal22b", "aspect_go");
		set_signal("signal9", "aspect_go");
		set_signal("signal12", "aspect_go");
		set_signal("signal14", "aspect_go");
		set_signal("signal2", "aspect_go");
		set_signal("signal4a", "aspect_go");
		set_signal("signal4b", "aspect_go");
		set_signal("signal20", "aspect_shunt");
		
		sleep(1);

		testsuite_driveTo("seg32", -126, train2);
		set_signal("signal22a", "aspect_stop");
		set_signal("signal22b", "aspect_stop");
		
		testsuite_driveTo("seg13", -126, train2);
		set_signal("signal9", "aspect_stop");
		
		testsuite_driveTo("seg17", -126, train2);
		set_signal("signal12", "aspect_stop");
		set_signal("signal14", "aspect_stop");
		
		testsuite_driveTo("seg3", -126, train2);
		set_signal("signal2", "aspect_stop");
		set_signal("signal4a", "aspect_stop");
		set_signal("signal4b", "aspect_stop");
		
		testsuite_driveTo("seg28", -50, train2);
		set_signal("signal20", "aspect_stop");
		
		testsuite_driveTo("seg78a", -50, train2);
		sleep(1);
		testsuite_driveTo("seg78a", -40, train2);
		sleep(1);
		testsuite_driveTo("seg78a", -30, train2);
		sleep(1);
		testsuite_driveToStop("seg78a", -20, train2);
		
		sleep(5);
	}
}

void testsuite_case_swtbahnFullMultipleTrains(const char *train1, const char *train2) {
	static pthread_t route99_thread;
	static pthread_t route100_thread;

	pthread_create(&route99_thread, NULL, route99, (void*) train1);
	pthread_create(&route100_thread, NULL, route100, (void*) train2);
	
	pthread_join(route99_thread, NULL);
	pthread_join(route100_thread, NULL);
}


bool route_custom_short(const char *train) {
	if (!testsuite_trainReady(train, "seg7a")) {
		return false;
	}

	switch_point("point2", "normal");
	switch_point("point1", "normal");
	switch_point("point7", "normal");
	switch_point("point6", "normal");
	switch_point("point5", "normal");
	switch_point("point4", "normal");
	switch_point("point3", "normal");
	

	set_signal("signal5", "aspect_go");
	set_signal("signal17", "aspect_go");
	set_signal("signal", "aspect_go");

	testsuite_driveTo("seg58", 50, train);
	sleep(1);
	testsuite_driveToStop("seg59", 20, train);
	sleep(2);
	return true;
}
void testsuite_case_swtbahnFullShortRoute(const char *train)
{
	sleep(1);
	if (!route_custom_short(train)) {
		return;
	}
	printf("testsuite_case_swtbahnFullShortRoute finished.\n\n");
}
