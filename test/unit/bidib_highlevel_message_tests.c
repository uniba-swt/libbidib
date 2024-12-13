/*
 *
 * Copyright (C) 2017 University of Bamberg, Software Technologies Research Group
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
 * - Nicolas Gross <https://github.com/nicolasgross>
 *
 */

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../include/bidib.h"
#include "../../src/transmission/bidib_transmission_intern.h"
#include "../../src/state/bidib_state_intern.h"
#include "../../src/state/bidib_state_getter_intern.h"
#include "../../src/highlevel/bidib_highlevel_intern.h"


static uint8_t output_buffer[128];
static unsigned int output_index = 0;


static uint8_t read_byte(int *read_byte) {
	*read_byte = 0;
	return 0x00;
}

static void write_bytes(uint8_t* msg, int32_t len) {
	if (msg != NULL && len > 0) {
		for (int32_t i = 0; i < len; ++i) {
			output_buffer[output_index] = msg[i];
			output_index++;
		}
	}
}

// Assume that the system receives a response to its request.
static void board_receives_response(const uint8_t response_type) {
	pthread_rwlock_wrlock(&bidib_boards_rwlock);
	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (board_i != NULL) {
			const uint8_t addr_stack[] = { 
				board_i->node_addr.top,
				board_i->node_addr.sub,
				board_i->node_addr.subsub,
				0x00
			};
			bidib_node_state_update(addr_stack, response_type);
			pthread_rwlock_unlock(&bidib_boards_rwlock);
			return;
		}
	}
	pthread_rwlock_unlock(&bidib_boards_rwlock);
}

static void set_all_boards_and_trains_connected(void) {
	// For accessing bidib_boards
	pthread_rwlock_wrlock(&bidib_boards_rwlock);
	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (board_i != NULL) {
			board_i->connected = true;
		}
	}
	pthread_rwlock_unlock(&bidib_boards_rwlock);
	// For bidib_state_get_train_state_ref (devnote: write)
	pthread_mutex_lock(&trackstate_trains_mutex);
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref("train1");
	if (train_state != NULL) {
		train_state->on_track = true;
	}
	pthread_mutex_unlock(&trackstate_trains_mutex);
}

static void set_board_point_is_sent_correctly(void **state __attribute__((unused))) {
	assert_int_equal(output_index, 0);
	int err = bidib_switch_point("point1", "normal");
	bidib_flush();
	assert_int_equal(err, 0);
	assert_int_equal(output_buffer[0], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[1], 0x05);
	assert_int_equal(output_buffer[2], 0x00);
	assert_int_equal(output_buffer[3], 0x01);
	assert_int_equal(output_buffer[4], MSG_ACCESSORY_SET);
	assert_int_equal(output_buffer[5], 0x02);
	assert_int_equal(output_buffer[6], 0x01);
	// crc
	assert_int_equal(output_buffer[8], BIDIB_PKT_MAGIC);
	assert_int_equal(output_index, 9);
	board_receives_response(MSG_ACCESSORY_STATE);
}

static void set_dcc_point_is_sent_correctly(void **state __attribute__((unused))) {
	int err = bidib_switch_point("point2", "reverse");
	bidib_flush();
	assert_int_equal(err, 0);
	assert_int_equal(output_buffer[9], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[10], 0x07);
	assert_int_equal(output_buffer[11], 0x00);
	assert_int_equal(output_buffer[12], 0x02);
	assert_int_equal(output_buffer[13], MSG_CS_ACCESSORY);
	assert_int_equal(output_buffer[14], 0x22);
	assert_int_equal(output_buffer[15], 0x11);
	assert_int_equal(output_buffer[16], 0b00000000);
	assert_int_equal(output_buffer[17], 0x00);
	assert_int_equal(output_buffer[18], 0x07);
	assert_int_equal(output_buffer[19], 0x00);
	assert_int_equal(output_buffer[20], 0x03);
	assert_int_equal(output_buffer[21], MSG_CS_ACCESSORY);
	assert_int_equal(output_buffer[22], 0x22);
	assert_int_equal(output_buffer[23], 0x11);
	assert_int_equal(output_buffer[24], 0b00100001);
	assert_int_equal(output_buffer[25], 0x00);
	// crc
	assert_int_equal(output_buffer[27], BIDIB_PKT_MAGIC);
	assert_int_equal(output_index, 28);
	board_receives_response(MSG_CS_ACCESSORY_ACK);
	board_receives_response(MSG_CS_ACCESSORY_ACK);
}

