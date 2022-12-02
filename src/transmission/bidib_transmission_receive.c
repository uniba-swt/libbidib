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

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "bidib_transmission_intern.h"
#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../state/bidib_state_intern.h"
#include "../state/bidib_state_setter_intern.h"
#include "../state/bidib_state_getter_intern.h"
#include "../highlevel/bidib_highlevel_intern.h"
#include "../../include/lowlevel/bidib_lowlevel_system.h"
#include "../../include/lowlevel/bidib_lowlevel_occupancy.h"
#include "../../include/lowlevel/bidib_lowlevel_accessory.h"


#define READ_BUFFER_SIZE 256
#define QUEUE_SIZE 128


pthread_mutex_t bidib_uplink_queue_mutex;
pthread_mutex_t bidib_uplink_error_queue_mutex;
pthread_mutex_t bidib_uplink_intern_queue_mutex;

static uint8_t (*read_byte)(int *byte_read);

static GQueue *uplink_queue = NULL;
static GQueue *uplink_error_queue = NULL;
static GQueue *uplink_intern_queue = NULL;


void bidib_set_read_src(uint8_t (*read)(int *)) {
	uplink_queue = g_queue_new();
	uplink_error_queue = g_queue_new();
	uplink_intern_queue = g_queue_new();
	read_byte = read;
	syslog_libbidib(LOG_INFO, "%s", "Read function was set");
}

void bidib_set_lowlevel_debug_mode(bool uplink_debug_mode_on) {
	bidib_lowlevel_debug_mode = uplink_debug_mode_on;
}

static void bidib_message_queue_free_head(GQueue *queue) {
	t_bidib_message_queue_entry *elem;
	elem = g_queue_pop_head(queue);
	if (elem != NULL) {
		if (elem->message != NULL)	{
			free(elem->message);
		}
		free(elem);
	}
}

static void bidib_message_queue_reset(GQueue *queue) {
	while (!g_queue_is_empty(queue)) {
		bidib_message_queue_free_head(queue);
	}
}

void bidib_uplink_queue_reset(bool lock_mutex) {
	if (lock_mutex) {
		pthread_mutex_lock(&bidib_uplink_queue_mutex);
	}
	if (uplink_queue != NULL) {
		bidib_message_queue_reset(uplink_queue);
	}
	if (lock_mutex) {
		pthread_mutex_unlock(&bidib_uplink_queue_mutex);
	}
	syslog_libbidib(LOG_INFO, "%s", "Message queue reset");
}

void bidib_uplink_queue_free(void) {
	if (!bidib_running) {
		pthread_mutex_lock(&bidib_uplink_queue_mutex);
		if (uplink_queue != NULL) {
			bidib_uplink_queue_reset(false);
			g_queue_free(uplink_queue);
		}
		pthread_mutex_unlock(&bidib_uplink_queue_mutex);
		syslog_libbidib(LOG_INFO, "%s", "Message queue freed");
	}
}

void bidib_uplink_error_queue_reset(bool lock_mutex) {
	if (lock_mutex) {
		pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
	}
	if (uplink_error_queue != NULL) {
		bidib_message_queue_reset(uplink_error_queue);
	}
	if (lock_mutex) {
		pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
	}
	syslog_libbidib(LOG_INFO, "%s", "Error message queue reset");
}

void bidib_uplink_error_queue_free(void) {
	if (!bidib_running) {
		pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
		if (uplink_error_queue != NULL) {
			bidib_uplink_error_queue_reset(false);
			g_queue_free(uplink_error_queue);
		}
		pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
		syslog_libbidib(LOG_INFO, "%s", "Error message queue freed");
	}
}

void bidib_uplink_intern_queue_reset(bool lock_mutex) {
	if (lock_mutex) {
		pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
	}
	if (uplink_intern_queue != NULL) {
		bidib_message_queue_reset(uplink_intern_queue);
	}
	if (lock_mutex) {
		pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
	}
	syslog_libbidib(LOG_INFO, "%s", "Intern message queue reset");
}

