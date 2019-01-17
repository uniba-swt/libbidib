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

#include "../include/bidib.h"
#include "../src/transmission/bidib_transmission_intern.h"


static uint8_t receiver_buffer[256];
static unsigned int receiver_index = 0;
static uint8_t input_buffer[256];
static unsigned int input_index = 0;
static volatile bool isWaiting = true;
static volatile bool stall_one_read = false;
static volatile bool wait_stall = true;
static volatile bool input_buffer_read_completely = false;

static void test_setup() {
	input_buffer[0] = BIDIB_PKT_MAGIC;
	input_buffer[1] = 0x08;
	input_buffer[2] = 0x01;
	input_buffer[3] = 0x01;
	input_buffer[4] = 0x01;
	input_buffer[5] = 0x00;
	input_buffer[6] = 0x01;
	input_buffer[7] = MSG_SYS_MAGIC;
	input_buffer[8] = 0x04;
	input_buffer[9] = 0x04;
	input_buffer[10] = 0x08;
	input_buffer[11] = 0x01;
	input_buffer[12] = 0x01;
	input_buffer[13] = 0x01;
	input_buffer[14] = 0x00;
	input_buffer[15] = 0x02;
	input_buffer[16] = MSG_SYS_MAGIC;
	input_buffer[17] = 0x04;
	input_buffer[18] = 0x04;
	input_buffer[19] = 0xDD;
	input_buffer[20] = BIDIB_PKT_MAGIC;
	input_buffer[21] = 0x08;
	input_buffer[22] = 0x01;
	input_buffer[23] = 0x01;
	input_buffer[24] = 0x01;
	input_buffer[25] = 0x00;
	input_buffer[26] = 0x03;
	input_buffer[27] = MSG_SYS_MAGIC;
	input_buffer[28] = 0x04;
	input_buffer[29] = 0x04;
	input_buffer[30] = 0x0F;
	input_buffer[31] = BIDIB_PKT_MAGIC;
	input_buffer[32] = 0x06;
	input_buffer[33] = 0x01;
	input_buffer[34] = 0x02;
	input_buffer[35] = 0x00;
	input_buffer[36] = 0x01;
	input_buffer[37] = MSG_STALL;
	input_buffer[38] = 0x01;
	input_buffer[39] = 0x3C;
	input_buffer[40] = BIDIB_PKT_MAGIC;
	input_buffer[41] = 0x06;
	input_buffer[42] = 0x01;
	input_buffer[43] = 0x02;
	input_buffer[44] = 0x00;
	input_buffer[45] = 0x02;
	input_buffer[46] = MSG_STALL;
	input_buffer[47] = 0x00;
	input_buffer[48] = 0x86;
	input_buffer[49] = BIDIB_PKT_MAGIC;
}

static void write_byte(uint8_t msg_byte) {
	receiver_buffer[receiver_index++] = msg_byte;
}

static uint8_t read_byte(int *read_byte) {
	if (isWaiting) {
		*read_byte = 0;
		return 0x00;
	}

	if (input_index == 41) {
		stall_one_read = true;
		if (wait_stall) {
			// wait until stall zero should be sent
			*read_byte = 0;
			return 0x00;
		}
	}

	if (input_index > 49) {
		input_buffer_read_completely = true;
		*read_byte = 0;
		return 0x00;
	}

	*read_byte = 1;
	return input_buffer[input_index++];
}

static void first_message_is_buffered(void **state) {
	t_bidib_node_address address = {0x00, 0x00, 0x00};
	bidib_send_sys_get_magic(address, 0);
	assert_int_equal(receiver_index, 0);
}

static void second_message_to_same_node_is_buffered(void **state) {
	t_bidib_node_address address = {0x00, 0x00, 0x00};
	bidib_send_sys_get_magic(address, 0);
	assert_int_equal(receiver_index, 0);
}

static void message_to_other_node_is_buffered(void **state) {
	t_bidib_node_address address = {0x01, 0x00, 0x00};
	bidib_send_sys_get_magic(address, 0);
	assert_int_equal(receiver_index, 0);
}

static void flush_sends_and_clears_buffer(void **state) {
	// should clear and downlink
	bidib_flush();
	// should do nothing, because the buffer is empty
	bidib_flush();
	assert_int_equal(receiver_index, 16);
}

