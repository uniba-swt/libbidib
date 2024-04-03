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
 * - Bernhard Luedtke <https://github.com/BLuedtke>
 *
 */
 #include "kinderuni_suite.h"

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>



bool ku_scenario1_initial(const char *train1) {
	if (!testsuite_trainReady(train1, "seg22")) {
		return false;
	}
	testsuite_set_signal("signal18a", "aspect_stop");
	testsuite_set_signal("signal18b", "aspect_stop");
	
	testsuite_set_signal("signal19", "aspect_stop");
	testsuite_set_signal("signal21", "aspect_stop");
	testsuite_set_signal("signal43", "aspect_stop");
	
	testsuite_switch_point("point8", "normal");
	testsuite_switch_point("point9", "normal");
	testsuite_switch_point("point10", "reverse");
	
	sleep(2);
	return true;
}

bool ku_scenario1_aktion(const char *train1) {
	testsuite_set_signal("signal18a", "aspect_go");
	sleep(2);
	testsuite_driveTo("seg78a", 40, train1);
	testsuite_driveToStop("seg78b", 20, train1);
	return true;
}

bool ku_scenario1_reset(const char *train1) {
	if (!testsuite_trainReady(train1, "seg78b") && !testsuite_trainReady(train1, "seg78a")) {
		return false;
	}
	
	testsuite_switch_point("point10", "reverse");
	sleep(1);
	testsuite_set_signal("signal19", "aspect_go");
	testsuite_set_signal("signal43", "aspect_shunt");
	sleep(1);
	testsuite_driveTo("seg23", -40, train1);
	testsuite_driveToStop("seg22", -20, train1);
	
	if (testsuite_trainReady(train1, "seg23")) {
		bidib_set_train_speed(train1, -20, "master");
		bidib_flush();
		sleep(1);
		bidib_set_train_speed(train1, 0, "master");
		bidib_flush();
	}
	
	testsuite_set_signal("signal19", "aspect_stop");
	testsuite_set_signal("signal43", "aspect_stop");
	
	if (!testsuite_trainReady(train1, "seg22")) {
		return false;
	}
	return true;
}



bool ku_scenario2_initial(const char *train1, const char *train2) {
	if (!testsuite_trainReady(train1, "seg22")) {
		return false;
	}
	if (!testsuite_trainReady(train2, "seg31a")) {
		return false;
	}
	testsuite_set_signal("signal18a", "aspect_go"); // intentional
	testsuite_set_signal("signal18b", "aspect_stop");
	
	testsuite_set_signal("signal19", "aspect_stop");
	testsuite_set_signal("signal21", "aspect_shunt");
	testsuite_set_signal("signal43", "aspect_stop");
	
	testsuite_switch_point("point8", "normal");
	testsuite_switch_point("point9", "normal");
	testsuite_switch_point("point10", "reverse");
	
	sleep(2);
	return true;
}

bool ku_scenario2_aktion(const char *train1, const char *train2) {
	testsuite_set_signal("signal21", "aspect_stop");
	sleep(2);
	testsuite_driveTo("seg78a", 40, train1);
	testsuite_driveToStop("seg78b", 20, train1);
	sleep(1);
	testsuite_switch_point("point10", "normal");
	sleep(2);
	testsuite_set_signal("signal19", "aspect_go");
	testsuite_set_signal("signal21", "aspect_shunt");
	sleep(2);
	testsuite_driveTo("seg22", -40, train2);
	testsuite_set_signal("signal21", "aspect_stop");
	testsuite_set_signal("signal19", "aspect_stop");
	sleep(2);
	bidib_set_train_speed(train2, 0, "master");
	bidib_flush();
	bidib_set_train_speed(train2, 10, "master");
	bidib_flush();
	bidib_set_train_speed(train2, 0, "master");
	bidib_flush();
	return true;
}

