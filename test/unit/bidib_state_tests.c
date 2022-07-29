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
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#include <stdint.h>

#include "../../include/bidib.h"
#include "../../include/definitions/bidib_definitions_custom.h"
#include "../../src/transmission/bidib_transmission_intern.h"


static uint8_t input_buffer[128];
static uint8_t output_buffer[256];
static volatile bool wait_for_accessory_change = true;
static volatile bool wait_for_peripheral_change = true;
static volatile bool wait_for_occupancy_change = true;
static volatile bool wait_for_second_occupancy_change = true;
static volatile bool wait_for_cs_drive_change = true;

static uint8_t read_byte(int *read_byte) {
	static unsigned int input_index = 0;
	while (bidib_discard_rx) {
		usleep(50000);
	}
	if (input_index > 111) {
		*read_byte = 0;
		return 0x00;
	} else {
		if (input_index == 7) {
			// Wait until reset is done, so that the following messages in the
			// buffer don't get lost.
			usleep(2000000);
		}
		while ((input_index == 45 && wait_for_accessory_change) ||
		       (input_index == 56 && wait_for_peripheral_change) ||
		       (input_index == 65 && wait_for_occupancy_change) ||
		       (input_index == 94 && wait_for_second_occupancy_change) ||
		       (input_index == 103 && wait_for_cs_drive_change)) {
			*read_byte = 0;
			return 0x00;
		}
		*read_byte = 1;
		return input_buffer[input_index++];
	}
}

static void write_byte(uint8_t byte) {
	static unsigned int output_index = 0;
	output_buffer[output_index++] = byte;
}