static void messages_are_put_into_packets_correctly(void **state) {
	assert_int_equal(receiver_buffer[0], BIDIB_PKT_MAGIC);
	assert_int_equal(receiver_buffer[1], 0x03);
	assert_int_equal(receiver_buffer[2], 0x00);
	assert_int_equal(receiver_buffer[3], 0x01);
	assert_int_equal(receiver_buffer[4], MSG_SYS_GET_MAGIC);
	assert_int_equal(receiver_buffer[5], 0x03);
	assert_int_equal(receiver_buffer[6], 0x00);
	assert_int_equal(receiver_buffer[7], 0x02);
	assert_int_equal(receiver_buffer[8], MSG_SYS_GET_MAGIC);
	assert_int_equal(receiver_buffer[9], 0x04);
	assert_int_equal(receiver_buffer[10], 0x01);
	assert_int_equal(receiver_buffer[11], 0x00);
	assert_int_equal(receiver_buffer[12], 0x01);
	assert_int_equal(receiver_buffer[13], MSG_SYS_GET_MAGIC);
	// CRC sum tested extra
	assert_int_equal(receiver_buffer[15], BIDIB_PKT_MAGIC);
}

static void message_with_data_bytes_is_sent_correctly(void **state) {
	t_bidib_node_address address = {0x00, 0x00, 0x00};
	bidib_send_sys_ping(address, 0xFE, 0);
	bidib_flush();
	assert_int_equal(receiver_buffer[16], BIDIB_PKT_MAGIC);
	assert_int_equal(receiver_buffer[17], 0x04);
	assert_int_equal(receiver_buffer[18], 0x00);
	assert_int_equal(receiver_buffer[19], 0x03);
	assert_int_equal(receiver_buffer[20], MSG_SYS_PING);
	assert_int_equal(receiver_buffer[21], BIDIB_PKT_ESCAPE);
	assert_int_equal(receiver_buffer[22], 0xFE ^ (uint8_t) 0x20);
	assert_int_equal(receiver_buffer[23], BIDIB_PKT_ESCAPE);
	assert_int_equal(receiver_buffer[24], 0xFE ^ (uint8_t) 0x20);
	assert_int_equal(receiver_buffer[25], BIDIB_PKT_MAGIC);
}

static void messages_flushed_if_packet_max_capacity_exceeded(void **state) {
	bidib_flush();
	t_bidib_node_address address1 = {0x01, 0x01, 0x01};
	t_bidib_node_address address2 = {0x01, 0x01, 0x02};
	bidib_send_sys_get_magic(address1, 0);
	bidib_send_sys_get_magic(address1, 0);
	bidib_send_sys_get_magic(address1, 0);
	bidib_send_sys_get_magic(address1, 0);
	bidib_send_sys_get_magic(address1, 0);
	bidib_send_sys_get_magic(address2, 0);
	bidib_send_sys_get_magic(address2, 0);
	bidib_send_sys_get_magic(address2, 0);
	assert_int_equal(receiver_index, 26);
	bidib_send_sys_get_magic(address2, 0);
	assert_int_equal(receiver_index, 92);
	bidib_flush();
	assert_int_equal(receiver_index, 92);
}

// Uthash uses char[] for hashing, but the address is of type uint8_t[].
// This test should check, whether bytes > 127 are hashed correctly, because
// negative values are not defined in ascii table.
static void big_address_bytes_buffered_correctly(void **state) {
	t_bidib_node_address address = {0xF2, 0xF1, 0xF3};
	bidib_send_sys_get_magic(address, 0);
	bidib_send_sys_get_magic(address, 0);
	bidib_flush();
	assert_int_equal(receiver_buffer[92], BIDIB_PKT_MAGIC);
	assert_int_equal(receiver_buffer[93], 0x06);
	assert_int_equal(receiver_buffer[94], 0xF2);
	assert_int_equal(receiver_buffer[95], 0xF1);
	assert_int_equal(receiver_buffer[96], 0xF3);
	assert_int_equal(receiver_buffer[97], 0x00);
	assert_int_equal(receiver_buffer[98], 0x01);
	assert_int_equal(receiver_buffer[99], MSG_SYS_GET_MAGIC);
	assert_int_equal(receiver_buffer[100], 0x06);
	assert_int_equal(receiver_buffer[101], 0xF2);
	assert_int_equal(receiver_buffer[102], 0xF1);
	assert_int_equal(receiver_buffer[103], 0xF3);
	assert_int_equal(receiver_buffer[104], 0x00);
	assert_int_equal(receiver_buffer[105], 0x02);
	assert_int_equal(receiver_buffer[106], MSG_SYS_GET_MAGIC);
	// CRC sum tested extra
	assert_int_equal(receiver_buffer[108], BIDIB_PKT_MAGIC);
}

