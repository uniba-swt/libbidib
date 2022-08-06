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

#include <stdint.h>

#include "../../include/lowlevel/bidib_lowlevel_occupancy.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"
#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../transmission/bidib_transmission_intern.h"


void bidib_send_bm_get_range(t_bidib_node_address node_address,
                             uint8_t start, uint8_t end,
                             unsigned int action_id) {
	if (start % 8 != 0) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_BM_GET_RANGE called with invalid parameter start = %02x", 
		                start);
		return;
	} else if (end % 8 != 0) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_BM_GET_RANGE called with invalid parameter end = %02x", 
		                end);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {start, end};
	bidib_buffer_message_with_data(addr_stack, MSG_BM_GET_RANGE, 2, data, action_id);
}

void bidib_send_bm_mirror_multiple(t_bidib_node_address node_address,
                                   uint8_t mnum, uint8_t size,
                                   const uint8_t *const data, unsigned int action_id) {
	if (mnum % 8 != 0) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_BM_MIRROR_MULTIPLE called with invalid parameter mnum = %02x", 
		                mnum);
		return;
	} else if (size < 8 || size > 128 || size % 8 != 0) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_BM_MIRROR_MULTIPLE called with invalid parameter size = %02x", 
		                size);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data_array_length = (uint8_t) 2 + (size / (uint8_t) 8);
	uint8_t data_array[data_array_length];
	data_array[0] = mnum;
	data_array[1] = size;
	for (int i = 0; i < (size / 8); i++) {
		data_array[2 + i] = data[i];
	}
	bidib_buffer_message_with_data(addr_stack, MSG_BM_MIRROR_MULTIPLE,
	                               data_array_length, data_array, action_id);
}

void bidib_send_bm_mirror_occ(t_bidib_node_address node_address,
                              uint8_t mnum, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {mnum};
	bidib_buffer_message_with_data(addr_stack, MSG_BM_MIRROR_OCC, 1, data, action_id);
}

void bidib_send_bm_mirror_free(t_bidib_node_address node_address,
                               uint8_t mnum, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {mnum};
	bidib_buffer_message_with_data(addr_stack, MSG_BM_MIRROR_FREE, 1, data, action_id);
}

void bidib_send_bm_addr_get_range(t_bidib_node_address node_address, uint8_t start,
                                  uint8_t end, unsigned int action_id) {
	if (start > end) {
		syslog_libbidib(LOG_ERR, "%s", 
		                "MSG_BM_ADDR_GET_RANGE called with invalid parameters, start > end.");
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {start, end};
	bidib_buffer_message_with_data(addr_stack, MSG_BM_ADDR_GET_RANGE, 2, data, action_id);
}

void bidib_send_bm_get_confidence(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_BM_GET_CONFIDENCE, action_id);
}

void bidib_send_msg_bm_mirror_position(t_bidib_node_address node_address,
                                       uint8_t type, uint8_t location_low,
                                       uint8_t location_high, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {type, location_low, location_high};
	bidib_buffer_message_with_data(addr_stack, MSG_BM_MIRROR_POSITION, 3, data, action_id);
}