static void test_setup(void) {
	// response for protocol initialization
	input_buffer[0] = BIDIB_PKT_MAGIC;
	input_buffer[1] = 0x03;
	input_buffer[2] = 0x00;
	input_buffer[3] = 0x00;
	input_buffer[4] = MSG_SYS_MAGIC;
	input_buffer[5] = 0x5A;
	input_buffer[6] = BIDIB_PKT_MAGIC;
	// nodetab get all response
	input_buffer[7] = BIDIB_PKT_MAGIC;
	input_buffer[8] = 0x04;
	input_buffer[9] = 0x00;
	input_buffer[10] = 0x01;
	input_buffer[11] = MSG_NODETAB_COUNT;
	input_buffer[12] = 0x01;
	input_buffer[13] = 0xB3;
	input_buffer[14] = BIDIB_PKT_MAGIC;
	// nodetab get next response
	input_buffer[15] = BIDIB_PKT_MAGIC;
	input_buffer[16] = 0x0C;
	input_buffer[17] = 0x00;
	input_buffer[18] = 0x02;
	input_buffer[19] = MSG_NODETAB;
	input_buffer[20] = 0x01; // version
	input_buffer[21] = 0x00; // address
	input_buffer[22] = 0xDA; // uid 1
	input_buffer[23] = 0x00; // uid 2
	input_buffer[24] = 0x0D; // uid 3
	input_buffer[25] = 0x68; // uid 4
	input_buffer[26] = 0x00; // uid 5
	input_buffer[27] = 0x01; // uid 6
	input_buffer[28] = 0xEE; // uid 7
	input_buffer[29] = 0x34; // crc
	input_buffer[30] = BIDIB_PKT_MAGIC;
	// feature set responses
	input_buffer[31] = 0x05;
	input_buffer[32] = 0x00;
	input_buffer[33] = 0x03;
	input_buffer[34] = MSG_FEATURE;
	input_buffer[35] = 0x01;
	input_buffer[36] = 0x00;
	input_buffer[37] = 0x05;
	input_buffer[38] = 0x00;
	input_buffer[39] = 0x04;
	input_buffer[40] = MSG_FEATURE;
	input_buffer[41] = 0x04;
	input_buffer[42] = 0x01;
	input_buffer[43] = 0x66;
	input_buffer[44] = BIDIB_PKT_MAGIC;
	// accessory state change
	input_buffer[45] = 0x08;
	input_buffer[46] = 0x00;
	input_buffer[47] = 0x05;
	input_buffer[48] = MSG_ACCESSORY_STATE;
	input_buffer[49] = 0x02; // ANUM
	input_buffer[50] = 0x01; // ASPECT
	input_buffer[51] = 0x02; // TOTAL
	input_buffer[52] = 0x00; // EXECUTE
	input_buffer[53] = 0x00; // WAIT
	input_buffer[54] = 0x71;
	input_buffer[55] = BIDIB_PKT_MAGIC;
	// peripheral state value change
	input_buffer[56] = 0x06;
	input_buffer[57] = 0x00;
	input_buffer[58] = 0x06;
	input_buffer[59] = MSG_LC_STAT;
	input_buffer[60] = 0x23; // port 0
	input_buffer[61] = 0x01; // port 1
	input_buffer[62] = 0x01; // portstat
	input_buffer[63] = 0xC1;
	input_buffer[64] = BIDIB_PKT_MAGIC;
	// occupancy detection
	input_buffer[65] = 0x04;
	input_buffer[66] = 0x00;
	input_buffer[67] = 0x07;
	input_buffer[68] = MSG_BM_OCC;
	input_buffer[69] = 0x00;
	input_buffer[70] = 0x06;
	input_buffer[71] = 0x00;
	input_buffer[72] = 0x08;
	input_buffer[73] = MSG_BM_CONFIDENCE;
	input_buffer[74] = 0x01;
	input_buffer[75] = 0x01;
	input_buffer[76] = 0x01;
	input_buffer[77] = 0x08;
	input_buffer[78] = 0x00;
	input_buffer[79] = 0x09;
	input_buffer[80] = MSG_BM_ADDRESS;
	input_buffer[81] = 0x00;
	input_buffer[82] = 0x23;
	input_buffer[83] = 0x01;
	input_buffer[84] = 0x02;
	input_buffer[85] = 0x03;
	input_buffer[86] = 0x05;
	input_buffer[87] = 0x00;
	input_buffer[88] = 0x0A;
	input_buffer[89] = MSG_BM_CURRENT;
	input_buffer[90] = 0x00;
	input_buffer[91] = 0x81;
	input_buffer[92] = 0xD6;
	input_buffer[93] = BIDIB_PKT_MAGIC;
	// one train less
	input_buffer[94] = 0x06;
	input_buffer[95] = 0x00;
	input_buffer[96] = 0x09;
	input_buffer[97] = MSG_BM_ADDRESS;
	input_buffer[98] = 0x00;
	input_buffer[99] = 0x23;
	input_buffer[100] = 0x01;
	input_buffer[101] = 0x96;
	input_buffer[102] = BIDIB_PKT_MAGIC;
	// cs drive
	input_buffer[103] = 0x06;
	input_buffer[104] = 0x00;
	input_buffer[105] = 0x0B;
	input_buffer[106] = MSG_CS_DRIVE_ACK;
	input_buffer[107] = 0x23;
	input_buffer[108] = 0x01;
	input_buffer[109] = 0x03;
	input_buffer[110] = 0xE6;
	input_buffer[111] = BIDIB_PKT_MAGIC;
}

