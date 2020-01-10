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
#include <stdlib.h>
#include <syslog.h>
#include <stdint.h>

#include "../../include/bidib.h"
#include "../../src/transmission/bidib_transmission_intern.h"


static uint8_t input_buffer[64];

static uint8_t read_byte(int *read_byte) {
	static unsigned int input_index = 0;
	if (input_index > 28) {
		*read_byte = 0;
		return 0x00;
	} else {
		*read_byte = 1;
		return input_buffer[input_index++];
	}
}

static void write_byte(uint8_t byte) {
	return;
}

static void test_setup(void) {
	// two good messages in one packet
	input_buffer[0] = BIDIB_PKT_MAGIC;
	input_buffer[1] = 0x04;
	input_buffer[2] = 0x01;
	input_buffer[3] = 0x00;
	input_buffer[4] = 0x01;
	input_buffer[5] = MSG_SYS_MAGIC;
	input_buffer[6] = 0x04;
	input_buffer[7] = 0x01;
	input_buffer[8] = 0x00;
	input_buffer[9] = 0x02;
	input_buffer[10] = MSG_SYS_MAGIC;
	input_buffer[11] = 0x84;
	input_buffer[12] = BIDIB_PKT_MAGIC;
	// packet with wrong crc
	input_buffer[13] = 0x04;
	input_buffer[14] = 0x01;
	input_buffer[15] = 0x00;
	input_buffer[16] = 0x06;
	input_buffer[17] = MSG_SYS_MAGIC;
	input_buffer[18] = 0xE7; // Should be 0xE8
	input_buffer[19] = BIDIB_PKT_MAGIC;
	// meaningless pkt magic
	input_buffer[20] = BIDIB_PKT_MAGIC;
	// good message in one packet
	input_buffer[21] = BIDIB_PKT_MAGIC;
	input_buffer[22] = 0x04;
	input_buffer[23] = 0x01;
	input_buffer[24] = 0x00;
	input_buffer[25] = 0x03;
	input_buffer[26] = MSG_SYS_MAGIC;
	input_buffer[27] = 0x17;
	input_buffer[28] = BIDIB_PKT_MAGIC;
}

static void packet_with_two_messages_correctly_handled(void **state) {
	uint8_t *first_message = bidib_read_message();
	while (first_message == NULL) {
		first_message = bidib_read_message();
	}
	assert_int_equal(first_message[0], 0x04);
	assert_int_equal(first_message[1], 0x01);
	assert_int_equal(first_message[2], 0x00);
	assert_int_equal(first_message[3], 0x01);
	assert_int_equal(first_message[4], MSG_SYS_MAGIC);
	free(first_message);
	uint8_t *second_message = bidib_read_message();
	while (second_message == NULL) {
		second_message = bidib_read_message();
	}
	assert_int_equal(second_message[0], 0x04);
	assert_int_equal(second_message[1], 0x01);
	assert_int_equal(second_message[2], 0x00);
	assert_int_equal(second_message[3], 0x02);
	assert_int_equal(second_message[4], MSG_SYS_MAGIC);
	free(second_message);
}

static void corrupted_packets_are_discarded_and_additional_pkt_magic_ignored(void **state) {
	uint8_t *message = bidib_read_message();
	while (message == NULL) {
		message = bidib_read_message();
	}
	assert_int_equal(message[0], 0x04);
	assert_int_equal(message[1], 0x01);
	assert_int_equal(message[2], 0x00);
	assert_int_equal(message[3], 0x03);
	assert_int_equal(message[4], MSG_SYS_MAGIC);
	free(message);
}

int main(void) {
	test_setup();
	bidib_set_lowlevel_debug_mode(true);
	bidib_start_pointer(&read_byte, &write_byte, NULL, 250);
	syslog(LOG_INFO, "bidib_receive_tests: %s", "Receive tests started");
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(packet_with_two_messages_correctly_handled),
		cmocka_unit_test(corrupted_packets_are_discarded_and_additional_pkt_magic_ignored)
	};
	int ret = cmocka_run_group_tests(tests, NULL, NULL);
	syslog(LOG_INFO, "bidib_receive_tests: %s", "Receive tests stopped");
	bidib_stop();
	return ret;
}
