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

// This initialisation function is specific to SWTbahn Full.
t_testsuite_test_result *testsuite_initTestSuite() {
	char *excludedSignalAccessories[4] = {"platformlight1", "platformlight2", 
	                                      "platformlight4a", "platformlight4b"};
	return testsuite_initTestSuite_common(excludedSignalAccessories, 4);
}

void testsuite_case_signal() {
	char *signalAspects[4] = {"aspect_caution", "aspect_go", "aspect_stop", "aspect_shunt"};
	testsuite_case_signal_common(signalAspects, 4);
}

void testsuite_case_pointParallel(t_testsuite_test_result *result) {
	testsuite_case_pointParallel_common(result);
}

void testsuite_case_pointSerial(t_testsuite_test_result *result) {
	testsuite_case_pointSerial_common(result);
}

typedef struct {
	char *train;
	char *forbidden_segment;
	// if true, only checks whether any of the forbidden segments gets occupied,
	// not if it gets occupied *by the train*.
	bool check_occ_only;
	// To be set by the caller to request the observer to terminate.
	volatile bool stop_requested;
} t_bidib_occ_observer_info;

static void free_obs_info_util(t_bidib_occ_observer_info *obs_info_ptr) {
	if (obs_info_ptr != NULL) {
		if (obs_info_ptr->forbidden_segment != NULL) {
			free(obs_info_ptr->forbidden_segment);
		}
		if (obs_info_ptr->train != NULL) {
			free(obs_info_ptr->train);
		}
		free(obs_info_ptr);
		obs_info_ptr = NULL;
	}
}

// Expects the arg pointer to be a pointer to a t_bidib_occ_observer_info struct.
static void *occupancy_observer(void *arg) {
	if (arg == NULL) {
		printf("testsuite: Occ-Observer exit: arg is NULL\n");
		pthread_exit(NULL);
	}
	t_bidib_occ_observer_info *obs_info = arg;
	t_bidib_dcc_address_query tr_dcc_addr = bidib_get_train_dcc_addr(obs_info->train);
	if (!tr_dcc_addr.known) {
		printf("testsuite: Occ-Observer exit: tr_dcc_addr unknown\n");
		pthread_exit(NULL);
	}
	long counter = 0;
	while (!obs_info->stop_requested && bidib_is_running()) {
		const char *i_segmt = obs_info->forbidden_segment;
		bool violation = false;
		if (i_segmt == NULL) {
			pthread_exit(NULL);
		} else if (obs_info->check_occ_only) {
			violation = testsuite_is_segment_occupied(i_segmt);
		} else {
			violation = testsuite_is_segment_occupied_by_dcc_addr(i_segmt, tr_dcc_addr.dcc_address);
		}
		if (violation) {
			printf("!!!\nOccupancy Observer: Train %s (or something else) occupies forbidden "
			       "segment %s! Stopping train, then stopping bidib.\n!!!\n",
			       obs_info->train, i_segmt);
			bidib_set_train_speed(obs_info->train, 0, "master");
			bidib_flush();
			// Stop via the signal handler for consistent debugging
			testsuite_signal_callback_handler(-1);
			pthread_exit(NULL);
		}
		usleep(100000); // 0.1s
		if (counter++ % 10 == 0) {
			printf("testsuite: Observer heartbeat - observing %s\n", i_segmt);
		}
	}
	pthread_exit(NULL);
}

static void prep_observer_segment_info(t_bidib_occ_observer_info *obs_info, const char *segment) {
	obs_info->stop_requested = false;
	if (obs_info->forbidden_segment != NULL) {
		free(obs_info->forbidden_segment);
	}
	obs_info->forbidden_segment = strdup(segment);
}

static bool stop_observer_and_check_still_running(t_bidib_occ_observer_info *obs_info, 
                                                  pthread_t observer_thread, 
                                                  const char *logname) {
	obs_info->stop_requested = true;
	pthread_join(observer_thread, NULL);
	if (!bidib_is_running()) {
		printf("testsuite: %s - stop, bidib is not running anymore.\n", logname);
		free_obs_info_util(obs_info);
		return false;
	}
	return true;
}

