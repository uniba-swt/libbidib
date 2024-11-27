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
	ssize_t count = inputIdQuery.length - filterOutIds.length;

	if (count <= 0) {
		printf("testsuite: No IDs might be left after filtering\n");
		count = 0;
	}

	t_bidib_id_list_query outputIdQuery;
	outputIdQuery.length = 0;
	// the outputIdQuery will likely be smaller, but we don't know if the IDs 
	// in filterOutIds are actually contained in inputIdQuery, thus theoretically
	// the length of outputIdQuery can be that of inputIdQuery at most.
	outputIdQuery.ids = malloc(sizeof(char *) * inputIdQuery.length);

	bool isFilteredOut = false;

	for (size_t i = 0; i < inputIdQuery.length; i++) {
		isFilteredOut = false;
		for (size_t j = 0; j < filterOutIds.length; j++) {
			if (strcmp(inputIdQuery.ids[i], filterOutIds.ids[j]) == 0) {
				isFilteredOut = true;
				break;
			}
		}

		if (!isFilteredOut) {
			outputIdQuery.ids[outputIdQuery.length] = strdup(inputIdQuery.ids[i]);
			outputIdQuery.length++;
		}
	}

	if (outputIdQuery.length != (size_t) count) {
		// can occur if the IDs to be filtered are those of accessories currently not connected.
		printf("testsuite: Notice: %zu IDs were to be filtered, %d IDs were actually filtered\n", 
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

void testsuite_recordTestResult(t_testsuite_test_result *result, 
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

void testsuite_driveTo(const char *segment, int speed, const char *train) {
	bidib_set_train_speed(train, speed, "master");
	bidib_flush();

	while (1) {
		t_bidib_train_position_query trainPosition = bidib_get_train_position(train);
		for (size_t i = 0; i < trainPosition.length; i++) {
			if (strcmp(segment, trainPosition.segments[i]) == 0) {
				bidib_free_train_position_query(trainPosition);
				return;
			}
		}
		bidib_free_train_position_query(trainPosition);
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
	// like the old pointParallel, but here an appropriate amount of time is waited
	// after setting the points until feedback is gathered. Because point switchting
	// takes some time, it doesn't make sense to get the state/feedback immediately
	// after sending the command to switch the point.
	
	for (size_t i = 0; i < points.length; i++) {
		testsuite_switch_point(points.ids[i], "reverse");
	}
	
	sleep(POINT_WAITING_TIME_S);
	
	for (size_t i = 0; i < points.length; i++) {
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_recordTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
	}

	sleep(POINT_WAITING_TIME_S);

	for (size_t i = 0; i < points.length; i++) {
		testsuite_switch_point(points.ids[i], "normal");
	}

	sleep(POINT_WAITING_TIME_S);
	
	for (size_t i = 0; i < points.length; i++) {
		t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
		testsuite_recordTestResult(result, state, i);
		bidib_free_unified_accessory_state_query(state);
	}
}


void testsuite_case_pointSerial_common(t_testsuite_test_result *result) {
	// The points are first all switched to reverse one after the other.
	// In iteration k, the switch for point with id at .ids[k] is commanded, 
	// AND the feedback for the point that was switched in iteration k-1 is gathered.
	// -> this is done to avoid having to wait 4x3 seconds per point.
	for (size_t i = 0; i <= points.length; i++) {
		if (i < points.length) {
			testsuite_switch_point(points.ids[i], "reverse");
		}
		if (i >= 1) {
			t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i-1]);
			testsuite_recordTestResult(result, state, i-1);
			bidib_free_unified_accessory_state_query(state);
		}
		sleep(POINT_WAITING_TIME_S);
	}
	
	// Now the same for switching the points to normal.
	for (size_t i = 0; i <= points.length; i++) {
		if (i < points.length) {
			testsuite_switch_point(points.ids[i], "normal");
		}
		if (i >= 1) {
			t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i-1]);
			testsuite_recordTestResult(result, state, i-1);
			bidib_free_unified_accessory_state_query(state);
		}
		sleep(POINT_WAITING_TIME_S);
	}
}

