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

#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"
#include "../state/bidib_state_setter_intern.h"
#include "../../include/bidib.h"


void bidib_send_cs_allocate(t_bidib_node_address node_address, unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {0x00};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_ALLOCATE, 1, data, action_id);
}

void bidib_send_cs_set_state(t_bidib_node_address node_address,
                             unsigned char state, unsigned int action_id) {
	if (state > 0x04 && state != 0x08 && state != 0x09 && state != 0x0D && state != 0xFF) {
		syslog(LOG_ERR, "%s%02x", "MSG_CS_SET_STATE called with invalid parameter "
		       "state = ", state);
		return;
	}
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {state};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_SET_STATE, 1, data, action_id);
}

void bidib_send_cs_drive_intern(t_bidib_node_address node_address,
                                t_bidib_cs_drive_mod cs_drive_params,
                                unsigned int action_id, bool lock) {
	if (cs_drive_params.dcc_format == 1 || cs_drive_params.dcc_format > 3) {
		syslog(LOG_ERR, "%s%02x", "MSG_CS_DRIVE called with invalid parameter "
		       "cs_drive_params.dcc_format = ", cs_drive_params.dcc_format);
		return;
	} else if (cs_drive_params.active > 63) {
		syslog(LOG_ERR, "%s%02x", "MSG_CS_DRIVE called with invalid parameter "
		       "cs_drive_params.active = ", cs_drive_params.active);
		return;
	} else if (cs_drive_params.function1 > 31) {
		syslog(LOG_ERR, "%s%02x", "MSG_CS_DRIVE called with invalid parameter "
		       "cs_drive_params.function1 = ", cs_drive_params.function1);
		return;
	}
	unsigned char addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	unsigned char data[] = {cs_drive_params.dcc_address.addrl,
							cs_drive_params.dcc_address.addrh, cs_drive_params.dcc_format,
							cs_drive_params.active, cs_drive_params.speed,
							cs_drive_params.function1, cs_drive_params.function2,
							cs_drive_params.function3, cs_drive_params.function4};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_DRIVE, 9, data, action_id);
	if (lock) {
		pthread_mutex_lock(&bidib_state_trains_mutex);
		bidib_state_cs_drive(cs_drive_params);
		pthread_mutex_unlock(&bidib_state_trains_mutex);
	} else {
		bidib_state_cs_drive(cs_drive_params);
	}
}

void bidib_send_cs_drive(t_bidib_node_address node_address,
                         t_bidib_cs_drive_mod cs_drive_params, unsigned int action_id) {
	bidib_send_cs_drive_intern(node_address, cs_drive_params, action_id, true);
}

void bidib_send_cs_accessory(t_bidib_node_address node_address,
                             t_bidib_cs_accessory_mod cs_accessory_params,
                             unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {cs_accessory_params.dcc_address.addrl,
	                        cs_accessory_params.dcc_address.addrh, cs_accessory_params.data,
	                        cs_accessory_params.time};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_ACCESSORY, 4, data, action_id);
	bidib_state_cs_accessory(node_address, cs_accessory_params);
}

void bidib_send_cs_pom(t_bidib_node_address node_address,
                       t_bidib_cs_pom_mod cs_pom_params, unsigned int action_id) {
	if (cs_pom_params.opcode > 0x03 && cs_pom_params.opcode != 0x43 &&
	    cs_pom_params.opcode != 0x47 && cs_pom_params.opcode != 0x80 &&
	    cs_pom_params.opcode != 0x81 && cs_pom_params.opcode != 0x82 &&
	    cs_pom_params.opcode != 0x83 && cs_pom_params.opcode != 0x87 &&
	    cs_pom_params.opcode != 0x8B && cs_pom_params.opcode != 0x8F) {
		syslog(LOG_ERR, "%s%02x", "MSG_CS_POM called with invalid parameter "
		       "cs_pom_params.opcode = ", cs_pom_params.opcode);
		return;
	}
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {cs_pom_params.dcc_address.addrl,
	                        cs_pom_params.dcc_address.addrh, cs_pom_params.addrxl,
	                        cs_pom_params.addrxh, cs_pom_params.mid, cs_pom_params.opcode,
	                        cs_pom_params.cv_addrl, cs_pom_params.cv_addrh,
	                        cs_pom_params.cv_addrx, cs_pom_params.data0,
	                        cs_pom_params.data1, cs_pom_params.data2, cs_pom_params.data3};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_POM, 13, data, action_id);
}