static bool route1(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg58")) {
		return false;
	}

	const char *points_normal[4] = { 
		"point15", "point23", "point20", "point19"
	};
	const char *points_reverse[8] = {
		"point22", "point24", "point12", "point13", "point14", "point16", "point21", "point18b"
	};

	if (!testsuite_set_and_check_points(points_normal, 4, points_reverse, 8)) {
		printf("testsuite: route1 - one or more points are not in expected aspect.\n");
		return false;
	}

	const char* signals_go_1[5] = {"signal30", "signal33", "signal35a", "signal35b", "signal37"};
	testsuite_set_signals_to(signals_go_1, 5, "aspect_go");

	testsuite_driveTo("seg57", 50, train);
	testsuite_set_signal("signal30", "aspect_stop");

	testsuite_driveTo("seg64", 50, train);

	const char* signals_stop_1[3] = {"signal33", "signal35a", "signal35b"};
	testsuite_set_signals_to(signals_stop_1, 3, "aspect_stop");

	testsuite_driveTo("seg69", 50, train);
	testsuite_set_signal("signal37", "aspect_stop");

	testsuite_driveTo("seg46", 50, train);
	testsuite_driveToStop("seg47", 20, train);

	// Drive backwards through the route
	const char* signals_go_2[5] = {"signal26", "signal38", "signal36", "signal34", "signal32"};
	testsuite_set_signals_to(signals_go_2, 5, "aspect_go");

	testsuite_driveTo("seg45", -50, train);
	testsuite_set_signal("signal26", "aspect_stop");

	testsuite_driveTo("seg67", -50, train);
	testsuite_set_signal("signal38", "aspect_stop");
	testsuite_set_signal("signal36", "aspect_stop");

	testsuite_driveTo("seg62", -50, train);
	testsuite_set_signal("signal34", "aspect_stop");
	testsuite_set_signal("signal32", "aspect_stop");

	testsuite_driveTo("seg58", -50, train);
	testsuite_driveToStop("seg59", -20, train);
	return true;
}

static bool route2(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg58")) {
		return false;
	}

	const char *points_normal_c1[3] = { 
		"point22", "point26", "point7"
	};
	const char *points_reverse_c1[16] = {
		"point20", "point21", "point16", "point15", "point5", "point4", "point12", "point11", 
		"point27", "point29", "point28", "point9", "point8", "point1", "point6", "point17",
	};

	if (!testsuite_set_and_check_points(points_normal_c1, 3, points_reverse_c1, 16)) {
		printf("testsuite: route2 - one or more points are not in expected aspect (1st check).\n");
		return false;
	}

	testsuite_driveToStop("seg42b", 50, train);

	const char *points_normal_c2[8] = { 
		"point16", "point15", "point8", "point18b", "point27", "point11", "point4", "point5"
	};
	const char *points_reverse_c2[10] = {
		"point14", "point13", "point12", "point24", "point23", 
		"point19", "point18a", "point9", "point26", "point3"
	};

	if (!testsuite_set_and_check_points(points_normal_c2, 8, points_reverse_c2, 10)) {
		printf("testsuite: route2 - one or more points are not in expected aspect (2nd check).\n");
		return false;
	}

	testsuite_driveToStop("seg69", 50, train);

	const char *points_normal_c3[1] = { "point6" };
	const char *points_reverse_c3[1] = { "point7" };

	if (!testsuite_set_and_check_points(points_normal_c3, 1, points_reverse_c3, 1)) {
		printf("testsuite: route2 - one or more points are not in expected aspect (3rd check).\n");
		return false;
	}

	testsuite_driveTo("seg46", 50, train);
	testsuite_driveToStop("seg47", 20, train);
	return true;
}

static bool route3(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg46")) {
		return false;
	}

	const char *points_normal[4] = { 
		"point7", "point12", "point16", "point9"
	};
	const char *points_reverse[12] = {
		"point18b", "point19", "point23", "point24", "point4", "point5", 
		"point15", "point17", "point6", "point1", "point8", "point10",
	};

	if (!testsuite_set_and_check_points(points_normal, 4, points_reverse, 12)) {
		printf("testsuite: route3 - one or more points are not in expected aspect.\n");
		return false;
	}

	testsuite_driveTo("seg29", -50, train);
	testsuite_driveToStop("seg78b", -20, train);

	return true;
}