void bidib_uplink_intern_queue_free(void) {
	if (!bidib_running) {
		pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
		if (uplink_intern_queue != NULL) {
			bidib_uplink_intern_queue_reset(false);
			g_queue_free(uplink_intern_queue);
		}
		pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
		syslog_libbidib(LOG_INFO, "%s", "Intern message queue freed");
	}
}

// Directs the message and hands over ownership to the specified queue
static void bidib_message_queue_add(GQueue *queue, uint8_t *message,
                                    uint8_t type, const uint8_t *const addr_stack) {
	t_bidib_message_queue_entry *message_queue_entry;
	message_queue_entry = malloc(sizeof(t_bidib_message_queue_entry));
	message_queue_entry->type = type;
	memcpy(message_queue_entry->addr, addr_stack, 4);
	message_queue_entry->message = message;
	if (g_queue_get_length(queue) == QUEUE_SIZE) {
		bidib_message_queue_free_head(queue);
	}
	g_queue_push_tail(queue, message_queue_entry);
}

// Directs the message and hands over ownership to uplink_queue
static void bidib_uplink_queue_add(uint8_t *message, uint8_t type,
                                   const uint8_t *const addr_stack) {
	pthread_mutex_lock(&bidib_uplink_queue_mutex);
	bidib_message_queue_add(uplink_queue, message, type, addr_stack);
	pthread_mutex_unlock(&bidib_uplink_queue_mutex);
}

// Directs the message and hands over ownership to uplink_error_queue
static void bidib_uplink_error_queue_add(uint8_t *message, uint8_t type,
                                         const uint8_t *const addr_stack) {
	pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
	bidib_message_queue_add(uplink_error_queue, message, type, addr_stack);
	pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
}

// Directs the message and hands over ownership to uplink_intern_queue
static void bidib_uplink_intern_queue_add(uint8_t *message, uint8_t type,
                                          const uint8_t *const addr_stack) {
	pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
	bidib_message_queue_add(uplink_intern_queue, message, type, addr_stack);
	pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
}

static void bidib_log_received_message(const uint8_t *const addr_stack, uint8_t msg_seqnum,
                                       uint8_t type, int log_level, const uint8_t *const message,
                                       unsigned int action_id) {
	syslog_libbidib(log_level, "Received from: 0x%02x 0x%02x 0x%02x 0x%02x seq: %d type: %s "
	                "(0x%02x) action id: %d",
	                addr_stack[0], addr_stack[1], addr_stack[2], addr_stack[3], msg_seqnum,
	                bidib_message_string_mapping[type], type, action_id);
	const int size = (message[0] + 1) * 5;
	char hex_string[size];
	bidib_build_message_hex_string(message, hex_string);
	syslog_libbidib(LOG_DEBUG, "Message bytes: %s", hex_string);
}

// Must be called with bidib_state_boards_rwlock read lock acquired. 
static void bidib_log_sys_error(const uint8_t *const message, 
                                t_bidib_node_address node_address, 
                                unsigned int action_id) {
	const t_bidib_board *const board = bidib_state_get_board_ref_by_nodeaddr(node_address);
	int data_index = bidib_first_data_byte_index(message);
	
	uint8_t error_type = message[data_index];
	const char *err_name;
	GString *fault_name = g_string_new("");
	if (error_type <= 0x30) {
		err_name = bidib_error_string_mapping[error_type];
		
		switch (error_type) {
			case (BIDIB_ERR_SEQUENCE):
				g_string_printf(fault_name, "Expected MSG_NUM %d not %d", 
				                message[data_index + 1], message[2]);
				break;
			case (BIDIB_ERR_BUS):
				g_string_printf(fault_name, "%s", 
				                bidib_bus_error_string_mapping[message[data_index + 1]]);
				break;
			default:
				g_string_printf(fault_name, "UNKNOWN");
				break;
		}
	} else {
		err_name = "UNKNOWN";
		g_string_printf(fault_name, "UNKNOWN");
	}
	syslog_libbidib(LOG_ERR, "Feedback for action id %d: MSG_SYS_ERROR %s type: %s (0x%02x): %s", 
	                action_id, board != NULL ? board->id->str : "UNKNOWN", 
	                err_name, error_type, fault_name->str);
	g_string_free(fault_name, TRUE);
}