static void set_train_speed_is_sent_correctly(void **state __attribute__((unused))) {
	int err = bidib_set_train_speed("train1", 64, "board1");
	bidib_flush();
	assert_int_equal(err, 0);
	assert_int_equal(output_buffer[28], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[29], 0x0C);
	assert_int_equal(output_buffer[30], 0x00);
	assert_int_equal(output_buffer[31], 0x04);
	assert_int_equal(output_buffer[32], MSG_CS_DRIVE);
	assert_int_equal(output_buffer[33], 0x23);
	assert_int_equal(output_buffer[34], 0x01);
	assert_int_equal(output_buffer[35], 0x00);
	assert_int_equal(output_buffer[36], 0x01);
	assert_int_equal(output_buffer[37], 0b11000001);
	assert_int_equal(output_buffer[38], 0x00);
	assert_int_equal(output_buffer[39], 0x00);
	assert_int_equal(output_buffer[40], 0x00);
	assert_int_equal(output_buffer[41], 0x00);
	// crc
	assert_int_equal(output_buffer[43], BIDIB_PKT_MAGIC);
	assert_int_equal(output_index, 44);
	board_receives_response(MSG_CS_DRIVE_ACK);
}

static void set_train_peripheral_is_sent_correctly(void **state __attribute__((unused))) {
	int err = bidib_set_train_peripheral("train1", "light", 0x01, "board1");
	bidib_flush();
	assert_int_equal(err, 0);
	assert_int_equal(output_buffer[44], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[45], 0x0C);
	assert_int_equal(output_buffer[46], 0x00);
	assert_int_equal(output_buffer[47], 0x05);
	assert_int_equal(output_buffer[48], MSG_CS_DRIVE);
	assert_int_equal(output_buffer[49], 0x23);
	assert_int_equal(output_buffer[50], 0x01);
	assert_int_equal(output_buffer[51], 0x00);
	assert_int_equal(output_buffer[52], 0x02);
	assert_int_equal(output_buffer[53], 0x00);
	assert_int_equal(output_buffer[54], 0x10);
	assert_int_equal(output_buffer[55], 0x00);
	assert_int_equal(output_buffer[56], 0x00);
	assert_int_equal(output_buffer[57], 0x00);
	// crc
	assert_int_equal(output_buffer[59], BIDIB_PKT_MAGIC);
	assert_int_equal(output_index, 60);
	board_receives_response(MSG_CS_DRIVE_ACK);
}

static void request_reverser_update_correctly(void **state __attribute__((unused))) {
	const char *reverser = "reverser1";
	const char *board = "board1";
	int err = bidib_request_reverser_state(reverser, board);
	bidib_flush();
	assert_int_equal(err, 0);
	assert_int_equal(output_buffer[60], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[61], 0x09);
	assert_int_equal(output_buffer[62], 0x00);
	assert_int_equal(output_buffer[63], 0x06);
	assert_int_equal(output_buffer[64], MSG_VENDOR_GET);
	assert_int_equal(output_buffer[65], 0x05);
	assert_int_equal(output_buffer[66], '3');
	assert_int_equal(output_buffer[67], '0');
	assert_int_equal(output_buffer[68], '0');
	assert_int_equal(output_buffer[69], '5');
	assert_int_equal(output_buffer[70], '1');
	// crc
	assert_int_equal(output_buffer[72], BIDIB_PKT_MAGIC);
	assert_int_equal(output_index, 73);
	board_receives_response(MSG_VENDOR);
}

int main(void) {
	bidib_set_lowlevel_debug_mode(true);
	if (!bidib_start_pointer(&read_byte, &write_bytes, "../test/unit/state_tests_config", 0)) {
		set_all_boards_and_trains_connected();
		syslog_libbidib(LOG_INFO, "bidib_highlevel_message_tests: Highlevel message tests started");
		const struct CMUnitTest tests[] = {
			cmocka_unit_test(set_board_point_is_sent_correctly),
			cmocka_unit_test(set_dcc_point_is_sent_correctly),
			cmocka_unit_test(set_train_speed_is_sent_correctly),
			cmocka_unit_test(set_train_peripheral_is_sent_correctly),
			cmocka_unit_test(request_reverser_update_correctly)
		};
		int ret = cmocka_run_group_tests(tests, NULL, NULL);
		syslog_libbidib(LOG_INFO, "bidib_highlevel_message_tests: Highlevel message tests stopped");
		bidib_stop();
		return ret;
	}
	return 1;
}