static void sys_reset_send_after_connection_is_established(void **state __attribute__((unused))) {
	assert_int_equal(output_buffer[0], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[1], 0x03);
	assert_int_equal(output_buffer[2], 0x00);
	assert_int_equal(output_buffer[3], 0x00);
	assert_int_equal(output_buffer[4], MSG_SYS_DISABLE);
	// crc
	assert_int_equal(output_buffer[6], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[7], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[8], 0x03);
	assert_int_equal(output_buffer[9], 0x00);
	assert_int_equal(output_buffer[10], 0x00);
	assert_int_equal(output_buffer[11], MSG_SYS_GET_MAGIC);
	// crc
	assert_int_equal(output_buffer[13], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[14], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[15], 0x03);
	assert_int_equal(output_buffer[16], 0x00);
	assert_int_equal(output_buffer[17], 0x00);
	assert_int_equal(output_buffer[18], MSG_SYS_GET_MAGIC);
	// crc
	assert_int_equal(output_buffer[20], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[21], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[22], 0x03);
	assert_int_equal(output_buffer[23], 0x00);
	assert_int_equal(output_buffer[24], 0x01);
	assert_int_equal(output_buffer[25], MSG_SYS_RESET);
	// crc
	assert_int_equal(output_buffer[27], BIDIB_PKT_MAGIC);
}

static void allocation_table_read_in_correctly(void **state __attribute__((unused))) {
	t_bidib_unique_id_mod uid = {0xDA, 0x00, 0x0D, 0x68, 0x00, 0x01, 0xEE};
	t_bidib_node_address_query address_query = bidib_get_nodeaddr_by_uniqueid(uid);
	assert_int_equal(address_query.known_and_connected, true);
	assert_int_equal(address_query.address.top, 0x00);
	assert_int_equal(address_query.address.sub, 0x00);
	assert_int_equal(address_query.address.subsub, 0x00);
}

static void accessory_state_change_updates_state_correctly(void **state __attribute__((unused))) {
	t_bidib_unified_accessory_state_query query = bidib_get_point_state("point1");
	assert_int_equal(query.known, true);
	assert_int_equal(query.type, BIDIB_ACCESSORY_BOARD);
	assert_string_equal(query.board_accessory_state.state_id, "unknown");
	assert_int_equal(query.board_accessory_state.state_value, 0x00);
	assert_int_equal(query.board_accessory_state.execution_state, BIDIB_EXEC_STATE_REACHED);
	wait_for_accessory_change = false;
	usleep(100000);
	bidib_free_unified_accessory_state_query(query);
	query = bidib_get_point_state("point1");
	assert_int_equal(query.known, true);
	assert_int_equal(query.type, BIDIB_ACCESSORY_BOARD);
	assert_string_equal(query.board_accessory_state.state_id, "normal");
	assert_int_equal(query.board_accessory_state.state_value, 0x01);
	assert_int_equal(query.board_accessory_state.execution_state, BIDIB_EXEC_STATE_REACHED_VERIFIED);
	bidib_free_unified_accessory_state_query(query);
}

static void peripheral_state_change_updates_state_correctly(void **state __attribute__((unused))) {
	t_bidib_peripheral_state_query query = bidib_get_peripheral_state("led1");
	assert_int_equal(query.available, true);
	assert_string_equal(query.data.state_id, "unknown");
	assert_int_equal(query.data.state_value, 0x00);
	assert_int_equal(query.data.time_unit, BIDIB_TIMEUNIT_MILLISECONDS);
	assert_int_equal(query.data.wait, 0x00);
	wait_for_peripheral_change = false;
	usleep(100000);
	bidib_free_peripheral_state_query(query);
	query = bidib_get_peripheral_state("led1");
	assert_int_equal(query.available, true);
	assert_string_equal(query.data.state_id, "state2");
	assert_int_equal(query.data.state_value, 0x01);
	assert_int_equal(query.data.time_unit, BIDIB_TIMEUNIT_MILLISECONDS);
	assert_int_equal(query.data.wait, 0x00);
	bidib_free_peripheral_state_query(query);
}