void bidib_send_cs_bin_state(t_bidib_node_address node_address,
                             t_bidib_bin_state_mod bin_state_params, unsigned int action_id) {
	if (bin_state_params.data > 1) {
		syslog(LOG_ERR, "%s%02x", "MSG_CS_BIN_STATE called with invalid parameter "
		       "bin_state_params.data = ", bin_state_params.data);
		return;
	}
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {bin_state_params.dcc_address.addrl,
	                        bin_state_params.dcc_address.addrh, bin_state_params.bin_numl,
	                        bin_state_params.bin_numh, bin_state_params.data};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_BIN_STATE, 5, data, action_id);
}

void bidib_send_cs_prog(t_bidib_node_address node_address,
                        t_bidib_cs_prog_mod cs_prog_params, unsigned int action_id) {
	if (cs_prog_params.opcode > 0x04) {
		syslog(LOG_ERR, "%s%02x", "MSG_CS_PROG called with invalid parameter "
		       "cs_prog_params.opcode = ", cs_prog_params.opcode);
		return;
	}
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {cs_prog_params.opcode, cs_prog_params.cv_addrl,
	                        cs_prog_params.cv_addrh, cs_prog_params.data};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_PROG, 4, data, action_id);
}

void bidib_send_cs_rcplus_get_id(t_bidib_node_address node_address, unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_GET_TID};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 1, data, action_id);
}

void bidib_send_cs_rcplus_set_id(t_bidib_node_address node_address,
                                 t_rcplus_tid rcplus_tid, unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_SET_TID, rcplus_tid.cid.mun_0, rcplus_tid.cid.mun_1,
	                        rcplus_tid.cid.mun_2, rcplus_tid.cid.mun_3,
	                        rcplus_tid.cid.mid, rcplus_tid.sid};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 7, data, action_id);
}

void bidib_send_cs_rcplus_ping(t_bidib_node_address node_address,
                               unsigned char interval, unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_PING, interval};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 2, data, action_id);
}

void bidib_send_cs_rcplus_ping_once_p0(t_bidib_node_address node_address,
                                       unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_PING_ONCE_P0};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 1, data, action_id);
}

void bidib_send_cs_rcplus_ping_once_p1(t_bidib_node_address node_address,
                                       unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_PING_ONCE_P1};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 1, data, action_id);
}

void bidib_send_cs_rcplus_bind(t_bidib_node_address node_address,
                               t_rcplus_unique_id rcplus_unique_id, unsigned char new_addrl,
                               unsigned char new_addrh, unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_BIND, rcplus_unique_id.mun_0, rcplus_unique_id.mun_1,
	                        rcplus_unique_id.mun_2, rcplus_unique_id.mun_3,
	                        rcplus_unique_id.mid, new_addrl, new_addrh};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 8, data, action_id);
}

void bidib_send_cs_rcplus_find_p0(t_bidib_node_address node_address,
                                  t_rcplus_unique_id rcplus_unique_id,
                                  unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_FIND_P0, rcplus_unique_id.mun_0, rcplus_unique_id.mun_1,
	                        rcplus_unique_id.mun_2, rcplus_unique_id.mun_3,
	                        rcplus_unique_id.mid};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 6, data, action_id);
}

void bidib_send_cs_rcplus_find_p1(t_bidib_node_address node_address,
                                  t_rcplus_unique_id rcplus_unique_id,
                                  unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub,
	                              node_address.subsub, 0x00};
	unsigned char data[] = {RC_FIND_P1, rcplus_unique_id.mun_0, rcplus_unique_id.mun_1,
	                        rcplus_unique_id.mun_2, rcplus_unique_id.mun_3,
	                        rcplus_unique_id.mid};
	bidib_buffer_message_with_data(addr_stack, MSG_CS_RCPLUS, 6, data, action_id);
}
