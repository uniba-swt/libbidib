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
 * - Bernhard Luedtke <https://github.com/BLuedtke>
 *
 */

#include <stdint.h>

#include "../../include/lowlevel/bidib_lowlevel_firmware.h"
#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"


void bidib_send_fw_update_op_enter(t_bidib_node_address node_address,
                                   t_bidib_unique_id_mod unique_id, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {BIDIB_MSG_FW_UPDATE_OP_ENTER, unique_id.class_id,
	                        unique_id.class_id_ext, unique_id.vendor_id, unique_id.product_id1,
	                        unique_id.product_id2, unique_id.product_id3, unique_id.product_id4};
	bidib_buffer_message_with_data(addr_stack, MSG_FW_UPDATE_OP, 8, data, action_id);
}

void bidib_send_fw_update_op_exit(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {BIDIB_MSG_FW_UPDATE_OP_EXIT};
	bidib_buffer_message_with_data(addr_stack, MSG_FW_UPDATE_OP, 1, data, action_id);
}

void bidib_send_fw_update_op_setdest(t_bidib_node_address node_address,
                                     uint8_t target_range, unsigned int action_id) {
	if (target_range > 1) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_FW_UPDATE_OP (SETDEST) called with invalid parameter target_range = %02x", 
		                target_range);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {BIDIB_MSG_FW_UPDATE_OP_SETDEST, target_range};
	bidib_buffer_message_with_data(addr_stack, MSG_FW_UPDATE_OP, 2, data, action_id);
}

void bidib_send_fw_update_op_data(t_bidib_node_address node_address, uint8_t data_size,
                                  const uint8_t *const data, unsigned int action_id) {
	if (data_size > 121) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_FW_UPDATE_OP (DATA) called with invalid parameter data_size = %02x, "
		                "message too long (max message length is 127 bytes)",
		                data_size);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data_array[data_size + (uint8_t) 1];
	data_array[0] = BIDIB_MSG_FW_UPDATE_OP_DATA;
	uint8_t array_index = 1;
	for (int i = 0; i < data_size; i++) {
		if (data[i] != 0x20 && data[i] != 0x09 && data[i] != 0x0D && data[i] != 0x0A) {
			data_array[array_index] = data[i];
			array_index++;
		}
	}
	bidib_buffer_message_with_data(addr_stack, MSG_FW_UPDATE_OP, array_index, data_array, action_id);
}

void bidib_send_fw_update_op_done(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {BIDIB_MSG_FW_UPDATE_OP_DONE};
	bidib_buffer_message_with_data(addr_stack, MSG_FW_UPDATE_OP, 1, data, action_id);
}