static void bidib_log_boost_stat_error(const uint8_t *const message, 
                                       t_bidib_node_address node_address,
                                       unsigned int action_id) {
	const t_bidib_board *const board = bidib_state_get_board_ref_by_nodeaddr(node_address);
	int data_index = bidib_first_data_byte_index(message);
	unsigned int error_type = message[data_index];
	
	GString *fault_name = g_string_new("");
	if (error_type <= 0x84) {		
		g_string_printf(fault_name, "%s", 
						bidib_boost_state_string_mapping[error_type]);
	} else {
		g_string_printf(fault_name, "UNKNOWN");
	}
	syslog_libbidib(LOG_ERR, "Feedback for action id %d: MSG_BOOST_STAT %s has error: %s", 
	                action_id, board != NULL ? board->id->str : "UNKNOWN", 
	                fault_name->str);
	g_string_free(fault_name, TRUE);
}

static void bidib_log_boost_stat_okay(const uint8_t *const message, 
                                      t_bidib_node_address node_address,
                                      unsigned int action_id) {
	const t_bidib_board *const board = bidib_state_get_board_ref_by_nodeaddr(node_address);
	int data_index = bidib_first_data_byte_index(message);
	unsigned int msg_type = message[data_index];
	
	GString *msg_name = g_string_new("");
	if (msg_type <= 0x84) {		
		g_string_printf(msg_name, "%s", 
						bidib_boost_state_string_mapping[msg_type]);
	} else {
		g_string_printf(msg_name, "UNKNOWN");
	}
	syslog_libbidib(LOG_INFO, "Feedback for action id %d: MSG_BOOST_STAT %s has state: %s", 
	                action_id, board != NULL ? board->id->str : "UNKNOWN", 
	                msg_name->str);
	g_string_free(msg_name, TRUE);
}