const char * const testsuite_cs_state_str[] = {
	[BIDIB_CS_OFF] = "BIDIB_CS_OFF",
	[BIDIB_CS_STOP] = "BIDIB_CS_STOP",
	[BIDIB_CS_SOFTSTOP] = "BIDIB_CS_SOFTSTOP",
	[BIDIB_CS_GO] = "BIDIB_CS_GO",
	[BIDIB_CS_GO_IGN_WD] = "BIDIB_CS_GO_IGN_WD",
	[BIDIB_CS_PROG] = "BIDIB_CS_PROG",
	[BIDIB_CS_PROGBUSY] = "BIDIB_CS_PROGBUSY",
	[BIDIB_CS_BUSY] = "BIDIB_CS_BUSY",
	[BIDIB_CS_QUERY] = "BIDIB_CS_QUERY"
};

void testsuite_logAllTrackOutputStates() {
	t_bidib_id_list_query tr_outpts_query = bidib_get_track_outputs();
	if (tr_outpts_query.ids == NULL || tr_outpts_query.length == 0) {
		printf("testsuite: list of track outputs ids is null or empty\n");
		return;
	}
	
	for (size_t i = 0; i < tr_outpts_query.length; i++) {
		t_bidib_track_output_state_query state_q;
		state_q = bidib_get_track_output_state(tr_outpts_query.ids[i]);
		if (!state_q.known) {
			printf("testsuite: track output %s is unknown and/or has unknown state\n", 
			       tr_outpts_query.ids[i]);
		} else {
			printf("testsuite: track output %s has state %s\n", 
			       tr_outpts_query.ids[i], testsuite_cs_state_str[state_q.cs_state]);
		}
	}
	
	bidib_free_id_list_query(tr_outpts_query);
}

// See also https://bidib.org/protokoll/bidib_booster.html -> MSG_BOOST_STAT
const char * const testsuite_bstr_powerstate_str[] = {
	[BIDIB_BSTR_OFF] = "BIDIB_BSTR_OFF",
	[BIDIB_BSTR_OFF_SHORT] = "BIDIB_BSTR_OFF_SHORT",
	[BIDIB_BSTR_OFF_HOT] = "BIDIB_BSTR_OFF_HOT",
	[BIDIB_BSTR_OFF_NOPOWER] = "BIDIB_BSTR_OFF_NOPOWER",
	[BIDIB_BSTR_OFF_GO_REQ] = "BIDIB_BSTR_OFF_GO_REQ",
	[BIDIB_BSTR_OFF_HERE] = "BIDIB_BSTR_OFF_HERE",
	[BIDIB_BSTR_OFF_NO_DCC] = "BIDIB_BSTR_OFF_NO_DCC",
	[BIDIB_BSTR_ON] = "BIDIB_BSTR_ON",
	[BIDIB_BSTR_ON_LIMIT] = "BIDIB_BSTR_ON_LIMIT",
	[BIDIB_BSTR_ON_HOT] = "BIDIB_BSTR_ON_HOT",
	[BIDIB_BSTR_ON_STOP_REQ] = "BIDIB_BSTR_ON_STOP_REQ",
	[BIDIB_BSTR_ON_HERE] = "BIDIB_BSTR_ON_HERE"
};

void testsuite_logAllBoosterPowerStates() {
	t_bidib_id_list_query boosters_query = bidib_get_boosters();
	if (boosters_query.ids == NULL || boosters_query.length == 0) {
		printf("testsuite: list of booster id query is null or empty\n");
		return;
	}
	
	for (size_t i = 0; i < boosters_query.length; i++) {
		t_bidib_booster_state_query b_state = bidib_get_booster_state(boosters_query.ids[i]);
		if (!b_state.known) {
			printf("testsuite: booster %s is unknown and/or has unknown state\n", 
			       boosters_query.ids[i]);
		} else {
			printf("testsuite: booster %s has power state %s\n", 
			       boosters_query.ids[i], testsuite_bstr_powerstate_str[b_state.data.power_state]);
		}
		
	}
	bidib_free_id_list_query(boosters_query);
}