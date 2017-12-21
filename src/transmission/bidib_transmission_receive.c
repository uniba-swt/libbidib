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

#include <stdlib.h>
#include <syslog.h>
#include <stdbool.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include <stdint.h>

#include "bidib_transmission_intern.h"
#include "../state/bidib_state_intern.h"
#include "../state/bidib_state_setter_intern.h"
#include "../../include/bidib.h"
#include "../state/bidib_state_getter_intern.h"

#define READ_BUFFER_SIZE 256
#define QUEUE_SIZE 128


pthread_mutex_t bidib_uplink_queue_mutex;
pthread_mutex_t bidib_uplink_error_queue_mutex;
pthread_mutex_t bidib_uplink_intern_queue_mutex;

static unsigned char (*read_byte)(int *byte_read);

static GQueue *uplink_queue = NULL;
static GQueue *uplink_error_queue = NULL;
static GQueue *uplink_intern_queue = NULL;


void bidib_set_read_src(unsigned char (*read)(int *)) {
	uplink_queue = g_queue_new();
	uplink_error_queue = g_queue_new();
	uplink_intern_queue = g_queue_new();
	read_byte = read;
	syslog(LOG_INFO, "%s", "Read function was set");
}

void bidib_set_lowlevel_debug_mode(bool uplink_debug_mode_on) {
	bidib_lowlevel_debug_mode = uplink_debug_mode_on;
}

static void bidib_message_queue_free_head(GQueue *queue) {
	t_bidib_message_queue_entry *elem;
	elem = g_queue_pop_head(queue);
	free(elem->message);
	free(elem);
}

static void bidib_message_queue_reset(GQueue *queue) {
	while (!g_queue_is_empty(queue)) {
		bidib_message_queue_free_head(queue);
	}
}

void bidib_uplink_queue_reset(void) {
	pthread_mutex_lock(&bidib_uplink_queue_mutex);
	if (uplink_queue != NULL) {
		bidib_message_queue_reset(uplink_queue);
	}
	pthread_mutex_unlock(&bidib_uplink_queue_mutex);
	syslog(LOG_INFO, "%s", "Message queue reset");
}

void bidib_uplink_queue_free(void) {
	if (!bidib_running) {
		pthread_mutex_lock(&bidib_uplink_queue_mutex);
		if (uplink_queue != NULL) {
			bidib_uplink_queue_reset();
			g_queue_free(uplink_queue);
		}
		pthread_mutex_unlock(&bidib_uplink_queue_mutex);
		syslog(LOG_INFO, "%s", "Message queue freed");
	}
}

void bidib_uplink_error_queue_reset(void) {
	pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
	if (uplink_error_queue != NULL) {
		bidib_message_queue_reset(uplink_error_queue);
	}
	pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
	syslog(LOG_INFO, "%s", "Error message queue reset");
}

void bidib_uplink_error_queue_free(void) {
	if (!bidib_running) {
		pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
		if (uplink_error_queue != NULL) {
			bidib_uplink_error_queue_reset();
			g_queue_free(uplink_error_queue);
		}
		pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
		syslog(LOG_INFO, "%s", "Error message queue freed");
	}
}

void bidib_uplink_intern_queue_reset(void) {
	pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
	if (uplink_intern_queue != NULL) {
		bidib_message_queue_reset(uplink_intern_queue);
	}
	pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
	syslog(LOG_INFO, "%s", "Intern message queue reset");
}

void bidib_uplink_intern_queue_free(void) {
	if (!bidib_running) {
		pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
		if (uplink_intern_queue != NULL) {
			bidib_uplink_intern_queue_reset();
			g_queue_free(uplink_intern_queue);
		}
		pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
		syslog(LOG_INFO, "%s", "Intern message queue freed");
	}
}