void bidib_handle_received_message(uint8_t *message, uint8_t type,
                                   const uint8_t *const addr_stack, uint8_t seqnum,
                                   unsigned int action_id) {
	if (type != MSG_STALL && bidib_lowlevel_debug_mode) {
		// add to message queue
		bidib_uplink_queue_add(message, type, addr_stack);
		return;
	}

	t_bidib_node_address node_address = {addr_stack[0], addr_stack[1], addr_stack[2]};
	int data_index = bidib_first_data_byte_index(message);
	t_bidib_unique_id_mod unique_id;
	t_bidib_dcc_address dcc_address;
	t_bidib_peripheral_port peripheral_port;
	t_bidib_cs_drive_mod cs_drive_params;
	t_bidib_board *board;
	bool secack_on;
	switch (type) {
		case MSG_PKT_CAPACITY:
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_packet_capacity(message[data_index]);
			free(message);
			break;
		case MSG_NODE_LOST:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			unique_id.class_id = message[data_index + 2];
			unique_id.class_id_ext = message[data_index + 3];
			unique_id.vendor_id = message[data_index + 4];
			unique_id.product_id1 = message[data_index + 5];
			unique_id.product_id2 = message[data_index + 6];
			unique_id.product_id3 = message[data_index + 7];
			unique_id.product_id4 = message[data_index + 8];
			bidib_state_node_lost(unique_id);
			bidib_send_node_changed_ack(node_address, message[data_index], 0);
			bidib_flush();
			free(message);
			break;
		case MSG_NODE_NEW:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			unique_id.class_id = message[data_index + 2];
			unique_id.class_id_ext = message[data_index + 3];
			unique_id.vendor_id = message[data_index + 4];
			unique_id.product_id1 = message[data_index + 5];
			unique_id.product_id2 = message[data_index + 6];
			unique_id.product_id3 = message[data_index + 7];
			unique_id.product_id4 = message[data_index + 8];
			bidib_state_node_new(node_address, message[data_index + 1], unique_id);
			bidib_send_node_changed_ack(node_address, message[data_index], 0);
			bidib_flush();
			free(message);
			break;
		case MSG_STALL:
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_node_update_stall(addr_stack, message[message[0]]);
			free(message);
			break;
		case MSG_CS_STATE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			bidib_state_cs_state(node_address, message[data_index], action_id);
			free(message);
			break;
		case MSG_CS_DRIVE_ACK:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			bidib_state_cs_drive_ack(dcc_address, message[data_index + 2], action_id);
			free(message);
			break;
		case MSG_CS_ACCESSORY_ACK:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			pthread_rwlock_wrlock(&bidib_state_track_rwlock);
			pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
			bidib_state_cs_accessory_ack(node_address, dcc_address,
			                             message[data_index + 2]);
			pthread_rwlock_unlock(&bidib_state_boards_rwlock);
			pthread_rwlock_unlock(&bidib_state_track_rwlock);
			free(message);
			break;
		case MSG_CS_DRIVE_MANUAL:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			cs_drive_params.dcc_address = dcc_address;
			cs_drive_params.dcc_format = message[data_index + 2];
			cs_drive_params.active = message[data_index + 3];
			cs_drive_params.speed = message[data_index + 4];
			cs_drive_params.function1 = message[data_index + 5];
			cs_drive_params.function2 = message[data_index + 6];
			cs_drive_params.function3 = message[data_index + 7];
			cs_drive_params.function4 = message[data_index + 8];
			pthread_rwlock_wrlock(&bidib_state_trains_rwlock);
			bidib_state_cs_drive(cs_drive_params);
			pthread_rwlock_unlock(&bidib_state_trains_rwlock);
			free(message);
			break;
		case MSG_CS_ACCESSORY_MANUAL:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			pthread_rwlock_wrlock(&bidib_state_track_rwlock);
			pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
			bidib_state_cs_accessory_manual(node_address, dcc_address,
			                                message[data_index + 2]);
			pthread_rwlock_unlock(&bidib_state_boards_rwlock);
			pthread_rwlock_unlock(&bidib_state_track_rwlock);
			free(message);
			break;
		case MSG_LC_STAT:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			peripheral_port.port0 = message[data_index];
			peripheral_port.port1 = message[data_index + 1];
			bidib_state_lc_stat(node_address, peripheral_port, message[data_index + 2], action_id);
			free(message);
			break;
		case MSG_LC_WAIT:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			peripheral_port.port0 = message[data_index];
			peripheral_port.port1 = message[data_index + 1];
			bidib_state_lc_wait(node_address, peripheral_port, message[data_index + 2]);
			free(message);
			break;
		case MSG_BM_OCC:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_occ(node_address, message[data_index], true);
			pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			secack_on = (board != NULL && board->secack_on);
			pthread_rwlock_unlock(&bidib_state_boards_rwlock);
			if (secack_on) {
				bidib_send_bm_mirror_occ(node_address, message[data_index], 0);
				bidib_flush();
			}
			free(message);
			break;
		case MSG_BM_FREE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_occ(node_address, message[data_index], false);
			pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			secack_on = board != NULL && board->secack_on;
			pthread_rwlock_unlock(&bidib_state_boards_rwlock);
			if (secack_on) {
				bidib_send_bm_mirror_free(node_address, message[data_index], 0);
				bidib_flush();
			}
			free(message);
			break;
		case MSG_BM_MULTIPLE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_multiple(node_address, message[data_index],
			                        message[data_index + 1], &message[data_index + 2]);
			pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			secack_on = board != NULL && board->secack_on;
			pthread_rwlock_unlock(&bidib_state_boards_rwlock);
			if (secack_on) {
				bidib_send_bm_mirror_multiple(node_address, message[data_index],
				                              message[data_index + 1], &message[data_index + 2], 0);
				bidib_flush();
			}
			free(message);
			break;
		case MSG_BM_CONFIDENCE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			bidib_state_bm_confidence(node_address, message[data_index],
			                          message[data_index + 1], message[data_index + 2],
			                          action_id);
			free(message);
			break;
		case MSG_BM_ADDRESS:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			bidib_state_bm_address(node_address, message[data_index],
			                       (uint8_t) ((message[0] - data_index) / 2),
			                       &message[data_index + 1]);
			free(message);
			break;
		case MSG_BM_CURRENT:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_current(node_address, message[data_index],
			                       message[data_index + 1]);
			free(message);
			break;
		case MSG_BM_SPEED:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			bidib_state_bm_speed(dcc_address, message[data_index + 2],
			                     message[data_index + 3]);
			free(message);
			break;
		case MSG_BM_DYN_STATE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			dcc_address.addrl = message[data_index + 1];
			dcc_address.addrh = message[data_index + 2];
			bidib_state_bm_dyn_state(dcc_address, message[data_index + 3],
			                         message[data_index + 4], action_id);
			free(message);
			break;
		case MSG_BOOST_DIAGNOSTIC:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			bidib_state_boost_diagnostic(node_address,
			                             (uint8_t) (message[0] - data_index + 1),
			                             &message[data_index], action_id);
			free(message);
			break;
		case MSG_ACCESSORY_STATE:
			// update state and check if error
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			bidib_state_accessory_state(node_address, message[data_index],
			                            message[data_index + 1], message[data_index + 2],
			                            message[data_index + 3], message[data_index + 4],
			                            action_id);
			if (message[data_index + 3] == BIDIB_ACC_STATE_ERROR) {
				// add to error queue
				bidib_uplink_error_queue_add(message, type, addr_stack);
			} else {
				free(message);
			}
			break;
		case MSG_ACCESSORY_NOTIFY:
			// update state and check if error
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			bidib_state_accessory_state(node_address, message[data_index],
			                            message[data_index + 1], message[data_index + 2],
			                            message[data_index + 3], message[data_index + 4],
			                            action_id);
			// acknowledge the accessory notification
			bidib_send_accessory_get(node_address, message[data_index], 0);
			if (message[data_index + 3] == BIDIB_ACC_STATE_ERROR) {
				// add to error queue
				bidib_uplink_error_queue_add(message, type, addr_stack);
			} else {
				free(message);
			}
			break;
		case MSG_BOOST_STAT:
			// update state and check if error
			bidib_state_boost_state(node_address, message[data_index]);
			if (bidib_booster_normal_to_simple(
					(t_bidib_booster_power_state) message[data_index])
			    == BIDIB_BSTR_SIMPLE_ERROR) {
				// add to error queue
				bidib_log_received_message(addr_stack, seqnum, type, LOG_ERR,
				                           message, action_id);
				pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
				bidib_log_boost_stat_error(message, node_address, action_id);
				pthread_rwlock_unlock(&bidib_state_boards_rwlock);
				bidib_uplink_error_queue_add(message, type, addr_stack);
			} else {
				bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
				                           message, action_id);
				pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
				bidib_log_boost_stat_okay(message, node_address, action_id);
				pthread_rwlock_unlock(&bidib_state_boards_rwlock);
				free(message);
			}
			break;
		case MSG_CS_DRIVE_EVENT:
			// check if error
			if (message[data_index] == 1) {
				// add to error queue
				bidib_log_received_message(addr_stack, seqnum, type, LOG_ERR,
				                           message, action_id);
				bidib_uplink_error_queue_add(message, type, addr_stack);
			} else {
				bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
				                           message, action_id);
				free(message);
			}
			break;
		case MSG_SYS_MAGIC:
		case MSG_NODETAB_COUNT:
		case MSG_NODETAB:
		case MSG_FEATURE_COUNT:
		case MSG_FEATURE:
			// add to intern message queue
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			bidib_uplink_intern_queue_add(message, type, addr_stack);
			break;
		case MSG_SYS_ERROR:
			// add to error message queue
			bidib_log_received_message(addr_stack, seqnum, type, LOG_DEBUG,
			                           message, action_id);
			pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
			bidib_log_sys_error(message, node_address, action_id);
			pthread_rwlock_unlock(&bidib_state_boards_rwlock);
			bidib_uplink_error_queue_add(message, type, addr_stack);
			break;
		case MSG_NODE_NA:
		case MSG_FEATURE_NA:
		case MSG_LC_NA:
			// add to error message queue
			bidib_log_received_message(addr_stack, seqnum, type, LOG_ERR,
			                           message, action_id);
			bidib_uplink_error_queue_add(message, type, addr_stack);
			break;
		case MSG_BM_POSITION:
			// add to message queue
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			secack_on = board != NULL && board->secack_on;
			pthread_rwlock_unlock(&bidib_state_boards_rwlock);
			if (secack_on) {
				bidib_send_msg_bm_mirror_position(node_address, message[data_index],
				                                  message[data_index + 1],
				                                  message[data_index + 2], 0);
				bidib_flush();
			}
			bidib_uplink_queue_add(message, type, addr_stack);
			break;
		case MSG_VENDOR:
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_vendor(node_address, (uint8_t) (message[0] - data_index + 1),
			                   &message[data_index], action_id);
			free(message);
			break;
		case MSG_SYS_PONG:
		case MSG_SYS_P_VERSION:
		case MSG_SYS_UNIQUE_ID:
		case MSG_SYS_SW_VERSION:
		case MSG_SYS_IDENTIFY_STATE:
		case MSG_VENDOR_ACK:
		case MSG_STRING:
		case MSG_CS_PROG_STATE:
		case MSG_ACCESSORY_PARA:
		case MSG_LC_CONFIGX:
		case MSG_LC_MACRO_STATE:
		case MSG_LC_MACRO:
		case MSG_LC_MACRO_PARA:
		case MSG_CS_POM_ACK:
		case MSG_BM_CV:
		case MSG_BM_XPOM:
		case MSG_BM_RCPLUS:
		case MSG_FW_UPDATE_STAT:
		default:
			// add to message queue
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_uplink_queue_add(message, type, addr_stack);
			break;
	}
}

