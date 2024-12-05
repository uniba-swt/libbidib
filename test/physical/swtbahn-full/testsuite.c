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

bool route1(const char *train) {
	if (!testsuite_trainReady(train, "seg58")) {
		return false;
	}

	testsuite_switch_point("point22", "reverse");
	testsuite_switch_point("point23", "normal");
	testsuite_switch_point("point24", "reverse");
	testsuite_switch_point("point12", "reverse");
	testsuite_switch_point("point13", "reverse");
	testsuite_switch_point("point14", "reverse");
	testsuite_switch_point("point15", "normal");
	testsuite_switch_point("point16", "reverse");
	testsuite_switch_point("point21", "reverse");
	testsuite_switch_point("point20", "normal");
	testsuite_switch_point("point19", "normal");
	testsuite_switch_point("point18b", "reverse");
	
	sleep(2);
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point22", "reverse");
	point_check &= testsuite_check_point_aspect("point23", "normal");
	point_check &= testsuite_check_point_aspect("point24", "reverse");
	point_check &= testsuite_check_point_aspect("point12", "reverse");
	point_check &= testsuite_check_point_aspect("point13", "reverse");
	point_check &= testsuite_check_point_aspect("point14", "reverse");
	point_check &= testsuite_check_point_aspect("point15", "normal");
	point_check &= testsuite_check_point_aspect("point16", "reverse");
	point_check &= testsuite_check_point_aspect("point21", "reverse");
	point_check &= testsuite_check_point_aspect("point20", "normal");
	point_check &= testsuite_check_point_aspect("point19", "normal");
	point_check &= testsuite_check_point_aspect("point18b", "reverse");
	if (!point_check) {
		printf("testsuite: route1 - one or more points are not in expected aspect.");
		return false;
	}
	

	testsuite_set_signal("signal30", "aspect_go");
	testsuite_set_signal("signal33", "aspect_go");
	testsuite_set_signal("signal35a", "aspect_go");
	testsuite_set_signal("signal35b", "aspect_go");
	testsuite_set_signal("signal37", "aspect_go");

	testsuite_driveTo("seg57", 50, train);
	testsuite_set_signal("signal30", "aspect_stop");

	testsuite_driveTo("seg64", 50, train);
	testsuite_set_signal("signal33", "aspect_stop");
	testsuite_set_signal("signal35a", "aspect_stop");
	testsuite_set_signal("signal35b", "aspect_stop");

	testsuite_driveTo("seg69", 50, train);
	testsuite_set_signal("signal37", "aspect_stop");

	testsuite_driveTo("seg46", 50, train);
	testsuite_driveToStop("seg47", 20, train);

	// Drive backwards through the route
	testsuite_set_signal("signal26", "aspect_go");
	testsuite_set_signal("signal38", "aspect_go");
	testsuite_set_signal("signal36", "aspect_go");
	testsuite_set_signal("signal34", "aspect_go");
	testsuite_set_signal("signal32", "aspect_go");

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

bool route2(const char *train) {
	if (!testsuite_trainReady(train, "seg58")) {
		return false;
	}

	testsuite_switch_point("point22", "normal");
	testsuite_switch_point("point20", "reverse");
	testsuite_switch_point("point21", "reverse");
	testsuite_switch_point("point16", "reverse");
	testsuite_switch_point("point15", "reverse");
	testsuite_switch_point("point5", "reverse");
	testsuite_switch_point("point4", "reverse");
	testsuite_switch_point("point12", "reverse");
	testsuite_switch_point("point11", "reverse");
	testsuite_switch_point("point27", "reverse");
	testsuite_switch_point("point29", "reverse");
	testsuite_switch_point("point28", "reverse");
	testsuite_switch_point("point26", "normal");
	testsuite_switch_point("point9", "reverse");
	testsuite_switch_point("point8", "reverse");
	testsuite_switch_point("point1", "reverse");
	testsuite_switch_point("point7", "normal");
	testsuite_switch_point("point6", "reverse");
	testsuite_switch_point("point17", "reverse");
	
	sleep(2);
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point22", "normal");
	point_check &= testsuite_check_point_aspect("point20", "reverse");
	point_check &= testsuite_check_point_aspect("point21", "reverse");
	point_check &= testsuite_check_point_aspect("point16", "reverse");
	point_check &= testsuite_check_point_aspect("point15", "reverse");
	point_check &= testsuite_check_point_aspect("point5", "reverse");
	point_check &= testsuite_check_point_aspect("point4", "reverse");
	point_check &= testsuite_check_point_aspect("point12", "reverse");
	point_check &= testsuite_check_point_aspect("point11", "reverse");
	point_check &= testsuite_check_point_aspect("point27", "reverse");
	point_check &= testsuite_check_point_aspect("point29", "reverse");
	point_check &= testsuite_check_point_aspect("point28", "reverse");
	point_check &= testsuite_check_point_aspect("point26", "normal");
	point_check &= testsuite_check_point_aspect("point9", "reverse");
	point_check &= testsuite_check_point_aspect("point8", "reverse");
	point_check &= testsuite_check_point_aspect("point1", "reverse");
	point_check &= testsuite_check_point_aspect("point7", "normal");
	point_check &= testsuite_check_point_aspect("point6", "reverse");
	point_check &= testsuite_check_point_aspect("point17", "reverse");
	if (!point_check) {
		printf("testsuite: route2 - one or more points are not in expected aspect (1st check).");
		return false;
	}

	testsuite_driveToStop("seg42b", 50, train);
	
	testsuite_switch_point("point16", "normal");
	testsuite_switch_point("point15", "normal");
	testsuite_switch_point("point14", "reverse");
	testsuite_switch_point("point13", "reverse");
	testsuite_switch_point("point12", "reverse");
	testsuite_switch_point("point24", "reverse");
	testsuite_switch_point("point23", "reverse");
	testsuite_switch_point("point19", "reverse");
	testsuite_switch_point("point18b", "normal");
	testsuite_switch_point("point18a", "reverse");
	testsuite_switch_point("point8", "normal");
	testsuite_switch_point("point9", "reverse");
	testsuite_switch_point("point26", "reverse");
	testsuite_switch_point("point27", "normal");
	testsuite_switch_point("point11", "normal");
	testsuite_switch_point("point3", "reverse");
	testsuite_switch_point("point4", "normal");
	testsuite_switch_point("point5", "normal");
	
	sleep(2);
	// Check that points are in desired position
	point_check &= testsuite_check_point_aspect("point16", "normal");
	point_check &= testsuite_check_point_aspect("point15", "normal");
	point_check &= testsuite_check_point_aspect("point14", "reverse");
	point_check &= testsuite_check_point_aspect("point13", "reverse");
	point_check &= testsuite_check_point_aspect("point12", "reverse");
	point_check &= testsuite_check_point_aspect("point24", "reverse");
	point_check &= testsuite_check_point_aspect("point23", "reverse");
	point_check &= testsuite_check_point_aspect("point19", "reverse");
	point_check &= testsuite_check_point_aspect("point18b", "normal");
	point_check &= testsuite_check_point_aspect("point18a", "reverse");
	point_check &= testsuite_check_point_aspect("point8", "normal");
	point_check &= testsuite_check_point_aspect("point9", "reverse");
	point_check &= testsuite_check_point_aspect("point26", "reverse");
	point_check &= testsuite_check_point_aspect("point27", "normal");
	point_check &= testsuite_check_point_aspect("point11", "normal");
	point_check &= testsuite_check_point_aspect("point3", "reverse");
	point_check &= testsuite_check_point_aspect("point4", "normal");
	point_check &= testsuite_check_point_aspect("point5", "normal");
	if (!point_check) {
		printf("testsuite: route2 - one or more points are not in expected aspect (2nd check).");
		return false;
	}

	testsuite_driveToStop("seg69", 50, train);
	
	testsuite_switch_point("point6", "normal");
	testsuite_switch_point("point7", "reverse");
	sleep(2);
	// Check that points are in desired position
	point_check &= testsuite_check_point_aspect("point6", "normal");
	point_check &= testsuite_check_point_aspect("point7", "reverse");
	if (!point_check) {
		printf("testsuite: route2 - one or more points are not in expected aspect (3rd check).");
		return false;
	}

	testsuite_driveTo("seg46", 50, train);
	testsuite_driveToStop("seg47", 20, train);
	return true;
}

bool route3(const char *train) {
	if (!testsuite_trainReady(train, "seg46")) {
		return false;
	}

	testsuite_switch_point("point18b", "reverse");
	testsuite_switch_point("point19", "reverse");
	testsuite_switch_point("point23", "reverse");
	testsuite_switch_point("point24", "reverse");
	testsuite_switch_point("point12", "normal");
	testsuite_switch_point("point4", "reverse");
	testsuite_switch_point("point5", "reverse");
	testsuite_switch_point("point15", "reverse");
	testsuite_switch_point("point16", "normal");
	testsuite_switch_point("point17", "reverse");
	testsuite_switch_point("point6", "reverse");
	testsuite_switch_point("point7", "normal");
	testsuite_switch_point("point1", "reverse");
	testsuite_switch_point("point8", "reverse");
	testsuite_switch_point("point9", "normal");
	testsuite_switch_point("point10", "reverse");
	
	sleep(2);
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point18b", "reverse");
	point_check &= testsuite_check_point_aspect("point19", "reverse");
	point_check &= testsuite_check_point_aspect("point23", "reverse");
	point_check &= testsuite_check_point_aspect("point24", "reverse");
	point_check &= testsuite_check_point_aspect("point12", "normal");
	point_check &= testsuite_check_point_aspect("point4", "reverse");
	point_check &= testsuite_check_point_aspect("point5", "reverse");
	point_check &= testsuite_check_point_aspect("point15", "reverse");
	point_check &= testsuite_check_point_aspect("point16", "normal");
	point_check &= testsuite_check_point_aspect("point17", "reverse");
	point_check &= testsuite_check_point_aspect("point6", "reverse");
	point_check &= testsuite_check_point_aspect("point7", "normal");
	point_check &= testsuite_check_point_aspect("point1", "reverse");
	point_check &= testsuite_check_point_aspect("point8", "reverse");
	point_check &= testsuite_check_point_aspect("point9", "normal");
	point_check &= testsuite_check_point_aspect("point10", "reverse");
	if (!point_check) {
		printf("testsuite: route3 - one or more points are not in expected aspect.");
		return false;
	}

	testsuite_driveTo("seg29", -50, train);
	testsuite_driveToStop("seg78b", -20, train);

	return true;
}

bool route4(const char *train) {
	if (!testsuite_trainReady(train, "seg78a")) {
		return false;
	}

	testsuite_switch_point("point10", "reverse");
	testsuite_switch_point("point9", "normal");
	testsuite_switch_point("point8", "reverse");
	testsuite_switch_point("point1", "reverse");
	testsuite_switch_point("point7", "normal");
	testsuite_switch_point("point6", "reverse");
	testsuite_switch_point("point17", "reverse");
	testsuite_switch_point("point16", "normal");
	testsuite_switch_point("point15", "normal");
	testsuite_switch_point("point14", "reverse");
	testsuite_switch_point("point13", "reverse");
	testsuite_switch_point("point12", "reverse");
	testsuite_switch_point("point24", "reverse");
	testsuite_switch_point("point23", "normal");
	testsuite_switch_point("point22", "reverse");
	
	sleep(2);
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point10", "reverse");
	point_check &= testsuite_check_point_aspect("point9", "normal");
	point_check &= testsuite_check_point_aspect("point8", "reverse");
	point_check &= testsuite_check_point_aspect("point1", "reverse");
	point_check &= testsuite_check_point_aspect("point7", "normal");
	point_check &= testsuite_check_point_aspect("point6", "reverse");
	point_check &= testsuite_check_point_aspect("point17", "reverse");
	point_check &= testsuite_check_point_aspect("point16", "normal");
	point_check &= testsuite_check_point_aspect("point15", "normal");
	point_check &= testsuite_check_point_aspect("point14", "reverse");
	point_check &= testsuite_check_point_aspect("point13", "reverse");
	point_check &= testsuite_check_point_aspect("point12", "reverse");
	point_check &= testsuite_check_point_aspect("point24", "reverse");
	point_check &= testsuite_check_point_aspect("point23", "normal");
	point_check &= testsuite_check_point_aspect("point22", "reverse");
	if (!point_check) {
		printf("testsuite: route4 - one or more points are not in expected aspect.");
		return false;
	}

	testsuite_driveTo("seg58", 50, train);
	testsuite_driveToStop("seg59", 20, train);

	return true;
}

bool route5(const char *train) {
	if (!testsuite_trainReady(train, "seg58")) {
		return false;
	}
	
	testsuite_switch_point("point22", "reverse");
	testsuite_switch_point("point23", "normal");
	testsuite_switch_point("point24", "reverse");
	testsuite_switch_point("point12", "reverse");
	testsuite_switch_point("point13", "reverse");
	testsuite_switch_point("point14", "reverse");
	testsuite_switch_point("point15", "normal");
	testsuite_switch_point("point16", "reverse");
	testsuite_switch_point("point21", "reverse");
	testsuite_switch_point("point20", "reverse");
	
	sleep(2);
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point22", "reverse");
	point_check &= testsuite_check_point_aspect("point23", "normal");
	point_check &= testsuite_check_point_aspect("point24", "reverse");
	point_check &= testsuite_check_point_aspect("point12", "reverse");
	point_check &= testsuite_check_point_aspect("point13", "reverse");
	point_check &= testsuite_check_point_aspect("point14", "reverse");
	point_check &= testsuite_check_point_aspect("point15", "normal");
	point_check &= testsuite_check_point_aspect("point16", "reverse");
	point_check &= testsuite_check_point_aspect("point21", "reverse");
	point_check &= testsuite_check_point_aspect("point20", "reverse");
	if (!point_check) {
		printf("testsuite: route5 - one or more points are not in expected aspect (1st check).");
		return false;
	}
	
	testsuite_driveToStop("seg64", -50, train);
	testsuite_switch_point("point22", "normal");
	sleep(2);
	// Check that point is in desired position
	if (!testsuite_check_point_aspect("point21", "reverse")) {
		printf("testsuite: route5 - point 21 is not in expected aspect (2nd check).");
		return false;
	}
	
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

	// train1: forwards
	testsuite_switch_point("point22", "reverse");
	testsuite_switch_point("point23", "normal");
	testsuite_switch_point("point24", "reverse");
	testsuite_switch_point("point12", "reverse");
	testsuite_switch_point("point13", "reverse");
	testsuite_switch_point("point14", "reverse");
	testsuite_switch_point("point15", "normal");
	testsuite_switch_point("point16", "reverse");
	testsuite_switch_point("point21", "reverse");
	testsuite_switch_point("point20", "normal");
	testsuite_switch_point("point19", "normal");
	testsuite_switch_point("point18b", "reverse");
	
	sleep(1);

	testsuite_set_signal("signal30", "aspect_go");
	testsuite_set_signal("signal33", "aspect_go");
	testsuite_set_signal("signal35a", "aspect_go");
	testsuite_set_signal("signal35b", "aspect_go");
	testsuite_set_signal("signal37", "aspect_go");
	
	sleep(1);
	
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point22", "reverse");
	point_check &= testsuite_check_point_aspect("point23", "normal");
	point_check &= testsuite_check_point_aspect("point24", "reverse");
	point_check &= testsuite_check_point_aspect("point12", "reverse");
	point_check &= testsuite_check_point_aspect("point13", "reverse");
	point_check &= testsuite_check_point_aspect("point14", "reverse");
	point_check &= testsuite_check_point_aspect("point15", "normal");
	point_check &= testsuite_check_point_aspect("point16", "reverse");
	point_check &= testsuite_check_point_aspect("point21", "reverse");
	point_check &= testsuite_check_point_aspect("point20", "normal");
	point_check &= testsuite_check_point_aspect("point19", "normal");
	point_check &= testsuite_check_point_aspect("point18b", "reverse");
	if (!point_check) {
		printf("testsuite: route99 - one or more points are not in expected aspect.");
		pthread_exit(NULL);
	}

	testsuite_driveTo("seg57", 50, train1);
	testsuite_set_signal("signal30", "aspect_stop");

	testsuite_driveTo("seg64", 50, train1);
	testsuite_set_signal("signal33", "aspect_stop");
	testsuite_set_signal("signal35a", "aspect_stop");
	testsuite_set_signal("signal35b", "aspect_stop");

	testsuite_driveTo("seg69", 50, train1);
	testsuite_set_signal("signal37", "aspect_stop");

	testsuite_driveTo("seg46", 50, train1);
	sleep(1);
	testsuite_driveToStop("seg47", 20, train1);
	
	sleep(4);

	// train1: backwards
	testsuite_set_signal("signal26", "aspect_go");
	testsuite_set_signal("signal38", "aspect_go");
	testsuite_set_signal("signal36", "aspect_go");
	testsuite_set_signal("signal34", "aspect_go");
	testsuite_set_signal("signal32", "aspect_go");

	sleep(1);

	testsuite_driveTo("seg45", -50, train1);
	testsuite_set_signal("signal26", "aspect_stop");

	testsuite_driveTo("seg67", -50, train1);
	testsuite_set_signal("signal38", "aspect_stop");
	testsuite_set_signal("signal36", "aspect_stop");

	testsuite_driveTo("seg62", -50, train1);
	testsuite_set_signal("signal34", "aspect_stop");
	testsuite_set_signal("signal32", "aspect_stop");

	testsuite_driveTo("seg60", -50, train1);
	testsuite_driveTo("seg53", -40, train1);
	testsuite_driveTo("seg58", -30, train1);
	sleep(2);
	testsuite_driveToStop("seg58", -20, train1);
	sleep(2);
	pthread_exit(NULL);
}

static void *route100(void *arg) {
	const char *train2 = arg;

	if (!testsuite_trainReady(train2, "seg78a")) {
		pthread_exit(NULL);
	}
	
	// train2: forwards
	testsuite_switch_point("point10", "reverse");
	testsuite_switch_point("point9", "normal");
	testsuite_switch_point("point8", "reverse");
	testsuite_switch_point("point1", "reverse");
	testsuite_switch_point("point7", "normal");
	testsuite_switch_point("point6", "normal");
	testsuite_switch_point("point5", "normal");
	testsuite_switch_point("point4", "normal");
	testsuite_switch_point("point3", "reverse");
	testsuite_switch_point("point11", "reverse");
	
	sleep(1);

	testsuite_set_signal("signal43", "aspect_shunt");
	testsuite_set_signal("signal19", "aspect_go");
	testsuite_set_signal("signal3", "aspect_go");
	testsuite_set_signal("signal1", "aspect_go");
	testsuite_set_signal("signal13", "aspect_go");
	testsuite_set_signal("signal11", "aspect_go");
	testsuite_set_signal("signal10", "aspect_go");
	testsuite_set_signal("signal8", "aspect_go");
	
	sleep(1);
	
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point10", "reverse");
	point_check &= testsuite_check_point_aspect("point9", "normal");
	point_check &= testsuite_check_point_aspect("point8", "reverse");
	point_check &= testsuite_check_point_aspect("point1", "reverse");
	point_check &= testsuite_check_point_aspect("point7", "normal");
	point_check &= testsuite_check_point_aspect("point6", "normal");
	point_check &= testsuite_check_point_aspect("point5", "normal");
	point_check &= testsuite_check_point_aspect("point4", "normal");
	point_check &= testsuite_check_point_aspect("point3", "reverse");
	point_check &= testsuite_check_point_aspect("point11", "reverse");
	if (!point_check) {
		printf("testsuite: route100 - one or more points are not in expected aspect.");
		pthread_exit(NULL);
	}
	
	testsuite_driveTo("seg77", 60, train2);
	testsuite_set_signal("signal43", "aspect_stop");
	
	testsuite_driveTo("seg26", 60, train2);
	testsuite_set_signal("signal19", "aspect_stop");
	
	testsuite_driveTo("seg1", 60, train2);
	testsuite_set_signal("signal3", "aspect_stop");
	testsuite_set_signal("signal1", "aspect_stop");
	
	testsuite_driveTo("seg15", 60, train2);
	testsuite_set_signal("signal13", "aspect_stop");
	testsuite_set_signal("signal11", "aspect_stop");
	
	testsuite_driveTo("seg11", 60, train2);
	testsuite_set_signal("signal10", "aspect_stop");
	testsuite_set_signal("signal8", "aspect_stop");
	
	testsuite_driveTo("seg31b", 60, train2);
	testsuite_driveTo("seg31b", 40, train2);
	sleep(1);
	testsuite_driveTo("seg31a", 30, train2);
	sleep(1);
	testsuite_driveToStop("seg31a", 20, train2);
	
	sleep(3);

	// train2: backwards
	testsuite_set_signal("signal22a", "aspect_go");
	testsuite_set_signal("signal22b", "aspect_go");
	testsuite_set_signal("signal9", "aspect_go");
	testsuite_set_signal("signal12", "aspect_go");
	testsuite_set_signal("signal14", "aspect_go");
	testsuite_set_signal("signal2", "aspect_go");
	testsuite_set_signal("signal4a", "aspect_go");
	testsuite_set_signal("signal4b", "aspect_go");
	testsuite_set_signal("signal20", "aspect_shunt");
	
	sleep(1);

	testsuite_driveTo("seg32", -60, train2);
	testsuite_set_signal("signal22a", "aspect_stop");
	testsuite_set_signal("signal22b", "aspect_stop");
	
	testsuite_driveTo("seg13", -60, train2);
	testsuite_set_signal("signal9", "aspect_stop");
	
	testsuite_driveTo("seg17", -60, train2);
	testsuite_set_signal("signal12", "aspect_stop");
	testsuite_set_signal("signal14", "aspect_stop");
	
	testsuite_driveTo("seg3", -60, train2);
	testsuite_set_signal("signal2", "aspect_stop");
	testsuite_set_signal("signal4a", "aspect_stop");
	testsuite_set_signal("signal4b", "aspect_stop");
	
	testsuite_driveTo("seg28", -50, train2);
	testsuite_set_signal("signal20", "aspect_stop");
	
	testsuite_driveTo("seg78a", -50, train2);
	sleep(2);
	testsuite_driveToStop("seg78a", -20, train2);
	sleep(2);
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

	testsuite_switch_point("point2", "normal");
	testsuite_switch_point("point1", "normal");
	testsuite_switch_point("point7", "normal");
	testsuite_switch_point("point6", "normal");
	testsuite_switch_point("point5", "normal");
	testsuite_switch_point("point4", "normal");
	testsuite_switch_point("point3", "normal");
	
	sleep(2);
	// Check that points are in desired position
	bool point_check = true;
	point_check &= testsuite_check_point_aspect("point2", "normal");
	point_check &= testsuite_check_point_aspect("point1", "normal");
	point_check &= testsuite_check_point_aspect("point7", "normal");
	point_check &= testsuite_check_point_aspect("point6", "normal");
	point_check &= testsuite_check_point_aspect("point5", "normal");
	point_check &= testsuite_check_point_aspect("point4", "normal");
	point_check &= testsuite_check_point_aspect("point3", "normal");
	if (!point_check) {
		printf("testsuite: route_custom_short - one or more points are not in expected aspect.");
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
