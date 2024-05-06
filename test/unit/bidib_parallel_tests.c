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
 * - Eugene Yip <https://github.com/eyip002>
 *
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "../../include/bidib.h"
#include "../../src/transmission/bidib_transmission_intern.h"
#include "../../src/state/bidib_state_intern.h"
#include "../../src/state/bidib_state_getter_intern.h"

static uint8_t output_buffer[128];
static unsigned int output_index = 0;

const int TEST_CASE_DURATION = 30;      // 30 seconds
const int BIDIB_ACTION_DELAY = 100000;  // 100 milliseconds 

static uint8_t read_byte(int *read_byte) {
	*read_byte = 0;
	return 0x00;
}

static void write_byte(uint8_t msg_byte) {
	output_buffer[output_index] = msg_byte;
	output_index++;
}

static void write_bytes(uint8_t* msg, int32_t len) {
	if (msg != NULL && len > 0) {
		for (int32_t i = 0; i < len; ++i) {
			output_buffer[output_index] = msg[i];
			output_index++;
		}
	}
}

static void set_all_boards_and_trains_connected(void) {
	pthread_rwlock_wrlock(&bidib_state_boards_rwlock);
	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (board_i != NULL) {
			board_i->connected = true;
		}
	}
	pthread_rwlock_unlock(&bidib_state_boards_rwlock);
	pthread_rwlock_wrlock(&bidib_state_track_rwlock);
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref("train1");
	if (train_state != NULL) {
		train_state->on_track = true;
	}
	train_state = bidib_state_get_train_state_ref("train2");
	if (train_state != NULL) {
		train_state->on_track = true;
	}
	pthread_rwlock_unlock(&bidib_state_track_rwlock);
}

static void set_board_point(const char *point, const char *aspect) {
	const int err = bidib_switch_point(point, aspect);
	assert_int_equal(err, 0);
	
	const t_bidib_unified_accessory_state_query query = bidib_get_point_state(point);
	assert_true(query.known);
	bidib_free_unified_accessory_state_query(query);
}

static void *loop_set_board_point(void *arg) {
	bool *run_test_case = (bool *)arg;
	do {
		set_board_point("point1", "normal");
		usleep(BIDIB_ACTION_DELAY);
		set_board_point("point1", "reverse");
		usleep(BIDIB_ACTION_DELAY);
	} while (*run_test_case);
	pthread_exit(NULL);
}

static void set_dcc_point(const char *point, const char *aspect) {
	const int err = bidib_switch_point(point, aspect);
	assert_int_equal(err, 0);
	
	const t_bidib_unified_accessory_state_query query = bidib_get_point_state(point);
	assert_true(query.known);
	bidib_free_unified_accessory_state_query(query);
}

static void *loop_set_dcc_point(void *arg) {
	bool *run_test_case = (bool *)arg;
	do {
		set_dcc_point("point2", "normal");
		usleep(BIDIB_ACTION_DELAY);
		set_dcc_point("point2", "reverse");
		usleep(BIDIB_ACTION_DELAY);
	} while (*run_test_case);
	pthread_exit(NULL);
}

static void set_signal(const char *signal, const char *aspect) {
	const int err = bidib_set_signal(signal, aspect);
	assert_int_equal(err, 0);

	const t_bidib_unified_accessory_state_query query = bidib_get_signal_state(signal);
	assert_true(query.known);
	bidib_free_unified_accessory_state_query(query);
}

static void *loop_set_signal(void *arg) {
	bool *run_test_case = (bool *)arg;
	do {
		set_signal("signal1", "green");
		usleep(BIDIB_ACTION_DELAY);
		set_signal("signal1", "orange");
		usleep(BIDIB_ACTION_DELAY);
		set_signal("signal1", "red");
		usleep(BIDIB_ACTION_DELAY);
	} while (*run_test_case);
	pthread_exit(NULL);
}

static void set_peripheral(const char *peripheral, const char *aspect) {
	const int err = bidib_set_peripheral(peripheral, aspect);
	assert_int_equal(err, 0);
	
	const t_bidib_peripheral_state_query query = bidib_get_peripheral_state(peripheral);
	assert_true(query.available);
	bidib_free_peripheral_state_query(query);
}

static void *loop_set_peripheral(void *arg) {
	bool *run_test_case = (bool *)arg;
	do {
		set_peripheral("led1", "state1");
		usleep(BIDIB_ACTION_DELAY);
		set_peripheral("led1", "state2");
		usleep(BIDIB_ACTION_DELAY);
	} while (*run_test_case);
	pthread_exit(NULL);
}

static void set_train(const char *train, const char *track_output, const int speed) {
	const int err = bidib_set_train_speed(train, speed, track_output);
	assert_int_equal(err, 0);
	
	const t_bidib_train_state_query query = bidib_get_train_state(train);	
	assert_true(query.known);
	assert_int_equal(speed, query.data.set_speed_step);
	bidib_free_train_state_query(query);
}

static void *loop_set_train(void *arg) {
	bool *run_test_case = (bool *)arg;
	do {
		set_train("train1", "board1", 0);
		usleep(BIDIB_ACTION_DELAY);
		set_train("train1", "board1", 60);
		usleep(BIDIB_ACTION_DELAY);
		set_train("train1", "board1", 126);
		usleep(BIDIB_ACTION_DELAY);
		set_train("train2", "board1", -126);
		usleep(BIDIB_ACTION_DELAY);
		set_train("train2", "board1", -60);
		usleep(BIDIB_ACTION_DELAY);
		set_train("train2", "board1", -0);
		usleep(BIDIB_ACTION_DELAY);
	} while (*run_test_case);
	pthread_exit(NULL);
}