static void crc_sums_are_correct(void **state) {
	// Calculated with http://tomeko.net/online_tools/crc8.php
	assert_int_equal(receiver_buffer[14], 0x59);
	assert_int_equal(receiver_buffer[107], 0x28);
}

static void messages_exceeding_max_response_are_enqueued(void **state) {
	t_bidib_node_address address = {0x01, 0x01, 0x01};
	bidib_send_sys_get_magic(address, 0); //sent
	bidib_send_sys_get_magic(address, 0); //sent
	bidib_send_sys_get_magic(address, 0); //sent
	bidib_send_sys_get_magic(address, 0); //queued
	bidib_send_sys_get_magic(address, 0); //queued
	bidib_send_sys_get_magic(address, 0); //queued
	bidib_flush();
	assert_int_equal(receiver_index, 133);
}

static void queued_messages_sent_if_capacity_free_again(void **state) {
	isWaiting = false;
	int i = 3;
	while (i > 0) { // make sure all answers are processed
		uint8_t *message = bidib_read_message();
		if (message != NULL) {
			i--;
			free(message);
		}
	}
	assert_int_equal(receiver_index, 163);
}

static void received_stall_one_blocks_node_and_subnodes(void **state) {
	while (!stall_one_read) {
		// wait until stall was read
	}
	t_bidib_node_address address = {0x01, 0x02};
	t_bidib_node_address subaddress = {0x01, 0x02, 0x01};
	bidib_send_sys_get_magic(address, 0);
	bidib_send_sys_get_magic(subaddress, 0);
	bidib_flush();
	assert_int_equal(receiver_index, 163);
}

static void received_stall_zero_flushes_node_and_subnodes(void **state) {
	wait_stall = false;
	while (input_buffer_read_completely == false) {
		// wait until stall zero is read
	}
	bidib_flush();
	assert_int_equal(receiver_buffer[163], BIDIB_PKT_MAGIC);
	assert_int_equal(receiver_buffer[164], 0x05);
	assert_int_equal(receiver_buffer[165], 0x01);
	assert_int_equal(receiver_buffer[166], 0x02);
	assert_int_equal(receiver_buffer[167], 0x00);
	assert_int_equal(receiver_buffer[168], 0x01);
	assert_int_equal(receiver_buffer[169], MSG_SYS_GET_MAGIC);
	// crc
	assert_int_equal(receiver_buffer[171], BIDIB_PKT_MAGIC);
	assert_int_equal(receiver_buffer[172], BIDIB_PKT_MAGIC);
	assert_int_equal(receiver_buffer[173], 0x06);
	assert_int_equal(receiver_buffer[174], 0x01);
	assert_int_equal(receiver_buffer[175], 0x02);
	assert_int_equal(receiver_buffer[176], 0x01);
	assert_int_equal(receiver_buffer[177], 0x00);
	assert_int_equal(receiver_buffer[178], 0x01);
	assert_int_equal(receiver_buffer[179], MSG_SYS_GET_MAGIC);
	// CRC sum tested extra
	assert_int_equal(receiver_buffer[181], BIDIB_PKT_MAGIC);
	assert_int_equal(receiver_index, 182);
}

int main(void) {
	test_setup();
	bidib_set_lowlevel_debug_mode(true);
	bidib_start_pointer(&read_byte, &write_byte, NULL, 0);
	syslog(LOG_INFO, "%s", "Send tests started");
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(first_message_is_buffered),
		cmocka_unit_test(second_message_to_same_node_is_buffered),
		cmocka_unit_test(message_to_other_node_is_buffered),
		cmocka_unit_test(flush_sends_and_clears_buffer),
		cmocka_unit_test(messages_are_put_into_packets_correctly),
		cmocka_unit_test(message_with_data_bytes_is_sent_correctly),
		cmocka_unit_test(messages_flushed_if_packet_max_capacity_exceeded),
		cmocka_unit_test(big_address_bytes_buffered_correctly),
		cmocka_unit_test(messages_exceeding_max_response_are_enqueued),
		cmocka_unit_test(crc_sums_are_correct),
		cmocka_unit_test(queued_messages_sent_if_capacity_free_again),
		cmocka_unit_test(received_stall_one_blocks_node_and_subnodes),
		cmocka_unit_test(received_stall_zero_flushes_node_and_subnodes)
	};
	int ret = cmocka_run_group_tests(tests, NULL, NULL);
	syslog(LOG_INFO, "%s", "Send tests stopped");
	bidib_stop();
	return ret;
}
