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
#define TRAIN_WAITING_TIME_US	125000 // in microseconds (0.125s)

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

t_bidib_id_list_query testsuite_filterOutIds(t_bidib_id_list_query inputIdQuery, 
                                             t_testsuite_ids filterOutIds) {
	if (filterOutIds.length >= inputIdQuery.length) {
		printf("testsuite: No IDs might be left after filtering!\n");
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
	printf("testsuite: Debug: After filtering IDs, there are %d IDs left", (int)outputIdQuery.length);
	return outputIdQuery;
}

void testsuite_stopBidib(void) {
	bidib_free_id_list_query(points);
	bidib_free_id_list_query(signals);
	bidib_stop();
}

void testsuite_signal_callback_handler(int signum) {
	char *sig_descr;
	if (signum == -1) {
		sig_descr = "ABORT-FROM-OBSERVER";
	} else {
		sig_descr = strsignal(signum);
	}
	printf("testsuite: SIG %s (%d) - before stopping, debug logs:\n", sig_descr, signum);
	printf("   Track output states:\n");
	testsuite_logAllTrackOutputStates();
	printf("\n");
	printf("   Booster power states:\n");
	testsuite_logAllBoosterPowerStates();
	printf("\n");
	printf("testsuite: SIG %s - now stopping libbidib.\n", sig_descr);
	testsuite_stopBidib();
	printf("testsuite: SIG %s - libbidib stopped.\n", sig_descr);
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
	if (result == NULL) {
		printf("testsuite: print test results - invalid parameters\n");
		return;
	}
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

// DEPRECATED
void testsuite_driveTo_legacy(const char *segment, int speed, const char *train) {
	// This driveTo impl queries the train position directly.
	// Kept for testing purposes, but deprecated.
	if (segment == NULL || train == NULL) {
		printf("testsuite: drive to (legacy) - invalid parameters\n");
		return;
	}
	printf("testsuite: drive %s to %s at speed %d (legacy)\n", train, segment, speed);
	bidib_set_train_speed(train, speed, "master");
	bidib_flush();
	long counter = 0;
	while (bidib_is_running()) {
		t_bidib_train_position_query trainPosition = bidib_get_train_position(train);
		for (size_t i = 0; i < trainPosition.length; i++) {
			if (strcmp(segment, trainPosition.segments[i]) == 0) {
				struct timespec tv;
				clock_gettime(CLOCK_MONOTONIC, &tv);
				bidib_free_train_position_query(trainPosition);
				printf("testsuite: drive %s to %s at speed %d - REACHED TARGET - "
				       "detected at time %ld.%.ld\n", 
				       train, segment, speed, tv.tv_sec, tv.tv_nsec);
				return;
			}
		}
		bidib_free_train_position_query(trainPosition);
		
		if (counter++ % 8 == 0) {
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			printf("testsuite: drive %s to %s at speed %d - "
			       "waiting for train to arrive, time %ld.%.ld\n", 
			       train, segment, speed, tv.tv_sec, tv.tv_nsec);
		}
		
		usleep(TRAIN_WAITING_TIME_US);
	}
}

void testsuite_driveTo(const char *segment, int speed, const char *train) {
	// This driveTo impl works by querying the segment state repeatedly, not the train position.
	// -> bidib_get_segment_state does not need to lock the trainstate rwlock, thus hopefully
	//    reducing lock contention.
	if (segment == NULL || train == NULL) {
		printf("testsuite: drive to - invalid parameters\n");
		return;
	}
	printf("testsuite: drive %s to %s at speed %d\n", train, segment, speed);
	bidib_set_train_speed(train, speed, "master");
	bidib_flush();
	t_bidib_dcc_address_query tr_dcc_addr = bidib_get_train_dcc_addr(train);
	t_bidib_dcc_address dcc_address;
	long counter = 0;
	while (bidib_is_running()) {
		t_bidib_segment_state_query seg_query = bidib_get_segment_state(segment);
		for (size_t j = 0; j < seg_query.data.dcc_address_cnt; j++) {
			dcc_address = seg_query.data.dcc_addresses[j];
			if (tr_dcc_addr.dcc_address.addrh == dcc_address.addrh 
			    &&  tr_dcc_addr.dcc_address.addrl == dcc_address.addrl) {
				struct timespec tv;
				clock_gettime(CLOCK_MONOTONIC, &tv);
				bidib_free_segment_state_query(seg_query);
				printf("testsuite: drive %s to %s at speed %d - REACHED TARGET - "
				       "detected at time %ld.%.ld\n", 
				       train, segment, speed, tv.tv_sec, tv.tv_nsec);
				return;
			}
		}
		bidib_free_segment_state_query(seg_query);
		
		if (counter++ % 8 == 0) {
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			printf("testsuite: drive %s to %s at speed %d - "
			       "waiting for train to arrive, time %ld.%.ld\n", 
			       train, segment, speed, tv.tv_sec, tv.tv_nsec);
		}
		
		usleep(TRAIN_WAITING_TIME_US);
	}
}

void testsuite_driveToStop(const char *segment, int speed, const char *train) {
	if (segment == NULL || train == NULL) {
		printf("testsuite: drive to Stop - invalid parameters\n");
		return;
	}
	testsuite_driveTo(segment, speed, train);
	bidib_set_train_speed(train, 0, "master");
	bidib_flush();
}

bool testsuite_is_segment_occupied(const char *segment) {
	t_bidib_segment_state_query seg_query = bidib_get_segment_state(segment);
	bool ret = seg_query.known && seg_query.data.occupied;
	bidib_free_segment_state_query(seg_query);
	return ret;
}

// Currently unused, but could be useful for future test adjustments
bool testsuite_is_segment_occupied_by_train(const char *segment, const char *train) {
	t_bidib_dcc_address_query tr_dcc_addr = bidib_get_train_dcc_addr(train);
	return testsuite_is_segment_occupied_by_dcc_addr(segment, tr_dcc_addr.dcc_address);
}

bool testsuite_is_segment_occupied_by_dcc_addr(const char *segment, t_bidib_dcc_address dcc_address) {
	t_bidib_segment_state_query seg_query = bidib_get_segment_state(segment);
	if (!(seg_query.known && seg_query.data.occupied)) {
		bidib_free_segment_state_query(seg_query);
		return false;
	}
	for (size_t j = 0; j < seg_query.data.dcc_address_cnt; j++) {
		t_bidib_dcc_address *seg_dcc_j = &seg_query.data.dcc_addresses[j];
		if (dcc_address.addrh == seg_dcc_j->addrh && dcc_address.addrl == seg_dcc_j->addrl) {
			bidib_free_segment_state_query(seg_query);
			return true;
		}
	}
	bidib_free_segment_state_query(seg_query);
	return false;
}

void testsuite_set_signal(const char *signal, const char *aspect) {
	if (signal == NULL || aspect == NULL) {
		printf("testsuite: set signal - invalid parameters\n");
		return;
	}
	bidib_set_signal(signal, aspect);
	bidib_flush();
}

void testsuite_switch_point(const char *point, const char *aspect) {
	if (point == NULL || aspect == NULL) {
		printf("testsuite: switch point - invalid parameters\n");
		return;
	}
	bidib_switch_point(point, aspect);
	bidib_flush();
}

bool testsuite_check_point_aspect(const char *point, const char *aspect) {
	if (point == NULL || aspect == NULL) {
		printf("testsuite: check point aspect - invalid parameters\n");
		return false;
	}
	t_bidib_unified_accessory_state_query state = bidib_get_point_state(point);
	if (!state.known) {
		printf("testsuite: check point aspect - unknown point %s\n", point);
		bidib_free_unified_accessory_state_query(state);
		return false;
	}
	if (state.type == BIDIB_ACCESSORY_BOARD) {
		if (strcmp(state.board_accessory_state.state_id, aspect) == 0) {
			bidib_free_unified_accessory_state_query(state);
			return true;
		} else {
			printf("testsuite: check point aspect - point %s is in aspect %s, not in %s\n",
			       point, state.board_accessory_state.state_id, aspect);
		}
	} else {
		if (strcmp(state.dcc_accessory_state.state_id, aspect) == 0) {
			bidib_free_unified_accessory_state_query(state);
			return true;
		} else {
			printf("testsuite: check point aspect - point %s is in aspect %s, not in %s\n",
			       point, state.dcc_accessory_state.state_id, aspect);
		}
	}
	
	bidib_free_unified_accessory_state_query(state);
	return false;
}

bool testsuite_set_and_check_points(const char **points_normal, int points_normal_len,
                                    const char **points_reverse, int points_reverse_len) {
	if (points_normal_len > 0 && points_normal != NULL) {
		for (int i = 0; i < points_normal_len; i++) {
			testsuite_switch_point(points_normal[i], "normal");
		}
	}
	if (points_reverse_len > 0 && points_reverse != NULL) {
		for (int i = 0; i < points_reverse_len; i++) {
			testsuite_switch_point(points_reverse[i], "reverse");
		}
	}
	sleep(POINT_WAITING_TIME_S);
	bool point_check = true;
	if (points_normal_len > 0 && points_normal != NULL) {
		for (int i = 0; i < points_normal_len; i++) {
			point_check &= testsuite_check_point_aspect(points_normal[i], "normal");
		}
	}
	if (points_reverse_len > 0 && points_reverse != NULL) {
		for (int i = 0; i < points_reverse_len; i++) {
			point_check &= testsuite_check_point_aspect(points_reverse[i], "reverse");
		}
	}
	return point_check;
}

void testsuite_set_signals_to(const char **signals, int signals_len, const char *aspect) {
	if (signals_len > 0 && signals != NULL && aspect != NULL) {
		for (int i = 0; i < signals_len; i++) {
			testsuite_set_signal(signals[i], aspect);
		}
	}
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
	const char *aspects[2] = {"reverse", "normal"};
	for (size_t aspect_index = 0; aspect_index < 2; aspect_index++) {
		for (size_t i = 0; i < points.length; i++) {
			testsuite_switch_point(points.ids[i], aspects[aspect_index]);
		}
		// give points time to switch and for this change in state to be sent as feedback by the boards
		sleep(POINT_WAITING_TIME_S);
		// record point state
		for (size_t i = 0; i < points.length; i++) {
			t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i]);
			testsuite_recordTestResult(result, state, i);
			bidib_free_unified_accessory_state_query(state);
		}
		// No sleep needed here, points/point states will have settled by this time
	}
}


void testsuite_case_pointSerial_common(t_testsuite_test_result *result) {
	const char *aspects[2] = {"reverse", "normal"};
	for (size_t aspect_index = 0; aspect_index < 2; aspect_index++) {
		// The points are first all switched to reverse one after the other.
		// In iteration k, the switch for point with id at .ids[k] is commanded, 
		// AND the feedback for the point that was switched in iteration k-1 is gathered.
		// -> this is done to avoid having to wait 4x3 seconds per point.
		for (size_t i = 0; i <= points.length; i++) {
			if (i < points.length) {
				testsuite_switch_point(points.ids[i], aspects[aspect_index]);
			}
			if (i >= 1) {
				t_bidib_unified_accessory_state_query state = bidib_get_point_state(points.ids[i-1]);
				testsuite_recordTestResult(result, state, i-1);
				bidib_free_unified_accessory_state_query(state);
			}
			sleep(POINT_WAITING_TIME_S);
		}
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