static bool route4(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg78a")) {
		return false;
	}

	const char *points_normal[5] = { 
		"point9", "point7", "point16", "point15", "point23"
	};
	const char *points_reverse[10] = {
		"point10", "point8", "point1", "point6", "point17", 
		"point14", "point13", "point12", "point24", "point22"
	};

	if (!testsuite_set_and_check_points(points_normal, 5, points_reverse, 10)) {
		printf("testsuite: route4 - one or more points are not in expected aspect.\n");
		return false;
	}

	testsuite_driveTo("seg58", 50, train);
	testsuite_driveToStop("seg59", 20, train);

	return true;
}

static bool route5(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg58")) {
		return false;
	}

	const char *points_normal_c1[2] = { 
		"point23", "point15"
	};
	const char *points_reverse_c1[8] = {
		"point22", "point24", "point12", "point13", "point14", "point16", "point21", "point20"
	};

	if (!testsuite_set_and_check_points(points_normal_c1, 2, points_reverse_c1, 8)) {
		printf("testsuite: route5 - one or more points are not in expected aspect (1st check).\n");
		return false;
	}
	
	testsuite_driveToStop("seg64", -50, train);
	testsuite_switch_point("point22", "normal");
	sleep(3);
	// Check that point is in desired position
	if (!testsuite_check_point_aspect("point21", "reverse")) {
		printf("testsuite: route5 - point 21 is not in expected aspect (2nd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg58", -50, train);
	testsuite_driveToStop("seg59", -20, train);
	
	return true;
}

void testsuite_case_swtbahnFullTrackCoverage(const char *train) {
	if (train == NULL) {
		printf("testsuite: swtbahn-full track coverage single train - train is NULL\n");
		return;
	}
	
	if (!route1(train)) {
		printf("testsuite: swtbahn-full track coverage single train - route1 failed.\n");
		return;
	}

	if (!route2(train)) {
		printf("testsuite: swtbahn-full track coverage single train - route2 failed.\n");
		return;
	}

	if (!route3(train)) {
		printf("testsuite: swtbahn-full track coverage single train - route3 failed.\n");
		return;
	}

	if (!route4(train)) {
		printf("testsuite: swtbahn-full track coverage single train - route4 failed.\n");
		return;
	}

	if (!route5(train)) {
		printf("testsuite: swtbahn-full track coverage single train - route5 failed.\n");
		return;
	}
}


static bool route_a(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg58")) {
		return false;
	}
	// Starting at platform1 (B13)
	
	const char *points_normal_c1[9] = {
		"point23", "point15", "point16", "point7", "point1", "point2", "point3", "point4", "point5"
	};
	const char *points_reverse_c1[6] = {
		"point22", "point24", "point12", "point13", "point14", "point17"
	};
	if (!testsuite_set_and_check_points(points_normal_c1, 9, points_reverse_c1, 6)) {
		printf("testsuite: route_a - one or more points are not in expected aspect (1st check).\n");
		return false;
	}
	
	testsuite_driveTo("seg16a", 50, train);
	testsuite_driveToStop("seg16b", 20, train);
	// Now at block B4
	
	const char *points_normal_c2[2] = {
		"point6", "point18b"
	};
	const char *points_reverse_c2[1] = {
		"point7"
	};
	if (!testsuite_set_and_check_points(points_normal_c2, 2, points_reverse_c2, 1)) {
		printf("testsuite: route_a - one or more points are not in expected aspect (2nd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg46", 50, train);
	testsuite_driveToStop("seg47", 20, train);
	// Now at platform3 (B11)
	return true;
}

static bool route_b(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg47")) {
		return false;
	}
	// Starting at platform3 (B11)
	
	const char *points_normal_c1[10] = {
		"point19", "point20", "point15", "point14", "point13", 
		"point12", "point11", "point10", "point9", "point8"
	};
	const char *points_reverse_c1[3] = {
		"point18b", "point21", "point16"
	};
	if (!testsuite_set_and_check_points(points_normal_c1, 10, points_reverse_c1, 3)) {
		printf("testsuite: route_b - one or more points are not in expected aspect (1st check).\n");
		return false;
	}
	
	
	testsuite_driveTo("seg27", -50, train);
	testsuite_driveToStop("seg26", -20, train);
	// Now at block B6
	
	const char *points_reverse_c2[1] = { "point10" };
	if (!testsuite_set_and_check_points(NULL, 0, points_reverse_c2, 1)) {
		printf("testsuite: route_b - one or more points are not in expected aspect (2nd check).\n");
		return false;
	}
	// point10 is a bit finicky -> give staff 1s extra to ensure the point switched correctly
	sleep(1);
	
	testsuite_driveTo("seg78b", 50, train);
	testsuite_driveToStop("seg79", 20, train);
	// Now at platform5 (B18)
	// No points need changing.
	sleep(1);
	
	testsuite_driveTo("seg22", -50, train);
	testsuite_driveToStop("seg21b", -20, train);
	// Now at block B5
	
	const char *points_normal_c3[4] = {
		"point18b", "point19", "point20", "point21"
	};
	const char *points_reverse_c3[2] = {
		"point18a", "point25"
	};
	if (!testsuite_set_and_check_points(points_normal_c3, 4, points_reverse_c3, 2)) {
		printf("testsuite: route_b - one or more points are not in expected aspect (3rd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg75", -50, train);
	testsuite_driveToStop("seg76", -20, train);
	// Now at platform6 (B17)
	
	return true;
}

