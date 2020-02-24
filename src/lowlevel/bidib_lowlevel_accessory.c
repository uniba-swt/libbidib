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

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"


void bidib_send_accessory_set(t_bidib_node_address node_address, uint8_t anum,
                              uint8_t aspect, unsigned int action_id) {
	if (anum > 127) {
		syslog_libbidib(LOG_ERR, "MSG_ACCESSORY_SET called with invalid parameter anum = %02x", anum);
		return;
	} else if (aspect > 127) {
		syslog_libbidib(LOG_ERR, "MSG_ACCESSORY_SET called with invalid parameter aspect = %02x", aspect);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {anum, aspect};
	bidib_buffer_message_with_data(addr_stack, MSG_ACCESSORY_SET, 2, data, action_id);
}

void bidib_send_accessory_get(t_bidib_node_address node_address, uint8_t anum,
                              unsigned int action_id) {
	if (anum > 127) {
		syslog_libbidib(LOG_ERR, "MSG_ACCESSORY_GET called with invalid parameter anum = %02x", anum);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {anum};
	bidib_buffer_message_with_data(addr_stack, MSG_ACCESSORY_GET, 1, data, action_id);
}

void bidib_send_accessory_para_set_opmode(t_bidib_node_address node_address, uint8_t anum,
                                          uint8_t anum_op, unsigned int action_id) {
	if (anum > 127) {
		syslog_libbidib(LOG_ERR,
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_PARA_OPMODE) "
		                "called with invalid parameter anum = %02x", anum);
		return;
	} else if (anum_op > 127) {
		syslog_libbidib(LOG_ERR,
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_PARA_OPMODE) "
		                "called with invalid parameter anum_op = %02x", anum_op);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {anum, BIDIB_ACCESSORY_PARA_OPMODE, anum_op};
	bidib_buffer_message_with_data(addr_stack, MSG_ACCESSORY_PARA_SET, 3, data, action_id);
}

void bidib_send_accessory_para_set_startup(t_bidib_node_address node_address, uint8_t anum,
                                           uint8_t startup_behaviour, unsigned int action_id) {
	if (anum > 127) {
		syslog_libbidib(LOG_ERR,
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_PARA_STARTUP) "
		                "called with invalid parameter anum = %02x", anum);
		return;
	} else if (startup_behaviour > 127 && startup_behaviour < 254) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_PARA_STARTUP) "
		                "called with invalid parameter startup_behaviour = %02x",
		                startup_behaviour);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {anum, BIDIB_ACCESSORY_PARA_STARTUP, startup_behaviour};
	bidib_buffer_message_with_data(addr_stack, MSG_ACCESSORY_PARA_SET, 3, data, action_id);
}

void bidib_send_accessory_para_set_macromap(t_bidib_node_address node_address, uint8_t anum,
                                            uint8_t data_size, uint8_t *data,
                                            unsigned int action_id) {
	if (anum > 127) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_PARA_MACROMAP) "
		                "called with invalid parameter anum = %02x", anum);
		return;
	} else if (data_size > 16) {
		syslog_libbidib(LOG_ERR,
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_PARA_MACROMAP) "
		                "called with invalid parameter data_size = %02x", data_size);
		return;
	} else if (data[data_size - 1] != 0xFF) {
		syslog_libbidib(LOG_ERR, "%s",
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_PARA_MACROMAP) "
		                "called with invalid parameter data, last byte != 0xFF");
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data_array_size = (uint8_t) 2 + data_size;
	uint8_t data_array[data_array_size];
	data_array[0] = anum;
	data_array[1] = BIDIB_ACCESSORY_PARA_MACROMAP;
	for (int i = 0; i < data_size; i++) {
		data_array[2 + i] = data[i];
	}
	bidib_buffer_message_with_data(addr_stack, MSG_ACCESSORY_PARA_SET, data_array_size, data_array, action_id);
}

void bidib_send_accessory_para_set_switch_time(t_bidib_node_address node_address, uint8_t anum,
                                               uint8_t time, unsigned int action_id) {
	if (anum > 127) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_ACCESSORY_PARA_SET (BIDIB_ACCESSORY_SWITCH_TIME) "
		                "called with invalid parameter anum = %02x", anum);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {anum, BIDIB_ACCESSORY_SWITCH_TIME, time};
	bidib_buffer_message_with_data(addr_stack, MSG_ACCESSORY_PARA_SET, 3, data, action_id);
}

void bidib_send_accessory_para_get(t_bidib_node_address node_address, uint8_t anum, uint8_t para_num,
                                   unsigned int action_id) {
	if (anum > 127) {
		syslog_libbidib(LOG_ERR, "MSG_ACCESSORY_PARA_GET called with invalid parameter anum = %02x", 
		                anum);
		return;
	} else if (para_num < 251) {
		syslog_libbidib(LOG_ERR, "MSG_ACCESSORY_PARA_GET called with invalid parameter para_num = %02x", 
		                para_num);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {anum, para_num};
	bidib_buffer_message_with_data(addr_stack, MSG_ACCESSORY_PARA_GET, 2, data, action_id);
}