static void occupancy_detection_updates_state_correctly(void **state __attribute__((unused))) {
	t_bidib_segment_state_query query = bidib_get_segment_state("seg1");
	assert_int_equal(query.known, true);
	assert_int_equal(query.data.occupied, false);
	assert_int_equal(query.data.dcc_address_cnt, 0);
	assert_int_equal(query.data.power_consumption.known, false);
	assert_int_equal(query.data.confidence.freeze, false);
	assert_int_equal(query.data.confidence.conf_void, false);
	assert_int_equal(query.data.confidence.nosignal, false);
	bidib_free_segment_state_query(query);
	wait_for_occupancy_change = false;
	usleep(100000);
	query = bidib_get_segment_state("seg1");
	assert_int_equal(query.known, true);
	assert_int_equal(query.data.occupied, true);
	assert_int_equal(query.data.dcc_address_cnt, 2);
	assert_int_equal(query.data.dcc_addresses[0].addrl, 0x23);
	assert_int_equal(query.data.dcc_addresses[0].addrh, 0x01);
	assert_int_equal(query.data.dcc_addresses[1].addrl, 0x02);
	assert_int_equal(query.data.dcc_addresses[1].addrh, 0x03);
	assert_int_equal(query.data.power_consumption.known, true);
	assert_int_equal(query.data.power_consumption.overcurrent, false);
	assert_int_equal(query.data.power_consumption.current, 1344);
	assert_int_equal(query.data.confidence.freeze, true);
	assert_int_equal(query.data.confidence.conf_void, true);
	assert_int_equal(query.data.confidence.nosignal, true);
	bidib_free_segment_state_query(query);
	wait_for_second_occupancy_change = false;
	usleep(100000);
	query = bidib_get_segment_state("seg1");
	assert_int_equal(query.known, true);
	assert_int_equal(query.data.occupied, true);
	assert_int_equal(query.data.dcc_address_cnt, 1);
	assert_int_equal(query.data.dcc_addresses[0].addrl, 0x23);
	assert_int_equal(query.data.dcc_addresses[0].addrh, 0x01);
	assert_int_equal(query.data.power_consumption.known, true);
	assert_int_equal(query.data.power_consumption.overcurrent, false);
	assert_int_equal(query.data.power_consumption.current, 1344);
	assert_int_equal(query.data.confidence.freeze, true);
	assert_int_equal(query.data.confidence.conf_void, true);
	assert_int_equal(query.data.confidence.nosignal, true);
	bidib_free_segment_state_query(query);
}

static void cs_drive_and_ack_update_state_correctly(void **state __attribute__((unused))) {
	t_bidib_train_state_query query = bidib_get_train_state("train1");
	assert_int_equal(query.known, true);
	assert_int_equal(query.data.on_track, true);
	assert_int_equal(query.data.set_speed_step, 0);
	assert_int_equal(query.data.ack, BIDIB_DCC_ACK_PENDING);
	bidib_set_train_speed("train1", 8, "board1");
	wait_for_cs_drive_change = false;
	usleep(100000);
	bidib_free_train_state_query(query);
	query = bidib_get_train_state("train1");
	assert_int_equal(query.known, true);
	assert_int_equal(query.data.on_track, true);
	assert_int_equal(query.data.set_speed_step, 8);
	assert_int_equal(query.data.orientation, BIDIB_TRAIN_ORIENTATION_LEFT);
	assert_int_equal(query.data.ack, BIDIB_DCC_ACK_OUTPUT);
	bidib_free_train_state_query(query);
}

int main(void) {
	test_setup();
	bidib_start_pointer(&read_byte, &write_byte, "../test/unit/state_tests_config", 250);
	syslog_libbidib(LOG_INFO, "bidib_state_tests: %s", "State tests started");
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(sys_reset_send_after_connection_is_established),
			cmocka_unit_test(allocation_table_read_in_correctly),
			cmocka_unit_test(accessory_state_change_updates_state_correctly),
			cmocka_unit_test(peripheral_state_change_updates_state_correctly),
			cmocka_unit_test(occupancy_detection_updates_state_correctly),
			cmocka_unit_test(cs_drive_and_ack_update_state_correctly)
	};
	int ret = cmocka_run_group_tests(tests, NULL, NULL);
	syslog_libbidib(LOG_INFO, "bidib_state_tests: %s", "State tests stopped");
	bidib_stop();
	return ret;
}
