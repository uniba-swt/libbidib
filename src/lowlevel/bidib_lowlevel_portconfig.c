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

#include <syslog.h>
#include <stdint.h>

#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"


void bidib_send_lc_output(t_bidib_node_address node_address, uint8_t port0,
                          uint8_t port1, uint8_t portstat,
                          unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {port0, port1, portstat};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_OUTPUT, 3, data, action_id);
}

void bidib_send_lc_port_query(t_bidib_node_address node_address, uint8_t port0,
                              uint8_t port1, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {port0, port1};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_PORT_QUERY, 2, data, action_id);
}

void bidib_send_lc_port_query_all(t_bidib_node_address node_address,
                                  t_bidib_port_query_params query_params,
                                  unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {query_params.select0, query_params.select1,
	                        query_params.range.start0, query_params.range.start1,
	                        query_params.range.end0, query_params.range.end1};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_PORT_QUERY_ALL, 6, data, action_id);
}

void bidib_send_lc_configx_set(t_bidib_node_address node_address, uint8_t port0,
                               uint8_t port1, uint8_t pairs_num,
                               uint8_t *pairs, unsigned int action_id) {
	if (pairs_num < 1 || pairs_num > 8) {
		syslog(LOG_ERR, "%s%02x",
		       "MSG_LC_CONFIGX_SET called with invalid parameter pairs_num = ", pairs_num);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data_length = (uint8_t) 2 + (pairs_num * (uint8_t) 2);
	uint8_t data[data_length];
	data[0] = port0;
	data[1] = port1;
	for (int i = 0; i < pairs_num; i++) {
		data[2 + i] = pairs[i];
	}
	bidib_buffer_message_with_data(addr_stack, MSG_LC_CONFIGX_SET, data_length,
	                               data, action_id);
}

void bidib_send_lc_configx_get(t_bidib_node_address node_address, uint8_t port0,
                               uint8_t port1, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {port0, port1};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_CONFIGX_GET, 2, data, action_id);
}

void bidib_send_lc_configx_get_all(t_bidib_node_address node_address, uint8_t port0,
                                   uint8_t port1,
                                   t_bidib_port_query_address_range address_range,
                                   unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {port0, port1, address_range.start0, address_range.start1,
	                        address_range.end0, address_range.end1};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_CONFIGX_GET_ALL, 6, data, action_id);
}

void bidib_send_lc_macro_handle(t_bidib_node_address node_address, uint8_t macro_index,
                                uint8_t opcode, unsigned int action_id) {
	if (opcode > 1 && opcode < 252) {
		syslog(LOG_ERR, "%s%02x", "MSG_LC_MACRO_HANDLE called with invalid parameter "
				"opcode = ", opcode);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {macro_index, opcode};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_MACRO_HANDLE, 2, data, action_id);
}

void bidib_send_lc_macro_set(t_bidib_node_address node_address,
                             t_bidib_macro_params macro_params, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {macro_params.data0, macro_params.data1, macro_params.data2,
	                        macro_params.data3, macro_params.data4, macro_params.data5};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_MACRO_SET, 6, data, action_id);
}

void bidib_send_lc_macro_get(t_bidib_node_address node_address, uint8_t macro_index,
                             uint8_t point_index, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {macro_index, point_index};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_MACRO_GET, 2, data, action_id);
}

void bidib_send_lc_macro_para_set(t_bidib_node_address node_address,
                                  t_bidib_macro_params macro_params, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {macro_params.data0, macro_params.data1, macro_params.data2,
	                        macro_params.data3, macro_params.data4, macro_params.data5};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_MACRO_PARA_SET, 6, data, action_id);
}

void bidib_send_lc_macro_para_get(t_bidib_node_address node_address, uint8_t macro_index,
                                  uint8_t param_index, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {macro_index, param_index};
	bidib_buffer_message_with_data(addr_stack, MSG_LC_MACRO_PARA_GET, 2, data, action_id);
}