static void get_boost_state(const char *board, const char *segment) {
	const t_bidib_booster_state_query query_board = bidib_get_booster_state(board);
	assert_true(query_board.known);
	
	const t_bidib_segment_state_query query_segment = bidib_get_segment_state(segment);
	assert_true(query_segment.known);
	bidib_free_segment_state_query(query_segment);
}

static void *loop_get_boost_state(void *arg) {
	bool *run_test_case = (bool *)arg;
	do {
		get_boost_state("board1", "seg1");
		usleep(BIDIB_ACTION_DELAY);
		get_boost_state("board1", "seg2");
		usleep(BIDIB_ACTION_DELAY);
		get_boost_state("board1", "seg3");
		usleep(BIDIB_ACTION_DELAY);
	} while (*run_test_case);
	pthread_exit(NULL);
}

static void get_booster_diagnostic(const char *board) {
	const t_bidib_booster_state_query query = bidib_get_booster_state(board);
	assert_true(query.known);
}

static void *loop_get_booster_diagnostic(void *arg) {
	bool *run_test_case = (bool *)arg;
	do {
		get_booster_diagnostic("board1");
		usleep(BIDIB_ACTION_DELAY);
	} while (*run_test_case);
	pthread_exit(NULL);
}

/**
 * Calls some of the high-level BiDiB functions in parallel.
 * Can only demonstrate deadlock problems.
 * BiDiB state is not tested because it relies on an active serial transmission.
 * 
 * FIXME: Test terminates early when BIDIB_ACTION_DELAY becomes very short.
 *        Possibly because the transmission queues/buffers in libbidib overflow?
 */
static void parallel_all(void **state __attribute__((unused))) {
	bool run_test_case = true;

	pthread_t loop_set_board_point_thread1;
	pthread_t loop_set_board_point_thread2;
	pthread_t loop_set_dcc_point_thread1;
	pthread_t loop_set_dcc_point_thread2;
	pthread_t loop_set_signal_thread1;
	pthread_t loop_set_signal_thread2;
	pthread_t loop_set_peripheral_thread1;
	pthread_t loop_set_peripheral_thread2;
	pthread_t loop_set_train_thread1;
	pthread_t loop_set_train_thread2;
	pthread_t loop_get_boost_state_thread1;
	pthread_t loop_get_boost_state_thread2;
	pthread_t loop_get_booster_diagnostic_thread1;
	pthread_t loop_get_booster_diagnostic_thread2;
	
	pthread_create(&loop_set_board_point_thread1, NULL, loop_set_board_point, (void*) &run_test_case);
	pthread_create(&loop_set_board_point_thread2, NULL, loop_set_board_point, (void*) &run_test_case);
	pthread_create(&loop_set_dcc_point_thread1, NULL, loop_set_dcc_point, (void*) &run_test_case);
	pthread_create(&loop_set_dcc_point_thread2, NULL, loop_set_dcc_point, (void*) &run_test_case);
	pthread_create(&loop_set_signal_thread1, NULL, loop_set_signal, (void*) &run_test_case);
	pthread_create(&loop_set_signal_thread2, NULL, loop_set_signal, (void*) &run_test_case);
	pthread_create(&loop_set_peripheral_thread1, NULL, loop_set_peripheral, (void*) &run_test_case);
	pthread_create(&loop_set_peripheral_thread2, NULL, loop_set_peripheral, (void*) &run_test_case);
	pthread_create(&loop_set_train_thread1, NULL, loop_set_train, (void*) &run_test_case);
	pthread_create(&loop_set_train_thread2, NULL, loop_set_train, (void*) &run_test_case);
	pthread_create(&loop_get_boost_state_thread1, NULL, loop_get_boost_state, (void*) &run_test_case);
	pthread_create(&loop_get_boost_state_thread2, NULL, loop_get_boost_state, (void*) &run_test_case);
	pthread_create(&loop_get_booster_diagnostic_thread1, NULL, loop_get_booster_diagnostic, (void*) &run_test_case);
	pthread_create(&loop_get_booster_diagnostic_thread2, NULL, loop_get_booster_diagnostic, (void*) &run_test_case);

	sleep(TEST_CASE_DURATION);
	run_test_case = false;

	pthread_join(loop_set_board_point_thread1, NULL);
	pthread_join(loop_set_board_point_thread2, NULL);
	pthread_join(loop_set_dcc_point_thread1, NULL);
	pthread_join(loop_set_dcc_point_thread2, NULL);
	pthread_join(loop_set_signal_thread1, NULL);
	pthread_join(loop_set_signal_thread2, NULL);
	pthread_join(loop_set_peripheral_thread1, NULL);
	pthread_join(loop_set_peripheral_thread2, NULL);
	pthread_join(loop_set_train_thread1, NULL);
	pthread_join(loop_set_train_thread2, NULL);
	pthread_join(loop_get_boost_state_thread1, NULL);
	pthread_join(loop_get_boost_state_thread2, NULL);
	pthread_join(loop_get_booster_diagnostic_thread1, NULL);
	pthread_join(loop_get_booster_diagnostic_thread2, NULL);
}

int main(void) {
	bidib_set_lowlevel_debug_mode(true);
	if (!bidib_start_pointer(&read_byte, &write_byte, &write_bytes, "../test/unit/state_tests_config", 0)) {
		set_all_boards_and_trains_connected();
		syslog_libbidib(LOG_INFO, "bidib_parallel_tests: Parallel tests started");
		const struct CMUnitTest tests[] = {
			cmocka_unit_test(parallel_all)
		};
		const int ret = cmocka_run_group_tests(tests, NULL, NULL);
		syslog_libbidib(LOG_INFO, "bidib_parallel_tests: Parallel tests stopped");
		bidib_stop();
		return ret;
	}
	return 1;
}
