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

#include "test_common.h"


#define SIGNAL_WAITING_TIME_S	3	   // in seconds
#define POINT_WAITING_TIME_S	3	   // in seconds
#define TRAIN_WAITING_TIME_US	250000 // in microseconds

t_bidib_id_list_query points;
t_bidib_id_list_query signals;

t_testsuite_test_result *testsuite_initTestSuite_common(char **excludedSignalAccessories, 
                                                        size_t excludedSignalAccessories_len) {
	points = bidib_get_connected_points();

	// Accessories that are not signals
	t_testsuite_ids filterOutIds;
	filterOutIds.ids = excludedSignalAccessories;
	filterOutIds.length = excludedSignalAccessories_len;
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

t_bidib_id_list_query testsuite_filterOutIds(t_bidib_id_list_query inputIdQuery, t_testsuite_ids filterOutIds) {
	const size_t count = inputIdQuery.length - filterOutIds.length;

	if (count <= 0) {
		printf("testsuite: No IDs will be left after filtering\n");
	}

	t_bidib_id_list_query outputIdQuery;
	outputIdQuery.length = 0;
	outputIdQuery.ids = malloc(sizeof(char *) * count);

	bool isFilteredOut = false;

	for (size_t i = 0; i < inputIdQuery.length; i++) {
		isFilteredOut = false;
		for (size_t j = 0; j < filterOutIds.length; j++) {
			if (!strcmp(inputIdQuery.ids[i], filterOutIds.ids[j])) {
				isFilteredOut = true;
				break;
			}
		}

		if (!isFilteredOut) {
			outputIdQuery.ids[outputIdQuery.length] = strdup(inputIdQuery.ids[i]);
			outputIdQuery.length++;
		}
	}

	if (outputIdQuery.length != count) {
		printf("testsuite: Error: %zu IDs were to be filtered, but %d IDs filtered instead\n", 
		       filterOutIds.length, (int)inputIdQuery.length - (int)outputIdQuery.length);
	}

	return outputIdQuery;
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

void testsuite_logTestResult(t_testsuite_test_result *result, 
                             t_bidib_unified_accessory_state_query state, 
                             int accessory_index) {
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

void testsuite_driveTo_legacy(const char *segment, int speed, const char *train) {
	// This driveTo impl queries the train position directly.
	// Kept for testing purposes, but deprecated.
	printf("testsuite: Drive %s to %s at speed %d\n", train, segment, speed);
	bidib_set_train_speed(train, speed, "master");
	bidib_flush();
	long counter = 0;
	while (1) {
		t_bidib_train_position_query trainPosition = bidib_get_train_position(train);
		for (size_t i = 0; i < trainPosition.length; i++) {
			if (!strcmp(segment, trainPosition.segments[i])) {
				struct timespec tv;
				clock_gettime(CLOCK_MONOTONIC, &tv);
				bidib_free_train_position_query(trainPosition);
				printf("testsuite: Drive %s to %s at speed %d - REACHED TARGET - detected at time %ld.%.5ld", 
				       train, segment, speed, tv.tv_sec, tv.tv_nsec);
				return;
			}
		}
		bidib_free_train_position_query(trainPosition);
		
		if (counter++ % 8 == 0) {
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			printf("testsuite: Drive %s to %s at speed %d - waiting for train to arrive, time %ld.%.5ld", 
			       train, segment, speed, tv.tv_sec, tv.tv_nsec);
		}
		
		usleep(TRAIN_WAITING_TIME_US);
	}
}

void testsuite_driveTo(const char *segment, int speed, const char *train) {
	// This driveTo impl works by querying the segment state repeatedly, not the train position.
	// -> bidib_get_segment_state does not need to lock the trainstate rwlock, thus hopefully
	//    reducing lock contention.
	printf("testsuite: Drive %s to %s at speed %d\n", train, segment, speed);
	bidib_set_train_speed(train, speed, "master");
	bidib_flush();
	t_bidib_dcc_address_query tr_dcc_addr = bidib_get_train_dcc_addr(train);
	t_bidib_dcc_address dcc_address;
	long counter = 0;
	while (1) {
		t_bidib_segment_state_query seg_query = bidib_get_segment_state(segment);
		for (size_t j = 0; j < seg_query.data.dcc_address_cnt; j++) {
			dcc_address = seg_query.data.dcc_addresses[j];
			if (tr_dcc_addr.dcc_address.addrh == dcc_address.addrh 
			    &&  tr_dcc_addr.dcc_address.addrl == dcc_address.addrl) {
				struct timespec tv;
				clock_gettime(CLOCK_MONOTONIC, &tv);
				bidib_free_segment_state_query(seg_query);
				printf("testsuite: Drive %s to %s at speed %d - REACHED TARGET - detected at time %ld.%.5ld\n", 
				       train, segment, speed, tv.tv_sec, tv.tv_nsec);
				return;
			}
		}
		bidib_free_segment_state_query(seg_query);
		
		if (counter++ % 8 == 0) {
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			printf("testsuite: Drive %s to %s at speed %d - waiting for train to arrive, time %ld.%.5ld\n", 
			       train, segment, speed, tv.tv_sec, tv.tv_nsec);
		}
		
		usleep(TRAIN_WAITING_TIME_US);
	}
}

void testsuite_driveToStop(const char *segment, int speed, const char *train) {
	testsuite_driveTo(segment, speed, train);
	bidib_set_train_speed(train, 0, "master");
	bidib_flush();
}

void testsuite_set_signal(const char *signal, const char *aspect) {
	bidib_set_signal(signal, aspect);
	bidib_flush();
}

void testsuite_switch_point(const char *point, const char *aspect) {
	bidib_switch_point(point, aspect);
	bidib_flush();
}

void testsuite_case_signal_common(char **aspects, size_t aspects_len) {
	for (size_t i = 0; i < aspects_len; i++) {
		for (size_t n = 0; n < signals.length; n++) {
			testsuite_set_signal(signals.ids[n], aspects[i]);
		}
		sleep(SIGNAL_WAITING_TIME_S);
	}
}

void testsuite_case_pointParallel_common(t_testsuite_test_result *result) {
	for (size_t i = 0; i < points.length; i++) {
		testsuite_switch_point(points.ids[i], "reverse");
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
	}

	sleep(POINT_WAITING_TIME_S);

	for (size_t i = 0; i < points.length; i++) {
		testsuite_switch_point(points.ids[i], "normal");
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
	}

	sleep(POINT_WAITING_TIME_S);
}

void testsuite_case_pointSerial_common(t_testsuite_test_result *result) {
	for (size_t i = 0; i < points.length; i++) {
		testsuite_switch_point(points.ids[i], "reverse");
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
		sleep(POINT_WAITING_TIME_S);

		testsuite_switch_point(points.ids[i], "normal");
		state = bidib_get_point_state(points.ids[i]);
		testsuite_logTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
		sleep(POINT_WAITING_TIME_S);
	}
}