bool ku_scenario2_reset(const char *train1, const char *train2) {
	if (!testsuite_trainReady(train1, "seg78b") && !testsuite_trainReady(train1, "seg78a")) {
		return false;
	}
	if (!testsuite_trainReady(train2, "seg22")) {
		return false;
	}
	
	testsuite_switch_point("point10", "reverse");
	sleep(1);
	testsuite_set_signal("signal19", "aspect_go");
	testsuite_set_signal("signal43", "aspect_shunt");
	sleep(1);
	testsuite_driveTo("seg23", -40, train1);
	testsuite_driveToStop("seg22", -30, train1);
	if (testsuite_trainReady(train1, "seg23")) {
		bidib_set_train_speed(train1, -20, "master");
		bidib_flush();
		sleep(1);
		bidib_set_train_speed(train1, 0, "master");
		bidib_flush();
	}
	bidib_set_train_speed(train1, 10, "master");
	bidib_flush();
	bidib_set_train_speed(train1, 0, "master");
	bidib_flush();
	testsuite_set_signal("signal19", "aspect_stop");
	testsuite_set_signal("signal43", "aspect_stop");
	
	if (!testsuite_trainReady(train1, "seg22") || !testsuite_trainReady(train2, "seg22")) {
		return false;
	}
	return true;
}



bool ku_scenario3_initial(const char *train1, const char *train2) {
	if (!testsuite_trainReady(train1, "seg22")) {
		return false;
	}
	if (!testsuite_trainReady(train2, "seg22")) {
		return false;
	}
	testsuite_set_signal("signal18a", "aspect_stop");
	testsuite_set_signal("signal18b", "aspect_stop");
	
	testsuite_set_signal("signal19", "aspect_stop");
	testsuite_set_signal("signal21", "aspect_stop");
	testsuite_set_signal("signal43", "aspect_stop");
	
	testsuite_switch_point("point8", "normal");
	testsuite_switch_point("point9", "normal");
	testsuite_switch_point("point10", "reverse");
	
	sleep(2);
	return true;
}

bool ku_scenario3_aktion(const char *train1, const char *train2) {
	testsuite_set_signal("signal18a", "aspect_go");
	sleep(2);
	testsuite_driveTo("seg23", 40, train1);
	
	testsuite_driveTo("seg22", 40, train2);
	sleep(1);
	
	testsuite_set_signal("signal18a", "aspect_stop");
	
	testsuite_driveToStop("seg23", 10, train2);
	
	testsuite_driveTo("seg78a", 40, train1);
	testsuite_driveToStop("seg78b", 20, train1);
	
	sleep(1);
	return true;
}

bool ku_scenario3_reset(const char *train1, const char *train2) {
	if (!testsuite_trainReady(train1, "seg78b") && !testsuite_trainReady(train1, "seg78a")) {
		return false;
	}
	if (!testsuite_trainReady(train2, "seg22") && !testsuite_trainReady(train2, "seg23")) {
		return false;
	}
	testsuite_set_signal("signal18a", "aspect_go");
	testsuite_driveToStop("seg26", 40, train2);
	testsuite_set_signal("signal18a", "aspect_stop");
	testsuite_switch_point("point8", "reverse");
	testsuite_switch_point("point1", "reverse");
	sleep(2);
	testsuite_driveToStop("seg2", -40, train2);
	bidib_set_train_speed(train2, 10, "master");
	bidib_flush();
	bidib_set_train_speed(train2, 0, "master");
	bidib_flush();
	sleep(1);
	
	testsuite_switch_point("point8", "normal");
	testsuite_set_signal("signal19", "aspect_go");
	testsuite_set_signal("signal43", "aspect_shunt");
	sleep(1);
	testsuite_driveTo("seg23", -40, train1);
	testsuite_driveToStop("seg22", -20, train1);
	if (testsuite_trainReady(train1, "seg23")) {
		bidib_set_train_speed(train1, -20, "master");
		bidib_flush();
		sleep(1);
		bidib_set_train_speed(train1, 0, "master");
		bidib_flush();
	}
	bidib_set_train_speed(train1, 10, "master");
	bidib_flush();
	bidib_set_train_speed(train1, 0, "master");
	bidib_flush();
	testsuite_set_signal("signal19", "aspect_stop");
	testsuite_set_signal("signal43", "aspect_stop");
	
	if (!testsuite_trainReady(train1, "seg22") || !testsuite_trainReady(train2, "seg2")) {
		return false;
	}
	return true;
}







