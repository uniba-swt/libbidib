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
#include "../state/bidib_state_intern.h"
#include "../state/bidib_state_getter_intern.h"
#include "bidib_highlevel_intern.h"


int bidib_ping(const char *board, uint8_t ping_byte) {
	if (board == NULL) {
		syslog_libbidib(LOG_ERR, "Ping: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_request_handling_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *tmp_board = bidib_state_get_board_ref(board);
	if (tmp_board != NULL && tmp_board->connected) {
		unsigned int action_id = bidib_get_and_incr_action_id();
		syslog_libbidib(LOG_NOTICE, "Send ping to board: "
		                "%s (0x%02x 0x%02x 0x%02x 0x00) "
		                "with action id: %d", tmp_board->id->str,
		                tmp_board->node_addr.top, tmp_board->node_addr.sub,
		                tmp_board->node_addr.subsub, action_id);
		t_bidib_node_address tmp_addr = tmp_board->node_addr;
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		bidib_send_sys_ping(tmp_addr, ping_byte, action_id);
		pthread_mutex_unlock(&bidib_request_handling_mutex);
		return 0;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	pthread_mutex_unlock(&bidib_request_handling_mutex);
	return 1;
}

int bidib_identify(const char *board, uint8_t state) {
	if (board == NULL) {
		syslog_libbidib(LOG_ERR, "Identify: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_request_handling_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *tmp_board = bidib_state_get_board_ref(board);
	if (tmp_board != NULL && tmp_board->connected) {
		unsigned int action_id = bidib_get_and_incr_action_id();
		syslog_libbidib(LOG_NOTICE, "Send identify to board: "
		                "%s (0x%02x 0x%02x 0x%02x 0x00) "
		                "with action id: %d", tmp_board->id->str,
		                tmp_board->node_addr.top, tmp_board->node_addr.sub,
		                tmp_board->node_addr.subsub, action_id);
		t_bidib_node_address tmp_addr = tmp_board->node_addr;
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		bidib_send_sys_identify(tmp_addr, state, action_id);
		pthread_mutex_unlock(&bidib_request_handling_mutex);
		return 0;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	pthread_mutex_unlock(&bidib_request_handling_mutex);
	return 1;
}

int bidib_get_protocol_version(const char *board) {
	if (board == NULL) {
		syslog_libbidib(LOG_ERR, "Get protocol version: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_request_handling_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *tmp_board = bidib_state_get_board_ref(board);
	if (tmp_board != NULL && tmp_board->connected) {
		unsigned int action_id = bidib_get_and_incr_action_id();
		syslog_libbidib(LOG_NOTICE, "Send get protocol version to board: "
		                "%s (0x%02x 0x%02x 0x%02x 0x00) with action id: %d", 
		                tmp_board->id->str,
		                tmp_board->node_addr.top, tmp_board->node_addr.sub,
		                tmp_board->node_addr.subsub, action_id);
		t_bidib_node_address tmp_addr = tmp_board->node_addr;
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		bidib_send_sys_get_p_version(tmp_addr, action_id);
		pthread_mutex_unlock(&bidib_request_handling_mutex);
		return 0;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	pthread_mutex_unlock(&bidib_request_handling_mutex);
	return 1;
}

int bidib_get_software_version(const char *board) {
	if (board == NULL) {
		syslog_libbidib(LOG_ERR, "Get software version: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_request_handling_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *tmp_board = bidib_state_get_board_ref(board);
	if (tmp_board != NULL && tmp_board->connected) {
		unsigned int action_id = bidib_get_and_incr_action_id();
		syslog_libbidib(LOG_NOTICE, "Send get software version to board: "
		                "%s (0x%02x 0x%02x 0x%02x 0x00) with action id: %d", 
		                tmp_board->id->str,
		                tmp_board->node_addr.top, tmp_board->node_addr.sub,
		                tmp_board->node_addr.subsub, action_id);
		t_bidib_node_address tmp_addr = tmp_board->node_addr;
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		bidib_send_sys_get_sw_version(tmp_addr, action_id);
		pthread_mutex_unlock(&bidib_request_handling_mutex);
		return 0;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	pthread_mutex_unlock(&bidib_request_handling_mutex);
	return 1;
}
