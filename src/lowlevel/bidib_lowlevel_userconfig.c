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

#include "../../include/lowlevel/bidib_lowlevel_userconfig.h"
#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"


void bidib_send_vendor_enable(t_bidib_node_address node_address,
                              t_bidib_unique_id_mod unique_id, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {unique_id.class_id, unique_id.class_id_ext,
	                        unique_id.vendor_id, unique_id.product_id1,
	                        unique_id.product_id2, unique_id.product_id3,
	                        unique_id.product_id4};
	bidib_buffer_message_with_data(addr_stack, MSG_VENDOR_ENABLE, 7, data, action_id);
}

void bidib_send_vendor_disable(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_VENDOR_DISABLE, action_id);
}

void bidib_send_vendor_set(t_bidib_node_address node_address,
                           t_bidib_vendor_data vendor_data, unsigned int action_id) {
	if (vendor_data.name_length + vendor_data.value_length > 119) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_VENDOR_SET called with invalid parameter vendor_data, "
		                "message too long (max message length is 127 bytes");
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data_length = vendor_data.name_length + vendor_data.value_length +
	                            (uint8_t) 2;
	uint8_t data[data_length];
	data[0] = vendor_data.name_length;
	for (int i = 0; i < vendor_data.name_length; i++) {
		data[i + 1] = vendor_data.name[i];
	}
	data[vendor_data.name_length + 1] = vendor_data.value_length;
	for (int i = 0; i < vendor_data.value_length; i++) {
		data[i + vendor_data.name_length + 2] = vendor_data.value[i];
	}
	bidib_buffer_message_with_data(addr_stack, MSG_VENDOR_SET, data_length, data, action_id);
}

void bidib_send_vendor_get(t_bidib_node_address node_address, uint8_t name_length,
                           const uint8_t *const name, unsigned int action_id) {
	if (name_length > 120) {
		syslog_libbidib(LOG_ERR,
		                "MSG_VENDOR_GET called with invalid parameter name_length = %02x, "
		                "message too long (max message length is 127 bytes", name_length);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data_length = name_length + (uint8_t) 1;
	uint8_t data[data_length];
	data[0] = name_length;
	for (int i = 0; i < name_length; i++) {
		data[i + 1] = name[i];
	}
	bidib_buffer_message_with_data(addr_stack, MSG_VENDOR_GET, data_length, data, action_id);
}

void bidib_send_string_set(t_bidib_node_address node_address, uint8_t namespace,
                           uint8_t string_id, uint8_t string_size,
                           const uint8_t *const string, unsigned int action_id) {
	if (string_size > 118) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_STRING_SET called with invalid parameter string_size = %02x, " 
		                "message too long (max message length is 127 bytes", string_size);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data_length = string_size + (uint8_t) 3;
	uint8_t data[data_length];
	data[0] = namespace;
	data[1] = string_id;
	data[2] = string_size;
	for (int i = 0; i < string_size; i++) {
		data[i + 3] = string[i];
	}
	bidib_buffer_message_with_data(addr_stack, MSG_STRING_SET, data_length, data, action_id);
}

void bidib_send_string_get(t_bidib_node_address node_address, uint8_t namespace,
                           uint8_t string_id, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {namespace, string_id};
	bidib_buffer_message_with_data(addr_stack, MSG_STRING_GET, 2, data, action_id);
}