bool ku_scenario4_initial(const char *train1, const char *train2) {
	if (!testsuite_trainReady(train1, "seg22")) {
		return false;
	}
	if (!testsuite_trainReady(train2, "seg2")) {
		return false;
	}
	testsuite_set_signal("signal18a", "aspect_go"); // intentional
	testsuite_set_signal("signal18b", "aspect_stop");
	testsuite_set_signal("signal4a", "aspect_go"); // intentional
	testsuite_set_signal("signal4b", "aspect_stop");
	
	testsuite_set_signal("signal5", "aspect_stop");
	testsuite_set_signal("signal19", "aspect_stop");
	testsuite_set_signal("signal21", "aspect_stop");
	testsuite_set_signal("signal43", "aspect_stop");
	testsuite_set_signal("signal51", "aspect_stop");
	
	testsuite_switch_point("point1", "reverse");
	testsuite_switch_point("point8", "normal");
	testsuite_switch_point("point9", "normal");
	testsuite_switch_point("point10", "reverse");
	
	sleep(2);
	return true;
}

bool ku_scenario4_aktion(const char *train1, const char *train2) {
	testsuite_set_signal("signal4a", "aspect_stop");
	sleep(1);
	testsuite_driveTo("seg27", 30, train1);
	testsuite_set_signal("signal18a", "aspect_stop");
	sleep(2);
	testsuite_set_signal("signal4a", "aspect_go");
	sleep(1);
	testsuite_driveTo("seg24", 40, train2);
	testsuite_driveTo("seg78a", 40, train1);
	testsuite_driveToStop("seg78b", 20, train1);
	testsuite_driveToStop("seg94a", 30, train2);
	return true;
}

bool ku_scenario4_reset(const char *train1, const char *train2) {
	if (!testsuite_trainReady(train1, "seg78b") && !testsuite_trainReady(train1, "seg79")) {
		return false;
	}
	if (!testsuite_trainReady(train2, "seg94a")) {
		return false;
	}
	testsuite_set_signal("signal43", "aspect_shunt");
	testsuite_set_signal("signal51", "aspect_go");
	sleep(1);
	testsuite_driveTo("seg77", -40, train1);
	bidib_set_train_speed(train1, -25, "master");
	bidib_flush();
	testsuite_driveTo("seg4", -50, train2);
	testsuite_set_signal("signal19", "aspect_go");
	testsuite_set_signal("signal51", "aspect_stop");
	testsuite_set_signal("signal43", "aspect_stop");
	
	testsuite_driveToStop("seg2", -40, train2);
	testsuite_driveToStop("seg22", -40, train1);
	
	bidib_set_train_speed(train1, 10, "master");
	bidib_set_train_speed(train2, 10, "master");
	bidib_flush();
	bidib_set_train_speed(train1, 0, "master");
	bidib_set_train_speed(train2, 0, "master");
	bidib_flush();
	testsuite_set_signal("signal19", "aspect_stop");
	
	if (!testsuite_trainReady(train1, "seg22") || !testsuite_trainReady(train2, "seg2")) {
		return false;
	}
	return true;
}


char read_char() {
	char inputChar;
	ssize_t bytesRead = read(STDIN_FILENO, &inputChar, 1);
	if (bytesRead == 1) {
		return inputChar;
	} else {
		return '-';
	}
}

bool ku_scenario1_interactive(const char *train1) {
	// init
	printf("Scenario 1 - Interactive. Will now initialise.\n");
	bool init_res = ku_scenario1_initial(train1);
	if (!init_res) {
		printf("Initialisation failed.\n");
		return false;
	}
	
	// ask for action.
	// - signal, point toggle -> always available
	// - train driving: once only, preset target.
	// - reset
	
	
	// - once goal reached, reset -> either to same config or to next.
}