// FIXME: Possible memory leak of 5 bytes? Message malloc not being freed?
static void bidib_split_packet(const uint8_t *const buffer, size_t buffer_size) {
	// j tracks the message size in terms of buffer elements.
	size_t j = 0;
	
	for (size_t i = 0; i < buffer_size; i += j) {
		// Length of message data is defined in buffer[i]. 
		// Message data starts at buffer[i + 1]
		// and ends at buffer[i + buffer[i]].
		// Thus, total message length is 1 + buffer[i].
		
		uint8_t *message = malloc(sizeof(uint8_t) * buffer[i] + 1);

		// Read up to the number of buffer elements specified in the param buffer_size.
		for (j = 0; j <= buffer[i] && (j + i) < buffer_size; j++) {
			message[j] = buffer[i + j];
		}

		uint8_t type = bidib_extract_msg_type(message);
		if (type == MSG_BM_OCC) {
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			printf("\nMSG_BM_OCC received %ld.%.9ld\n", tv.tv_sec, tv.tv_nsec);
		} else if (type == MSG_BM_MULTIPLE) {
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			printf("\nMSG_BM_MULTIPLE received %ld.%.9ld\n", tv.tv_sec, tv.tv_nsec);
		} else if (type == MSG_BM_ADDRESS) {
			struct timespec tv;
			clock_gettime(CLOCK_MONOTONIC, &tv);
			printf("\nMSG_BM_ADDRESS received %ld.%.9ld\n", tv.tv_sec, tv.tv_nsec);
		}
		uint8_t addr_stack[4];
		bidib_extract_address(message, addr_stack);
		uint8_t msg_seqnum = bidib_extract_seq_num(message);

		if (msg_seqnum != 0x00) {
			uint8_t expected_seqnum = bidib_node_state_get_and_incr_receive_seqnum(addr_stack);
			if (msg_seqnum != expected_seqnum) {
				// Handle wrong sequence numbers
				syslog_libbidib(LOG_ERR, "Wrong sequence number, expected %d", expected_seqnum);
				if (msg_seqnum == 255) {
					bidib_node_state_set_receive_seqnum(addr_stack, 0x01);
				} else {
					bidib_node_state_set_receive_seqnum(addr_stack, ++msg_seqnum);
				}
			}
		}
		unsigned int action_id = bidib_node_state_update(addr_stack, type);
		bidib_handle_received_message(message, type, addr_stack, msg_seqnum, action_id);
	}
}

