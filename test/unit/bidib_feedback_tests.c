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
#include <unistd.h>
#include <stdint.h>

#include "../../include/bidib.h"
#include "../../src/transmission/bidib_transmission_intern.h"
#include "../../src/state/bidib_state_intern.h"
#include "../../src/state/bidib_state_getter_intern.h"


static uint8_t input_buffer[128];
static uint8_t output_buffer[256];


static uint8_t read_byte(int *read_byte) {
	static unsigned int input_index = 0;
	while (bidib_discard_rx) {
		usleep(50000);
	}
	if (input_index > 44) {
		*read_byte = 0;
		return 0x00;
	} else {
		if (input_index == 7) {
			// Wait until reset is done, so that the following messages in the
			// buffer don't get lost.
			sleep(2);
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
}

static void feedback_system_error(void **state __attribute__((unused))) {
	const uint8_t type = MSG_SYS_ERROR;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	const uint8_t seqnum = 0x00;
	const unsigned int action_id = 0;

	uint8_t *message = malloc(6 * sizeof(uint8_t));
	message[0] = 0x05;           // Message length
	message[1] = addr_stack[0];  // Message address
	message[2] = seqnum;         // Message sequence number
	message[3] = type;           // Message type
	message[4] = 0x04;           // Error message
	message[5] = 0x02;           // Error type
	
	bidib_handle_received_message(message, type, addr_stack, seqnum, action_id);
}

static void feedback_board_feature(void **state __attribute__((unused))) {
	const uint8_t type = MSG_FEATURE;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	const uint8_t seqnum = 0x01;
	const unsigned int action_id = 1;

	uint8_t *message = malloc(6 * sizeof(uint8_t));
	message[0] = 0x05;           // Message length
	message[1] = addr_stack[0];  // Message address
	message[2] = seqnum;         // Message sequence number
	message[3] = type;           // Message type
	message[4] = 0x03;           // Error message
	message[5] = 0x14;           // Error type
	
	bidib_handle_received_message(message, type, addr_stack, seqnum, action_id);
}

static void feedback_accessory_port_state(void **state __attribute__((unused))) {
	const uint8_t type = MSG_LC_STAT;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	const uint8_t seqnum = 0x02;
	const unsigned int action_id = 2;

	uint8_t *message = malloc(7 * sizeof(uint8_t));
	message[0] = 0x06;           // Message length
	message[1] = addr_stack[0];  // Message address
	message[2] = seqnum;         // Message sequence number
	message[3] = type;           // Message type
	message[4] = 0x23;           // Data[0] = Port address (low)
	message[5] = 0x01;           // Data[1] = Port address (high)
	message[6] = 0x00;           // Data[2] = Port status
	
	bidib_handle_received_message(message, type, addr_stack, seqnum, action_id);
}

static void feedback_command_station_state(void **state __attribute__((unused))) {
	const uint8_t type = MSG_CS_STATE;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	const uint8_t seqnum = 0x03;
	const unsigned int action_id = 3;

	uint8_t *message = malloc(5 * sizeof(uint8_t));
	message[0] = 0x04;           // Message length
	message[1] = addr_stack[0];  // Message address
	message[2] = seqnum;         // Message sequence number
	message[3] = type;           // Message type
	message[4] = 0x03;           // State
	
	bidib_handle_received_message(message, type, addr_stack, seqnum, action_id);
}

static void feedback_boost_state(void **state __attribute__((unused))) {
	const uint8_t type = MSG_BOOST_STAT;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t seqnum = 0x04;
	unsigned int action_id = 4;

	uint8_t *message1 = malloc(5 * sizeof(uint8_t));
	message1[0] = 0x04;                // Message length
	message1[1] = addr_stack[0];       // Message address
	message1[2] = seqnum;              // Message sequence number
	message1[3] = type;                // Message type
	message1[4] = BIDIB_BST_STATE_ON;  // State
	
	bidib_handle_received_message(message1, type, addr_stack, seqnum, action_id);
	
	seqnum = 0x05;
	action_id = 5;

	uint8_t *message2 = malloc(5 * sizeof(uint8_t));
	message2[0] = 0x04;                       // Message length
	message2[1] = addr_stack[0];              // Message address
	message2[2] = seqnum;                     // Message sequence number
	message2[3] = type;                       // Message type
	message2[4] = BIDIB_BST_STATE_OFF_SHORT;  // State
	
	bidib_handle_received_message(message2, type, addr_stack, seqnum, action_id);
}

static void feedback_boost_confidence(void **state __attribute__((unused))) {
	const uint8_t type = MSG_BM_CONFIDENCE;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t seqnum = 0x06;
	unsigned int action_id = 6;

	uint8_t *message1 = malloc(7 * sizeof(uint8_t));
	message1[0] = 0x06;                // Message length
	message1[1] = addr_stack[0];       // Message address
	message1[2] = seqnum;              // Message sequence number
	message1[3] = type;                // Message type
	message1[4] = 0x00;                // Void
	message1[5] = 0x05;                // Freeze
	message1[6] = 0x05;                // No signal
	
	bidib_handle_received_message(message1, type, addr_stack, seqnum, action_id);

	seqnum = 0x07;
	action_id = 7;

	uint8_t *message2 = malloc(7 * sizeof(uint8_t));
	message2[0] = 0x06;                // Message length
	message2[1] = addr_stack[0];       // Message address
	message2[2] = seqnum;              // Message sequence number
	message2[3] = type;                // Message type
	message2[4] = 0x00;                // Void
	message2[5] = 0x00;                // Freeze
	message2[6] = 0x00;                // No signal

	bidib_handle_received_message(message2, type, addr_stack, seqnum, action_id);
}

static void feedback_train_acknowledgment(void **state __attribute__((unused))) {
	const uint8_t type = MSG_CS_DRIVE_ACK;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	const uint8_t seqnum = 0x08;
	const unsigned int action_id = 8;

	uint8_t *message = malloc(7 * sizeof(uint8_t));
	message[0] = 0x06;           // Message length
	message[1] = addr_stack[0];  // Message address
	message[2] = seqnum;         // Message sequence number
	message[3] = type;           // Message type
	message[4] = 0x23;           // Train address (low)
	message[5] = 0x81;           // Train address (high)
	message[6] = 0x01;           // Acknowledgement level
	
	bidib_handle_received_message(message, type, addr_stack, seqnum, action_id);
}

static void feedback_train_state(void **state __attribute__((unused))) {
	const uint8_t type = MSG_BM_DYN_STATE;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t seqnum = 0x09;
	unsigned int action_id = 9;

	uint8_t *message1 = malloc(9 * sizeof(uint8_t));
	message1[0] = 0x08;           // Message length
	message1[1] = addr_stack[0];  // Message address
	message1[2] = seqnum;         // Message sequence number
	message1[3] = type;           // Message type
	message1[4] = 0x09;           // Track segment
	message1[5] = 0x23;           // Train address (low)
	message1[6] = 0x81;           // Train address (high)
	message1[7] = 0x01;           // Signal quality
	message1[8] = 0x00;           // Signal is error-free
	
	bidib_handle_received_message(message1, type, addr_stack, seqnum, action_id);

	seqnum = 0x0a;
	action_id = 10;

	uint8_t *message2 = malloc(9 * sizeof(uint8_t));
	message2[0] = 0x08;           // Message length
	message2[1] = addr_stack[0];  // Message address
	message2[2] = seqnum;         // Message sequence number
	message2[3] = type;           // Message type
	message2[4] = 0x09;           // Track segment
	message2[5] = 0x23;           // Train address (low)
	message2[6] = 0x81;           // Train address (high)
	message2[7] = 0x02;           // Temperature
	message2[8] = 0x0f;           // 15 degrees Celsius
	
	bidib_handle_received_message(message2, type, addr_stack, seqnum, action_id);
}

static void feedback_booster_diagnostic(void **state __attribute__((unused))) {
	const uint8_t type = MSG_BOOST_DIAGNOSTIC;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	const uint8_t seqnum = 0x0b;
	const unsigned int action_id = 11;

	uint8_t *message = malloc(10 * sizeof(uint8_t));
	message[0] = 0x09;           // Message length
	message[1] = addr_stack[0];  // Message address
	message[2] = seqnum;         // Message sequence number
	message[3] = type;           // Message type
	message[4] = 0x00;           // Current type
	message[5] = 0x3e;           // Current value
	message[6] = 0x01;           // Voltage type
	message[7] = 0x97;           // Voltage value
	message[8] = 0x02;           // Temperature type
	message[9] = 0x15;           // Temperature value
	
	bidib_handle_received_message(message, type, addr_stack, seqnum, action_id);
}

static void feedback_accessory_state(void **state __attribute__((unused))) {
	const uint8_t type = MSG_ACCESSORY_STATE;
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	uint8_t seqnum = 0x0c;
	unsigned int action_id = 12;

	uint8_t *message1 = malloc(9 * sizeof(uint8_t));
	message1[0] = 0x08;           // Message length
	message1[1] = addr_stack[0];  // Message address
	message1[2] = seqnum;         // Message sequence number
	message1[3] = type;           // Message type
	message1[4] = 0x02;           // Accessory number
	message1[5] = 0x01;           // Aspect number
	message1[6] = 0x02;           // Total number of aspects
	message1[7] = 0x01;           // Execute error
	message1[8] = 0x11;           // Error code
	
	bidib_handle_received_message(message1, type, addr_stack, seqnum, action_id);

	seqnum = 0x0d;
	action_id = 13;

	uint8_t *message2 = malloc(9 * sizeof(uint8_t));
	message2[0] = 0x08;           // Message length
	message2[1] = addr_stack[0];  // Message address
	message2[2] = seqnum;         // Message sequence number
	message2[3] = type;           // Message type
	message2[4] = 0x10;           // Accessory number
	message2[5] = 0x00;           // Aspect number
	message2[6] = 0x03;           // Total number of aspects
	message2[7] = 0x00;           // Execute error
	message2[8] = 0x00;           // Error code
	
	bidib_handle_received_message(message2, type, addr_stack, seqnum, action_id);

	seqnum = 0x0e;
	action_id = 14;

	uint8_t *message3 = malloc(9 * sizeof(uint8_t));
	message3[0] = 0x08;           // Message length
	message3[1] = addr_stack[0];  // Message address
	message3[2] = seqnum;         // Message sequence number
	message3[3] = type;           // Message type
	message3[4] = 0x02;           // Accessory number
	message3[5] = 0x01;           // Aspect number
	message3[6] = 0x02;           // Total number of aspects
	message3[7] = 0x80;           // Execute error
	message3[8] = 0x06;           // Error code
	
	bidib_handle_received_message(message3, type, addr_stack, seqnum, action_id);
}

int main(void) {
	test_setup();
	bidib_start_pointer(&read_byte, &write_byte, "../test/unit/state_tests_config", 250);
	syslog_libbidib(LOG_INFO, "bidib_feedback_tests: Feedback tests started");
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(feedback_system_error),
		cmocka_unit_test(feedback_board_feature),
		cmocka_unit_test(feedback_accessory_port_state),
		cmocka_unit_test(feedback_command_station_state),
		cmocka_unit_test(feedback_boost_state),
		cmocka_unit_test(feedback_boost_confidence),
		cmocka_unit_test(feedback_train_acknowledgment),
		cmocka_unit_test(feedback_train_state),
		cmocka_unit_test(feedback_booster_diagnostic),
		cmocka_unit_test(feedback_accessory_state)
	};
	int ret = cmocka_run_group_tests(tests, NULL, NULL);
	syslog_libbidib(LOG_INFO, "bidib_feedback_tests: Feedback tests stopped");
	bidib_stop();
	return ret;
}