static void bidib_message_queue_add(GQueue *queue, unsigned char *message,
                                    unsigned char type, unsigned char *addr_stack) {
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

static void bidib_uplink_queue_add(unsigned char *message, unsigned char type,
                                   unsigned char *addr_stack) {
	pthread_mutex_lock(&bidib_uplink_queue_mutex);
	bidib_message_queue_add(uplink_queue, message, type, addr_stack);
	pthread_mutex_unlock(&bidib_uplink_queue_mutex);
}

static void bidib_uplink_error_queue_add(unsigned char *message, unsigned char type,
                                         unsigned char *addr_stack) {
	pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
	bidib_message_queue_add(uplink_error_queue, message, type, addr_stack);
	pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
}

static void bidib_uplink_intern_queue_add(unsigned char *message, unsigned char type,
                                          unsigned char *addr_stack) {
	pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
	bidib_message_queue_add(uplink_intern_queue, message, type, addr_stack);
	pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
}

static void bidib_log_received_message(unsigned char *addr_stack, unsigned char msg_seqnum,
                                       unsigned char type, int log_level, unsigned char *message,
                                       unsigned int action_id) {
	syslog(log_level, "Received from: 0x%02x 0x%02x 0x%02x 0x%02x seq: %d type: %s "
			       "(0x%02x) action id: %d",
	       addr_stack[0], addr_stack[1], addr_stack[2], addr_stack[3], msg_seqnum,
	       bidib_message_string_mapping[type], type, action_id);
	char hex_string[message[0] * 5];
	bidib_build_message_hex_string(message, hex_string);
	syslog(LOG_DEBUG, "Message bytes: %s", hex_string);
}

static void bidib_log_sys_error(unsigned char error_type) {
	const char *err_name;
	if (error_type <= 0x30) {
		err_name = bidib_error_string_mapping[error_type];
	} else {
		err_name = "UNKNOWN";
	}
	syslog(LOG_ERR, "MSG_SYS_ERROR type: %s (0x%02x)", err_name, error_type);
}

static void bidib_handle_received_message(unsigned char *message, unsigned char type,
                                          unsigned char *addr_stack, unsigned char seqnum,
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

	switch (type) {
		case MSG_PKT_CAPACITY:
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_packet_capacity(message[data_index]);
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
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_cs_state(node_address, message[data_index]);
			free(message);
			break;
		case MSG_CS_DRIVE_ACK:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			bidib_state_cs_drive_ack(dcc_address, message[data_index + 2]);
			free(message);
			break;
		case MSG_CS_ACCESSORY_ACK:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			bidib_state_cs_accessory_ack(node_address, dcc_address,
			                             message[data_index + 2]);
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
			bidib_state_cs_drive(cs_drive_params);
			free(message);
			break;
		case MSG_CS_ACCESSORY_MANUAL:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index];
			dcc_address.addrh = message[data_index + 1];
			bidib_state_cs_accessory_manual(node_address, dcc_address,
			                                message[data_index + 2]);
			free(message);
			break;
		case MSG_LC_STAT:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			peripheral_port.port0 = message[data_index];
			peripheral_port.port1 = message[data_index + 1];
			bidib_state_lc_stat(node_address, peripheral_port, message[data_index + 2]);
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
			pthread_mutex_lock(&bidib_state_boards_mutex);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			if (board != NULL && board->secack_on) {
				bidib_send_bm_mirror_occ(node_address, message[data_index], 0);
				bidib_flush();
			}
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			free(message);
			break;
		case MSG_BM_FREE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_occ(node_address, message[data_index], false);
			pthread_mutex_lock(&bidib_state_boards_mutex);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			if (board != NULL && board->secack_on) {
				bidib_send_bm_mirror_free(node_address, message[data_index], 0);
				bidib_flush();
			}
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			free(message);
			break;
		case MSG_BM_MULTIPLE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_multiple(node_address, message[data_index],
			                        message[data_index + 1], &message[data_index + 2]);
			pthread_mutex_lock(&bidib_state_boards_mutex);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			if (board != NULL && board->secack_on) {
				bidib_send_bm_mirror_multiple(node_address, message[data_index],
				                              message[data_index + 1], &message[data_index + 2], 0);
				bidib_flush();
			}
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			free(message);
			break;
		case MSG_BM_CONFIDENCE:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_confidence(node_address, message[data_index],
			                          message[data_index + 1], message[data_index + 2]);
			free(message);
			break;
		case MSG_BM_ADDRESS:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_bm_address(node_address, message[data_index],
			                       (unsigned char) ((message[0] - data_index) / 2),
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
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			dcc_address.addrl = message[data_index + 1];
			dcc_address.addrh = message[data_index + 2];
			bidib_state_bm_dyn_state(dcc_address, message[data_index + 3],
			                         message[data_index + 4]);
			free(message);
			break;
		case MSG_BOOST_DIAGNOSTIC:
			// update state
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_state_boost_diagnostic(node_address,
			                             (unsigned char) (message[0] - data_index - 1),
			                             &message[data_index]);
			free(message);
			break;
		case MSG_ACCESSORY_STATE:
			// update state and check if error
			bidib_state_accessory_state(node_address, message[data_index],
			                            message[data_index + 1], message[data_index + 2],
			                            message[data_index + 3], message[data_index + 4]);
			if (message[data_index + 3] == BIDIB_ACC_STATE_ERROR) {
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
		case MSG_ACCESSORY_NOTIFY:
			// update state and check if error
			bidib_state_accessory_state(node_address, message[data_index],
			                            message[data_index + 1], message[data_index + 2],
			                            message[data_index + 3], message[data_index + 4]);
			if (message[data_index + 3] == BIDIB_ACC_STATE_ERROR) {
				// add to error queue
				bidib_log_received_message(addr_stack, seqnum, type, LOG_ERR,
				                           message, action_id);
				bidib_uplink_error_queue_add(message, type, addr_stack);
			} else {
				bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
				                           message, action_id);
				free(message);
			}
			bidib_send_accessory_get(node_address, message[data_index], 0);
			break;
		case MSG_BOOST_STAT:
			// update state and check if error
			bidib_state_boost_stat(node_address, message[data_index]);
			if (bidib_booster_normal_to_simple(
					(t_bidib_booster_power_state) message[data_index])
			    == BIDIB_BSTR_SIMPLE_ERROR) {
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
			bidib_log_received_message(addr_stack, seqnum, type, LOG_INFO,
			                           message, action_id);
			bidib_uplink_intern_queue_add(message, type, addr_stack);
			break;
		case MSG_SYS_ERROR:
			// add to error message queue
			bidib_log_received_message(addr_stack, seqnum, type, LOG_ERR,
			                           message, action_id);
			bidib_log_sys_error(message[data_index]);
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
			pthread_mutex_lock(&bidib_state_boards_mutex);
			board = bidib_state_get_board_ref_by_nodeaddr(node_address);
			if (board != NULL && board->secack_on) {
				bidib_send_msg_bm_mirror_position(node_address, message[data_index],
				                                  message[data_index + 1],
				                                  message[data_index + 2], 0);
				bidib_flush();
			}
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			bidib_uplink_queue_add(message, type, addr_stack);
			break;
		case MSG_SYS_PONG:
		case MSG_SYS_P_VERSION:
		case MSG_SYS_UNIQUE_ID:
		case MSG_SYS_SW_VERSION:
		case MSG_SYS_IDENTIFY_STATE:
		case MSG_VENDOR:
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

static void bidib_split_packet(unsigned char *buffer, size_t buffer_size) {
	size_t i = 0;
	while (i < buffer_size) {
		unsigned char *message = malloc(sizeof(unsigned char) * buffer[i] + 1);

		// Read as many bytes as in param LENGTH specified
		size_t j = 0;
		while (j <= buffer[i] && (j + i) < buffer_size) {
			message[j] = buffer[i + j];
			j++;
		}

		unsigned char type = bidib_extract_msg_type(message);
		unsigned char addr_stack[4];
		bidib_extract_address(message, addr_stack);
		unsigned char msg_seqnum = bidib_extract_seq_num(message);

		if (msg_seqnum != 0x00) {
			unsigned char expected_seqnum = bidib_node_state_get_and_incr_receive_seqnum(addr_stack);
			if (msg_seqnum != expected_seqnum) {
				// Handling of wrong sequence numbers
				syslog(LOG_ERR, "Wrong sequence number, expected %d", expected_seqnum);
				if (msg_seqnum == 255) {
					bidib_node_state_set_receive_seqnum(addr_stack, 0x01);
				} else {
					bidib_node_state_set_receive_seqnum(addr_stack, ++msg_seqnum);
				}
			}
		}
		unsigned int action_id = bidib_node_state_update(addr_stack, type);
		bidib_handle_received_message(message, type, addr_stack, msg_seqnum, action_id);
		i += j;
	}
}

static void bidib_receive_packet(void) {
	unsigned char data;
	int read_byte_success = 0;

	unsigned char buffer[READ_BUFFER_SIZE];
	size_t buffer_index = 0;
	bool escape_hot = false;
	unsigned char crc = 0x00;

	// Read the packet bytes
	while (bidib_running) {
		data = read_byte(&read_byte_success);
		while (!read_byte_success) {
			usleep(5000);
			data = read_byte(&read_byte_success);
			if (!bidib_running) {
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

	syslog(LOG_INFO, "%s", "Received packet");

	if (!bidib_running) {
		return;
	} else if (crc == 0x00) {
		syslog(LOG_INFO, "%s", "CRC correct, split packet in messages");
		// Split packet in messages and add them to queue, exclude crc sum
		buffer_index--;
		bidib_split_packet(buffer, buffer_index);
	} else {
		syslog(LOG_ERR, "%s", "CRC wrong, packet ignored");
	}
}

static void bidib_receive_first_pkt_magic(void) {
	unsigned char data;
	int read_byte_success = 0;

	while (bidib_running) {
		data = read_byte(&read_byte_success);
		while (!read_byte_success) {
			usleep(10000);
			data = read_byte(&read_byte_success);
			if (!bidib_running) {
				return;
			}
		}
		read_byte_success = 0;
		if (data == BIDIB_PKT_MAGIC) {
			return;
		}
	}
}

void *bidib_auto_receive(void *par) {
	bidib_receive_first_pkt_magic();
	while (bidib_running) {
		bidib_receive_packet();
	}
	return NULL;
}

static unsigned char *bidib_read_message_from_queue(GQueue *queue) {
	unsigned char *message = NULL;
	if (!g_queue_is_empty(queue)) {
		t_bidib_message_queue_entry *entry = g_queue_pop_head(queue);
		message = entry->message;
		free(entry);
	}
	return message;
}

unsigned char *bidib_read_message(void) {
	unsigned char *message;
	pthread_mutex_lock(&bidib_uplink_queue_mutex);
	message = bidib_read_message_from_queue(uplink_queue);
	pthread_mutex_unlock(&bidib_uplink_queue_mutex);
	return message;
}

unsigned char *bidib_read_error_message(void) {
	unsigned char *error_message;
	pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
	error_message = bidib_read_message_from_queue(uplink_error_queue);
	pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
	return error_message;
}

unsigned char *bidib_read_intern_message(void) {
	unsigned char *intern_message;
	pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
	intern_message = bidib_read_message_from_queue(uplink_intern_queue);
	pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
	return intern_message;
}