static void bidib_receive_packet(void) {
	uint8_t data;
	int read_byte_success = 0;

	uint8_t buffer[READ_BUFFER_SIZE];
	size_t buffer_index = 0;
	bool escape_hot = false;
	uint8_t crc = 0x00;

	// Read the packet bytes
	while (bidib_running && !bidib_discard_rx) {
		data = read_byte(&read_byte_success);
		while (!read_byte_success) {
			usleep(5000); // 0.005s
			data = read_byte(&read_byte_success);
			if (!bidib_running || bidib_discard_rx) {
				return;
			}
		}
		read_byte_success = 0;

		if (data == BIDIB_PKT_MAGIC) {
			if (buffer_index == 0) {
				continue;
			}
			break;
		} else if (data == BIDIB_PKT_ESCAPE) {
			// Next byte is escaped
			escape_hot = true;
		} else {
			// Put byte in buffer
			if (escape_hot) {
				data ^= 0x20;
				escape_hot = false;
			}
			buffer[buffer_index] = data;
			crc = bidib_crc_array[buffer[buffer_index] ^ crc];
			buffer_index++;
		}
	}

	if (!bidib_running || bidib_discard_rx) {
		return;
	}

	syslog_libbidib(LOG_DEBUG, "%s", "Received packet");

	if (crc == 0x00) {
		syslog_libbidib(LOG_DEBUG, "%s", "CRC correct, split packet in messages");
		// Split packet in messages and add them to queue, exclude crc sum
		buffer_index--;
		bidib_split_packet(buffer, buffer_index);
	} else {
		syslog_libbidib(LOG_ERR, "%s", "CRC wrong, packet ignored");
	}
}

