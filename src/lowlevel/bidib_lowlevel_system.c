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

#include <unistd.h>
#include <stdint.h>

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"
#include "../state/bidib_state_intern.h"
#include "../../include/bidib.h"


void bidib_send_sys_get_magic(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_GET_MAGIC, action_id);
}

void bidib_send_sys_get_p_version(t_bidib_node_address node_address,
                                  unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_GET_P_VERSION, action_id);
}

void bidib_send_sys_enable(unsigned int action_id) {
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_ENABLE, action_id);
}

void bidib_send_sys_disable(unsigned int action_id) {
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_DISABLE, action_id);
}

void bidib_send_sys_get_unique_id(t_bidib_node_address node_address,
                                  unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_GET_UNIQUE_ID, action_id);
}

void bidib_send_sys_get_sw_version(t_bidib_node_address node_address,
                                   unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_GET_SW_VERSION, action_id);
}

void bidib_send_sys_ping(t_bidib_node_address node_address,
                         uint8_t ping_byte, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {ping_byte};
	bidib_buffer_message_with_data(addr_stack, MSG_SYS_PING, 1, data, action_id);
}

void bidib_send_sys_identify(t_bidib_node_address node_address,
                             uint8_t identify_status, unsigned int action_id) {
	if (identify_status > 1) {
		syslog_libbidib(LOG_ERR, 
		                "MSG_SYS_IDENTIFY called with invalid parameter identify_status = %02x",
		                identify_status);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {identify_status};
	bidib_buffer_message_with_data(addr_stack, MSG_SYS_IDENTIFY, 1, data, action_id);
}

void bidib_send_sys_get_error(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_GET_ERROR, action_id);
}

void bidib_send_sys_reset(unsigned int action_id) {
	uint8_t addr_stack[] = {0x00, 0x00, 0x00, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_SYS_RESET, action_id);
	bidib_flush();
	usleep(1500000); // wait for node login
	//provocate race condition: params should be true here
	bidib_node_state_table_reset(false);
	bidib_uplink_queue_reset(false);
	bidib_uplink_error_queue_reset(false);
	bidib_uplink_intern_queue_reset(false);
	bidib_state_reset();
	bidib_state_init_allocation_table();
	t_bidib_node_address interface = {0x00, 0x00, 0x00};
	bidib_send_get_pkt_capacity(interface, 0);
	bidib_state_set_board_features();
	bidib_send_sys_enable(action_id);
	bidib_state_reset_train_params();
	bidib_set_track_output_state_all(BIDIB_CS_GO);
	bidib_flush();
	usleep(500000); // wait for track output so it can receive initial values
	pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
	bidib_state_query_occupancy();
	pthread_rwlock_unlock(&bidib_state_boards_rwlock);
	bidib_flush();
	usleep(500000); // wait for occupancy data
	bidib_state_set_initial_values();
}

void bidib_send_nodetab_getall(t_bidib_node_address node_address,
                               unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_NODETAB_GETALL, action_id);
}

void bidib_send_nodetab_getnext(t_bidib_node_address node_address,
                                unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_NODETAB_GETNEXT, action_id);
}


void bidib_send_get_pkt_capacity(t_bidib_node_address node_address,
                                 unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_GET_PKT_CAPACITY, action_id);
}

void bidib_send_node_changed_ack(t_bidib_node_address node_address,
                                 uint8_t confirmed_number, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	uint8_t data[] = {confirmed_number};
	bidib_buffer_message_with_data(addr_stack, MSG_NODE_CHANGED_ACK, 1, data, action_id);
}

void bidib_send_sys_clock(t_bidib_node_address node_address, uint8_t tcode0, uint8_t tcode1,
                          uint8_t tcode2, uint8_t tcode3, unsigned int action_id) {
	if (tcode0 > 59) {
		syslog_libbidib(LOG_ERR, "MSG_SYS_CLOCK called with invalid parameter tcode0 = %02x",
		                tcode0);
		return;
	} else if (tcode1 > 151 || tcode1 < 128) {
		syslog_libbidib(LOG_ERR, "MSG_SYS_CLOCK called with invalid parameter tcode1 = %02x",
		                tcode1);
		return;
	} else if (tcode2 > 70 || tcode2 < 64) {
		syslog_libbidib(LOG_ERR, "MSG_SYS_CLOCK called with invalid parameter tcode2 = %02x",
		                tcode2);
		return;
	} else if (tcode3 > 223 || tcode3 < 192) {
		syslog_libbidib(LOG_ERR, "MSG_SYS_CLOCK called with invalid parameter tcode3 = %02x",
		                tcode3);
		return;
	}
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {tcode0, tcode1, tcode2, tcode3};
	bidib_buffer_message_with_data(addr_stack, MSG_SYS_CLOCK, 4, data, action_id);
}
