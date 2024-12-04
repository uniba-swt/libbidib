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

static void vendor_data_set_correctly_send(void **state __attribute__((unused))) {
	t_bidib_node_address address = {0x01, 0x01, 0x00};
	char *name = "Test";
	char *value = "Value";
	t_bidib_vendor_data vendor_data = {
		0x04,
		(uint8_t *) name,
		0x05,
		(uint8_t *) value
	};
	bidib_send_vendor_set(address, vendor_data, 0);
	bidib_flush();
	assert_int_equal(output_buffer[0], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[1], 0x10);
	assert_int_equal(output_buffer[2], 0x01);
	assert_int_equal(output_buffer[3], 0x01);
	assert_int_equal(output_buffer[4], 0x00);
	assert_int_equal(output_buffer[5], 0x01);
	assert_int_equal(output_buffer[6], MSG_VENDOR_SET);
	assert_int_equal(output_buffer[7], 0x04);
	assert_int_equal(output_buffer[8], 'T');
	assert_int_equal(output_buffer[9], 'e');
	assert_int_equal(output_buffer[10], 's');
	assert_int_equal(output_buffer[11], 't');
	assert_int_equal(output_buffer[12], 0x05);
	assert_int_equal(output_buffer[13], 'V');
	assert_int_equal(output_buffer[14], 'a');
	assert_int_equal(output_buffer[15], 'l');
	assert_int_equal(output_buffer[16], 'u');
	assert_int_equal(output_buffer[17], 'e');
	assert_int_equal(output_buffer[18], 0xC9);
	assert_int_equal(output_buffer[19], BIDIB_PKT_MAGIC);
}

static void vendor_data_get_correctly_send(void **state __attribute__((unused))) {
	t_bidib_node_address address = {0x01, 0x02, 0x00};
	char *name = "Name";
	bidib_send_vendor_get(address, 0x04, (uint8_t *) name, 0);
	bidib_flush();
	assert_int_equal(output_buffer[20], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[21], 0x0A);
	assert_int_equal(output_buffer[22], 0x01);
	assert_int_equal(output_buffer[23], 0x02);
	assert_int_equal(output_buffer[24], 0x00);
	assert_int_equal(output_buffer[25], 0x01);
	assert_int_equal(output_buffer[26], MSG_VENDOR_GET);
	assert_int_equal(output_buffer[27], 0x04);
	assert_int_equal(output_buffer[28], 'N');
	assert_int_equal(output_buffer[29], 'a');
	assert_int_equal(output_buffer[30], 'm');
	assert_int_equal(output_buffer[31], 'e');
	// crc tested extra
	assert_int_equal(output_buffer[33], BIDIB_PKT_MAGIC);
}

static void string_set_correctly_send(void **state __attribute__((unused))) {
	t_bidib_node_address address = {0x01, 0x03, 0x00};
	char *string = "Test";
	bidib_send_string_set(address, 0x00, 0x01, 0x04, (uint8_t *) string, 0);
	bidib_flush();
	assert_int_equal(output_buffer[34], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[35], 0x0C);
	assert_int_equal(output_buffer[36], 0x01);
	assert_int_equal(output_buffer[37], 0x03);
	assert_int_equal(output_buffer[38], 0x00);
	assert_int_equal(output_buffer[39], 0x01);
	assert_int_equal(output_buffer[40], MSG_STRING_SET);
	assert_int_equal(output_buffer[41], 0x00);
	assert_int_equal(output_buffer[42], 0x01);
	assert_int_equal(output_buffer[43], 0x04);
	assert_int_equal(output_buffer[44], 'T');
	assert_int_equal(output_buffer[45], 'e');
	assert_int_equal(output_buffer[46], 's');
	assert_int_equal(output_buffer[47], 't');
	// crc tested extra
	assert_int_equal(output_buffer[49], BIDIB_PKT_MAGIC);
}

static void fw_update_op_data_correctly_send(void **state __attribute__((unused))) {
	t_bidib_node_address address = {0x01, 0x01, 0x00};
	uint8_t data[] = {0x01, 0x00, 0x20, 0x12, 0x09, 0x0D, 0x14, 0x0A};
	bidib_send_fw_update_op_data(address, 8, data, 0);
	bidib_flush();
	assert_int_equal(output_buffer[50], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[51], 0x0A);
	assert_int_equal(output_buffer[52], 0x01);
	assert_int_equal(output_buffer[53], 0x01);
	assert_int_equal(output_buffer[54], 0x00);
	assert_int_equal(output_buffer[55], 0x02);
	assert_int_equal(output_buffer[56], MSG_FW_UPDATE_OP);
	assert_int_equal(output_buffer[57], BIDIB_MSG_FW_UPDATE_OP_DATA);
	assert_int_equal(output_buffer[58], 0x01);
	assert_int_equal(output_buffer[59], 0x00);
	assert_int_equal(output_buffer[60], 0x12);
	assert_int_equal(output_buffer[61], 0x14);
	assert_int_equal(output_buffer[62], 0x39);
	assert_int_equal(output_buffer[63], BIDIB_PKT_MAGIC);
}

static void bm_mirror_multiple_correctly_send(void **state __attribute__((unused))) {
	t_bidib_node_address address = {0x01, 0x04, 0x00};
	uint8_t data[] = {0x01, 0x27};
	bidib_send_bm_mirror_multiple(address, 0x40, 0x11, data, 0); // bad params
	bidib_send_bm_mirror_multiple(address, 0x41, 0x10, data, 0); // bad params
	bidib_send_bm_mirror_multiple(address, 0x40, 0x10, data, 0); // good
	bidib_flush();
	assert_int_equal(output_buffer[64], BIDIB_PKT_MAGIC);
	assert_int_equal(output_buffer[65], 0x09);
	assert_int_equal(output_buffer[66], 0x01);
	assert_int_equal(output_buffer[67], 0x04);
	assert_int_equal(output_buffer[68], 0x00);
	assert_int_equal(output_buffer[69], 0x01);
	assert_int_equal(output_buffer[70], MSG_BM_MIRROR_MULTIPLE);
	assert_int_equal(output_buffer[71], 0x40);
	assert_int_equal(output_buffer[72], 0x10);
	assert_int_equal(output_buffer[73], 0x01);
	assert_int_equal(output_buffer[74], 0x27);
	// crc tested extra
	assert_int_equal(output_buffer[76], BIDIB_PKT_MAGIC);
}

int main(void) {
	bidib_set_lowlevel_debug_mode(true);
	bidib_start_pointer(&read_byte, &write_bytes, NULL, 0);
	syslog_libbidib(LOG_INFO, "bidib_lowlevel_message_tests: %s", "Lowlevel message tests started");
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(vendor_data_set_correctly_send),
		cmocka_unit_test(vendor_data_get_correctly_send),
		cmocka_unit_test(string_set_correctly_send),
		cmocka_unit_test(fw_update_op_data_correctly_send),
		cmocka_unit_test(bm_mirror_multiple_correctly_send)
	};
	int ret = cmocka_run_group_tests(tests, NULL, NULL);
	syslog_libbidib(LOG_INFO, "bidib_lowlevel_message_tests: %s", "Lowlevel message tests started");
	bidib_stop();
	return ret;
}