// Wait for BIDIB_PKT_MAGIC to be received from a master node.
static void bidib_receive_first_pkt_magic(void) {
	uint8_t data;
	int read_byte_success = 0;

	while (bidib_running) {
		data = read_byte(&read_byte_success);
		while (!read_byte_success) {
			usleep(10000); // 0.01s
			data = read_byte(&read_byte_success);
			if (!bidib_running) {
				return;
			}
		}
		read_byte_success = 0;
		if (bidib_discard_rx) {
			continue;
		} else if (data == BIDIB_PKT_MAGIC) {
			return;
		}
	}
}

// Function is forked as a pthread, which requires the
// function to have a void * parameter. The function does
// not use this parameter.
void *bidib_auto_receive(void *par __attribute__((unused))) {
	while (bidib_running) {
		bidib_receive_first_pkt_magic();
		while (bidib_running && !bidib_discard_rx) {
			bidib_receive_packet();
		}
	}
	return NULL;
}

static uint8_t *bidib_read_message_from_queue(GQueue *queue) {
	uint8_t *message = NULL;
	if (!g_queue_is_empty(queue)) {
		t_bidib_message_queue_entry *entry = g_queue_pop_head(queue);
		message = entry->message;
		free(entry);
	}
	return message;
}

uint8_t *bidib_read_message(void) {
	uint8_t *message;
	pthread_mutex_lock(&bidib_uplink_queue_mutex);
	message = bidib_read_message_from_queue(uplink_queue);
	pthread_mutex_unlock(&bidib_uplink_queue_mutex);
	return message;
}

uint8_t *bidib_read_error_message(void) {
	uint8_t *error_message;
	pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
	error_message = bidib_read_message_from_queue(uplink_error_queue);
	pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
	return error_message;
}

uint8_t *bidib_read_intern_message(void) {
	uint8_t *intern_message;
	pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
	intern_message = bidib_read_message_from_queue(uplink_intern_queue);
	pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
	return intern_message;
}