static bool route_c(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg76")) {
		return false;
	}
	// Starting at platform6 (B17)
	
	const char *points_normal_c1[1] = {
		"point21"
	};
	const char *points_reverse_c1[3] = {
		"point25", "point20", "point22"
	};
	if (!testsuite_set_and_check_points(points_normal_c1, 1, points_reverse_c1, 3)) {
		printf("testsuite: route_c - one or more points are not in expected aspect (1st check).\n");
		return false;
	}
	
	testsuite_driveTo("seg55", 50, train);
	testsuite_driveToStop("seg56", 20, train);
	// Now at platform2 (B12)
	
	const char *points_normal_c2[2] = {
		"point22", "point23"
	};
	if (!testsuite_set_and_check_points(points_normal_c2, 2, NULL, 0)) {
		printf("testsuite: route_c - one or more points are not in expected aspect (2nd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg62", -50, train);
	testsuite_driveToStop("seg63", -20, train);
	// Now at B14
	
	const char *points_normal_c3[2] = {
		"point18b", "point12"
	};
	const char *points_reverse_c3[5] = {
		"point23", "point19", "point18a", "point8", "point4"
	};
	if (!testsuite_set_and_check_points(points_normal_c3, 2, points_reverse_c3, 5)) {
		printf("testsuite: route_c - one or more points are not in expected aspect (3rd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg11", 50, train);
	testsuite_driveToStop("seg12", 20, train);
	// Now at B3
	
	const char *points_normal_c4[5] = {
		"point4", "point11", "point29", "point28", "point26"
	};
	const char *points_reverse_c4[4] = {
		"point27", "point9", "point8", "point1"
	};
	if (!testsuite_set_and_check_points(points_normal_c4, 5, points_reverse_c4, 4)) {
		printf("testsuite: route_c - one or more points are not in expected aspect (4th check).\n");
		return false;
	}
	
	testsuite_driveTo("seg3", -50, train);
	testsuite_driveToStop("seg2", -20, train);
	// Now at B1
	
	const char *points_normal_c5[5] = {
		"point1", "point9", "point26", "point11", "point4"
	};
	const char *points_reverse_c5[5] = {
		"point2", "point28", "point29", "point27", "point3"
	};
	if (!testsuite_set_and_check_points(points_normal_c5, 5, points_reverse_c5, 5)) {
		printf("testsuite: route_c - one or more points are not in expected aspect (5th check).\n");
		return false;
	}
	
	testsuite_driveTo("seg11", 50, train);
	testsuite_driveToStop("seg12", 20, train);
	// Now at B3
	
	const char *points_normal_c6[3] = {
		"point4", "point11", "point27"
	};
	const char *points_reverse_c6[1] = {
		"point3"
	};
	if (!testsuite_set_and_check_points(points_normal_c6, 3, points_reverse_c6, 1)) {
		printf("testsuite: route_c - one or more points are not in expected aspect (6th check).\n");
		return false;
	}
	
	testsuite_driveTo("seg82b", -50, train);
	testsuite_driveToStop("seg82a", -20, train);
	// Now at B19
	
	return true;
}

static bool route_d(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg82a")) {
		return false;
	}
	// Starting at B19
	
	const char *points_normal_c1[5] = {
		"point9", "point1", "point7", "point6", "point5"
	};
	const char *points_reverse_c1[2] = {
		"point26", "point2"
	};
	if (!testsuite_set_and_check_points(points_normal_c1, 5, points_reverse_c1, 2)) {
		printf("testsuite: route_d - one or more points are not in expected aspect (1st check).\n");
		return false;
	}
	
	testsuite_driveTo("seg13", -50, train);
	testsuite_driveToStop("seg12", -20, train);
	// Now at B3
	
	const char *points_normal_c2[3] = {
		"point16", "point17", "point18a"
	};
	const char *points_reverse_c2[2] = {
		"point5", "point15"
	};
	if (!testsuite_set_and_check_points(points_normal_c2, 3, points_reverse_c2, 2)) {
		printf("testsuite: route_d - one or more points are not in expected aspect (2nd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg21b", 50, train);
	testsuite_driveToStop("seg22", 20, train);
	// Now at B5
	
	return true;
}

static bool route_e(const char *train) {
	if (train == NULL || !testsuite_trainReady(train, "seg22")) {
		return false;
	}
	// Starting at B5
	
	const char *points_normal_c1[5] = {
		"point18b", "point19", "point20", "point21", "point25"
	};
	const char *points_reverse_c1[1] = {
		"point18a"
	};
	if (!testsuite_set_and_check_points(points_normal_c1, 5, points_reverse_c1, 1)) {
		printf("testsuite: route_e - one or more points are not in expected aspect (1st check).\n");
		return false;
	}
	
	testsuite_driveTo("seg72", -50, train);
	testsuite_driveToStop("seg73", -20, train);
	// Now at platform7 (B16)
	
	const char *points_normal_c2[3] = {
		"point25", "point21", "point22"
	};
	const char *points_reverse_c2[1] = {
		"point20"
	};
	if (!testsuite_set_and_check_points(points_normal_c2, 3, points_reverse_c2, 1)) {
		printf("testsuite: route_e - one or more points are not in expected aspect (2nd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg57", 50, train);
	testsuite_driveToStop("seg58", 20, train);
	// Now at platform1 (B13) (but not with the same orientation as initially -> so drive 
	// reverser once more)
	
	const char *points_normal_c3[2] = {
		"point22", "point15"
	};
	const char *points_reverse_c3[7] = {
		"point20", "point21", "point16", "point14", "point13", "point12", "point24"
	};
	if (!testsuite_set_and_check_points(points_normal_c3, 2, points_reverse_c3, 7)) {
		printf("testsuite: route_e - one or more points are not in expected aspect (3rd check).\n");
		return false;
	}
	
	testsuite_driveTo("seg63", -50, train);
	testsuite_driveToStop("seg62", -20, train);
	// Now at B14
	
	const char *points_normal_c4[1] = {
		"point23"
	};
	const char *points_reverse_c4[1] = {
		"point22"
	};
	if (!testsuite_set_and_check_points(points_normal_c4, 1, points_reverse_c4, 1)) {
		printf("testsuite: route_e - one or more points are not in expected aspect (4th check).\n");
		return false;
	}
	
	testsuite_driveToStop("seg58", -50, train);
	// Now at platform1 (B13)
	
	return true;
}

void testsuite_case_swtbahnFullCompleteTrackCoverage(const char *train) {
	if (train == NULL) {
		printf("testsuite: swtbahn-full complete track coverage single train - train is NULL\n");
		return;
	}
	
	if (!route_a(train)) {
		printf("testsuite: swtbahn-full complete track coverage single train - route_a failed.\n");
		return;
	}

	if (!route_b(train)) {
		printf("testsuite: swtbahn-full complete track coverage single train - route_b failed.\n");
		return;
	}

	if (!route_c(train)) {
		printf("testsuite: swtbahn-full complete track coverage single train - route_c failed.\n");
		return;
	}

	if (!route_d(train)) {
		printf("testsuite: swtbahn-full complete track coverage single train - route_d failed.\n");
		return;
	}

	if (!route_e(train)) {
		printf("testsuite: swtbahn-full complete track coverage single train - route_e failed.\n");
		return;
	}
}

static bool wrap_drive_and_observe(t_bidib_occ_observer_info *obs_info, 
                                   int speed, const char *train, bool driveToStop, 
                                   const char *target_segment, const char *observe_segment, 
                                   const char *logname) 
{
	pthread_t r_obs_thr;
	prep_observer_segment_info(obs_info, observe_segment);
	pthread_create(&r_obs_thr, NULL, occupancy_observer, (void*) obs_info);

	if (driveToStop) {
		testsuite_driveToStop(target_segment, speed, train);
	} else {
		testsuite_driveTo(target_segment, speed, train);
	}

	return stop_observer_and_check_still_running(obs_info, r_obs_thr, logname);
}

static void *route99(void *arg) {
	const char *train1 = arg;

	if (train1 == NULL || !testsuite_trainReady(train1, "seg58")) {
		pthread_exit(NULL);
	}
	
	// train1: forwards
	
	const char *points_normal[4] = {
		"point23", "point15", "point20", "point19"
	};
	const char *points_reverse[8] = {
		"point22", "point24", "point12", "point13", "point14", "point16", "point21", "point18b"
	};
	
	if (!testsuite_set_and_check_points(points_normal, 4, points_reverse, 8)) {
		printf("testsuite: route99 - one or more points are not in expected aspect.\n");
		pthread_exit(NULL);
	}

	const char* signals_go_1[5] = {"signal30", "signal33", "signal35a", "signal35b", "signal37"};
	testsuite_set_signals_to(signals_go_1, 5, "aspect_go");

	sleep(1);
	
	
	// Note: Most but not all segments the observer is configured to observe
	// for this route are not the immediate successors of the segment in driveTo,
	// but rather the successor of the successor -> to prevent the observer
	// from falsely recognizing a violation due to the combination of sleep 
	// statements (observer sleeps 0.1s, driveTo 0.25s -> fast train could cause issues).
	t_bidib_occ_observer_info *obs_i = malloc(sizeof(t_bidib_occ_observer_info));
	obs_i->check_occ_only = false;
	obs_i->train = strdup(train1);
	obs_i->stop_requested = false;
	obs_i->forbidden_segment = NULL; 
	const char *l_name = "route99";
	
	if (!wrap_drive_and_observe(obs_i, 50, train1, false, "seg57", "seg60", l_name)) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal30", "aspect_stop");

	if (!wrap_drive_and_observe(obs_i, 50, train1, false, "seg64", "seg66", l_name)) {
		pthread_exit(NULL);
	}
	const char* signals_stop_1[3] = {"signal33", "signal35a", "signal35b"};
	testsuite_set_signals_to(signals_stop_1, 3, "aspect_stop");


	if (!wrap_drive_and_observe(obs_i, 50, train1, false, "seg69", "seg40", l_name)) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal37", "aspect_stop");


	if (!wrap_drive_and_observe(obs_i, 40, train1, false, "seg46", "seg47", l_name)) {
		pthread_exit(NULL);
	}
	
	sleep(1);
	// -> last segment before end of that platform - can't observe any overrun.
	testsuite_driveToStop("seg47", 20, train1);
	
	sleep(4);
	if (!bidib_is_running()) {
		printf("testsuite: route99 - stop, bidib is not running anymore\n");
		free_obs_info_util(obs_i);
		pthread_exit(NULL);
	}

	// train1: backwards

	const char* signals_go_2[5] = {"signal26", "signal38", "signal36", "signal34", "signal32"};
	testsuite_set_signals_to(signals_go_2, 5, "aspect_go");
	sleep(1);

	if (!wrap_drive_and_observe(obs_i, -50, train1, false, "seg45", "seg48", l_name)) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal26", "aspect_stop");

	if (!wrap_drive_and_observe(obs_i, -50, train1, false, "seg67", "seg34", l_name)) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal38", "aspect_stop");
	testsuite_set_signal("signal36", "aspect_stop");


	if (!wrap_drive_and_observe(obs_i, -50, train1, false, "seg62", "seg60", l_name)) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal34", "aspect_stop");
	testsuite_set_signal("signal32", "aspect_stop");


	if (!wrap_drive_and_observe(obs_i, -30, train1, false, "seg58", "seg59", l_name)) {
		pthread_exit(NULL);
	}
	
	sleep(2);
	testsuite_driveToStop("seg58", -20, train1);
	sleep(1);
	
	free_obs_info_util(obs_i);
	pthread_exit(NULL);
}

static void *route100(void *arg) {
	const char *train2 = arg;

	if (train2 == NULL || !testsuite_trainReady(train2, "seg78a")) {
		pthread_exit(NULL);
	}
	
	// train2: forwards

	const char *points_normal[5] = { 
		"point9", "point7", "point6", "point5", "point4"
	};
	const char *points_reverse[5] = {
		"point10",  "point8",  "point1",  "point3",  "point11"
	};

	if (!testsuite_set_and_check_points(points_normal, 5, points_reverse, 5)) {
		printf("testsuite: route100 - one or more points are not in expected aspect.\n");
		pthread_exit(NULL);
	}

	const char* signals_go_1[7] = {
		"signal19", "signal3", "signal1", "signal13", "signal11", "signal10", "signal8"
	};
	testsuite_set_signals_to(signals_go_1, 7, "aspect_go");
	testsuite_set_signal("signal43", "aspect_shunt");
	
	sleep(1);
	
	
	static pthread_t route_observer_thread;
	// Note: Most but not all segments the observer is configured to observe
	// for this route are not the immediate successors of the segment in driveTo,
	// but rather the successor of the successor -> to prevent the observer
	// from falsely recognizing a violation due to the combination of sleep 
	// statements (observer sleeps 0.1s, driveTo 0.25s -> fast train could cause issues).
	t_bidib_occ_observer_info *obs1_info = malloc(sizeof(t_bidib_occ_observer_info));
	obs1_info->check_occ_only = false;
	obs1_info->train = strdup(train2);
	obs1_info->stop_requested = false;
	obs1_info->forbidden_segment = strdup("seg28"); 
	
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);
	
	testsuite_driveTo("seg77", 60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal43", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg4");
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);
	
	testsuite_driveTo("seg26", 60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal19", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg18");
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);
	
	testsuite_driveTo("seg1", 60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal1", "aspect_stop");
	testsuite_set_signal("signal3", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg13");
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg15", 60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal13", "aspect_stop");
	testsuite_set_signal("signal11", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg9"); // seg11 -> seg10 -> *seg9*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg11", 60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal10", "aspect_stop");
	testsuite_set_signal("signal8", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg30"); // seg31a -> *seg30*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg31b", 60, train2);
	testsuite_driveTo("seg31b", 40, train2);
	sleep(1);
	testsuite_driveTo("seg31a", 20, train2);
	sleep(1);
	testsuite_driveToStop("seg31a", 20, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}

	sleep(1);

	// train2: backwards

	const char* signals_go_2[8] = {
		"signal22a", "signal22b", "signal9", "signal12", 
		"signal14", "signal2", "signal4a", "signal4b"
	};
	testsuite_set_signals_to(signals_go_2, 8, "aspect_go");
	testsuite_set_signal("signal20", "aspect_shunt");
	
	sleep(1);

	prep_observer_segment_info(obs1_info, "seg9"); // seg32 -> seg33 -> *seg9*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg32", -60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal22a", "aspect_stop");
	testsuite_set_signal("signal22b", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg15"); // seg13 -> seg14 -> *seg15*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg13", -60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal9", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg19"); // seg17 -> seg18 -> *seg19*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg17", -60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal12", "aspect_stop");
	testsuite_set_signal("signal14", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg24"); // seg3 -> seg4 -> *seg24*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg3", -60, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal2", "aspect_stop");
	testsuite_set_signal("signal4a", "aspect_stop");
	testsuite_set_signal("signal4b", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg77"); // seg28 -> seg29 -> *seg77*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);

	testsuite_driveTo("seg28", -50, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	testsuite_set_signal("signal20", "aspect_stop");


	prep_observer_segment_info(obs1_info, "seg78b"); // seg78a -> *seg78b*
	pthread_create(&route_observer_thread, NULL, occupancy_observer, (void*) obs1_info);
	
	testsuite_driveToStop("seg78a", -50, train2);
	if (!stop_observer_and_check_still_running(obs1_info, route_observer_thread, "route100")) {
		pthread_exit(NULL);
	}
	sleep(1);
	
	free_obs_info_util(obs1_info);
	pthread_exit(NULL);
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
	if (!testsuite_trainReady(train, "seg7b")) {
		return false;
	}

	const char *points_normal[7] = { 
		"point2", "point1", "point7", "point6", "point5", "point4", "point3"
	};
	if (!testsuite_set_and_check_points(points_normal, 7, NULL, 0)) {
		printf("testsuite: route_custom_short - one or more points are not in expected aspect.\n");
		return false;
	}

	testsuite_driveTo("seg7a", 50, train);
	testsuite_driveToStop("seg7b", 50, train);
	return true;
}

void testsuite_case_swtbahnFullShortRoute(const char *train) {
	if (!route_custom_short(train)) {
		return;
	}
}
