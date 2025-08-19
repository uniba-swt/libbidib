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

#include <glib.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <time.h>
#include <stdint.h>

#include "bidib_transmission_intern.h"
#include "../../include/highlevel/bidib_highlevel_util.h"

#define RESPONSE_QUEUE_EXPIRATION_SECS 2

pthread_mutex_t bidib_node_state_table_mutex;

static GHashTable *node_state_table = NULL;
// Limit for the number of bytes expected in form of responses from a node.
// (to avoid node overload)
static int response_limit = 48;

void bidib_node_state_table_init() {
	node_state_table = g_hash_table_new(g_str_hash, g_str_equal);
}

static void bidib_node_state_add_response(uint8_t type, t_bidib_node_state *state,
                                          int message_max_resp, unsigned int action_id) {
	if (message_max_resp > 0) {
		state->current_response_bytes += message_max_resp;
		t_bidib_response_queue_entry *response = malloc(sizeof(t_bidib_response_queue_entry));
		response->type = type;
		response->creation_time = time(NULL);
		response->action_id = action_id;
		g_queue_push_tail(state->response_queue, response);
	}
}

static void bidib_node_state_add_message(const uint8_t *const addr_stack, uint8_t type,
                                         const uint8_t *const message, t_bidib_node_state *state,
                                         unsigned int action_id) {
	t_bidib_message_queue_entry *message_entry = malloc(sizeof(t_bidib_message_queue_entry));
	message_entry->type = type;
	memcpy(message_entry->addr, addr_stack, 4);
	message_entry->message = malloc(sizeof(uint8_t) * (message[0] + 1));
	memcpy(message_entry->message, message, message[0] + 1);
	message_entry->action_id = action_id;
	g_queue_push_tail(state->message_queue, message_entry);
	syslog_libbidib(LOG_DEBUG, 
	                "Enqueued msg with type: %s to: 0x%02x 0x%02x 0x%02x 0x%02x action id: %d",
	                bidib_message_string_mapping[type], addr_stack[0], addr_stack[1], addr_stack[2], 
	                addr_stack[3], action_id);
}

// May write to member in node_state_table.
static t_bidib_node_state *bidib_node_query(const uint8_t *const addr_stack) {
	t_bidib_node_state *state = g_hash_table_lookup(node_state_table, addr_stack);
	// Check whether node is already registered in the table
	if (state == NULL) {
		state = malloc(sizeof(t_bidib_node_state));
		memcpy(state->addr, addr_stack, 4);
		state->receive_seqnum = 0x01;
		state->send_seqnum = 0x01;
		state->stall = false;
		state->current_response_bytes = 0;
		state->stall_affected_nodes_queue = g_queue_new();
		state->response_queue = g_queue_new();
		state->message_queue = g_queue_new();
		g_hash_table_insert(node_state_table, state->addr, state);
		syslog_libbidib(LOG_DEBUG, "Add to node state table: 0x%02x 0x%02x 0x%02x 0x%02x",
		                addr_stack[0], addr_stack[1], addr_stack[2], addr_stack[3]);
	}
	return state;
}

static int bidib_node_stall_queue_entry_equals(const t_bidib_stall_queue_entry *const elem,
                                               const t_bidib_node_state *const searched) {
	if (elem->addr[0] == searched->addr[0] && elem->addr[1] == searched->addr[1] &&
	    elem->addr[2] == searched->addr[2] && elem->addr[3] == searched->addr[3]) {
		return 0;
	}
	return 1;
}

/**
 * Checks if the node at addr_stack or any of its super-nodes are NOT stalled.
 * If the node or any of its super-nodes is stalled, adds the node at addr_stack to the 
 * stall_affected_nodes_queue of the stalled node.
 * 
 * @param addr_stack the node for which to check if it or any of its super-nodes is NOT stalled
 * @return true if no super-node is stalled and the node itself is not stalled
 * @return false otherwise
 */
static bool bidib_node_stall_ready(const uint8_t *const addr_stack) {
	uint8_t addr_cpy[4];
	memcpy(addr_cpy, addr_stack, 4);
	while (addr_cpy[0] != 0x00) {
		t_bidib_node_state *state = g_hash_table_lookup(node_state_table, addr_cpy);
		// Is the node at the address of current addr_cpy stalled?
		if (state != NULL && state->stall) {
			// Node at addr_cpy is stalled -> search its stall_affected_nodes_queue to see
			// if the queue contains the (node at) addr_stack.
			if (!g_queue_find_custom(state->stall_affected_nodes_queue, addr_stack, 
			                         (GCompareFunc)bidib_node_stall_queue_entry_equals)) 
			{
				// stalled subnode (addr_stack) is not yet in stall_affected_nodes_queue, 
				// so add it
				t_bidib_stall_queue_entry *stall_entry = malloc(sizeof(t_bidib_stall_queue_entry));
				memcpy(stall_entry->addr, addr_stack, 4);
				g_queue_push_tail(state->stall_affected_nodes_queue, stall_entry);
			}
			return false;
		}
		// search next super-node by setting the last non-zero addr_cpy byte to 0.
		for (int i = 2; i >= 0; i--) {
			if (addr_cpy[i] != 0x00) {
				addr_cpy[i] = 0x00;
				break;
			}
		}
	}
	return true;
}

bool bidib_node_try_send(const uint8_t *const addr_stack, uint8_t type,
                         const uint8_t *const message, unsigned int action_id) {
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	t_bidib_node_state *state = bidib_node_query(addr_stack);
	int max_response = bidib_response_info[type][1];
	bool status;
	if (bidib_node_stall_ready(addr_stack) && g_queue_is_empty(state->message_queue) &&
	    state->current_response_bytes + max_response <= response_limit) {
		// Node is ready
		bidib_node_state_add_response(type, state, max_response, action_id);
		status = true;
		syslog_libbidib(LOG_DEBUG, 
		                "Expecting responses with a total of %d bytes from 0x%02x 0x%02x 0x%02x 0x%02x"
						" after sending msg of type %s with action id: %d",
		                state->current_response_bytes, addr_stack[0], addr_stack[1], addr_stack[2], 
		                addr_stack[3], bidib_message_string_mapping[type], action_id);
	} else {
		// Node is not ready
		bidib_node_state_add_message(addr_stack, type, message, state, action_id);
		status = false;
	}
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
	return status;
}

// Returns the number of sent (dequeued) messages
static int bidib_node_try_queued_messages(t_bidib_node_state *state) {
	if (state == NULL) {
		syslog_libbidib(LOG_WARNING, "bidib_node_try_queued_messages - Called with NULL state");
		return 0;
	}
	int sent_count = 0;
	while (bidib_node_stall_ready((uint8_t *) state->addr) &&
	       !g_queue_is_empty(state->message_queue)) {
		t_bidib_message_queue_entry *queued_msg = g_queue_peek_head(state->message_queue);
		if (state->current_response_bytes + bidib_response_info[queued_msg->type][1] <= response_limit) {
			// capacity sufficient -> send messages
			bidib_node_state_add_response(queued_msg->type, state,
			                              bidib_response_info[queued_msg->type][1],
			                              queued_msg->action_id);
			bidib_add_to_buffer(queued_msg->message);
			syslog_libbidib(LOG_DEBUG, 
			                "Dequeued msg with type: %s to: 0x%02x 0x%02x 0x%02x 0x%02x action id: %d",
			                bidib_message_string_mapping[queued_msg->type], state->addr[0], 
			                state->addr[1], state->addr[2], state->addr[3], queued_msg->action_id);
			g_queue_pop_head(state->message_queue);
			free(queued_msg->message);
			free(queued_msg);
			sent_count++;
		} else {
			syslog_libbidib(LOG_DEBUG, 
			                "Unable to dequeue msg, response queue full. Msg info: "
			                "type: %s to: 0x%02x 0x%02x 0x%02x 0x%02x action id: %d. "
			                "Current response bytes: %d; size of response to add: %d",
			                bidib_message_string_mapping[queued_msg->type], 
			                state->addr[0], state->addr[1], state->addr[2], state->addr[3], 
			                queued_msg->action_id, state->current_response_bytes, 
			                bidib_response_info[queued_msg->type][1]);
			break;
		}
	}
	if (sent_count > 0) {
		bidib_flush();
	}
	return sent_count;
}

unsigned int bidib_node_state_update(const uint8_t *const addr_stack, uint8_t response_type) {
	unsigned int action_id = 0;
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	t_bidib_node_state *state = g_hash_table_lookup(node_state_table, addr_stack);

	if (state != NULL && !g_queue_is_empty(state->response_queue)) {
		// node in table and awaiting answers
		t_bidib_response_queue_entry *response = g_queue_peek_head(state->response_queue);
		time_t current_time = time(NULL);
		int sent_msgs = 0;
		// Iterate over the response types that are expected for the response at the front of the 
		// queue and see if any of them match the actual received response type.
		// Also check if the response at the front of the queue is expired/stale, 
		// in which case it is removed.
		for (int i = 2; i <= bidib_response_info[response->type][0]; i++) {
			if (bidib_response_info[response->type][i] == response_type) {
				// awaited answer matches message -> extend free capacity
				g_queue_pop_head(state->response_queue);
				state->current_response_bytes -= bidib_response_info[response->type][1];
				action_id = response->action_id;
				free(response);
				response = NULL;
				sent_msgs += bidib_node_try_queued_messages(state);
				break;
			} else if (difftime(current_time, response->creation_time) >=
			           RESPONSE_QUEUE_EXPIRATION_SECS) {
				// remove response queue entries older than x seconds
				syslog_libbidib(LOG_ERR,
				                "Response from: 0x%02x 0x%02x 0x%02x 0x%02x to type: %s "
				                "with action id: %d expected but not received within %d s",
				                addr_stack[0], addr_stack[1], addr_stack[2], addr_stack[3],
				                bidib_message_string_mapping[response->type], response->action_id,
				                RESPONSE_QUEUE_EXPIRATION_SECS);
				g_queue_pop_head(state->response_queue);
				state->current_response_bytes -= bidib_response_info[response->type][1];
				free(response);
				response = NULL;
				if (!g_queue_is_empty(state->response_queue)) {
					/// TODO: This does not restart the for-loop. Only in very specific
					// circumstances (head of response_queue has 2 possible response msg types) 
					// will this cause the now new head of the response to be properly considered.
					response = g_queue_peek_head(state->response_queue);
				} else {
					break;
				}
			}
		}
		syslog_libbidib(LOG_DEBUG, 
		                "Expecting responses with a total of %d bytes from 0x%02x 0x%02x 0x%02x 0x%02x"
		                " after receiving message of type %s with action id: %d "
		                "and sending (dequeing) %d messages",
		                state->current_response_bytes, addr_stack[0], addr_stack[1], addr_stack[2], 
		                addr_stack[3], bidib_message_string_mapping[response_type], action_id, sent_msgs);
	}
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
	return action_id;
}

void bidib_node_update_stall(const uint8_t *const addr_stack, uint8_t stall_status) {
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	t_bidib_node_state *state = bidib_node_query(addr_stack);
	if (stall_status == 0x00) {
		state->stall = false;
		syslog_libbidib(LOG_WARNING, "Stall inactive for: 0x%02x 0x%02x 0x%02x 0x%02x",
		                addr_stack[0], addr_stack[1], addr_stack[2], addr_stack[3]);
		t_bidib_stall_queue_entry *elem;
		// Node is not stalled anymore. Therefore, for all nodes in the stall_affected_nodes_queue,
		// i.e. nodes that were stalled because this/their supernode was stalled,
		// try to send any queued messages.
		while (!g_queue_is_empty(state->stall_affected_nodes_queue)) {
			elem = g_queue_pop_head(state->stall_affected_nodes_queue);
			t_bidib_node_state *waiting_node_state = g_hash_table_lookup(
					node_state_table, elem->addr);
			if (waiting_node_state != NULL) {
				bidib_node_try_queued_messages(waiting_node_state);
			}
			free(elem);
			elem = NULL;
		}
	} else {
		state->stall = true;
		syslog_libbidib(LOG_WARNING, "Stall active for: 0x%02x 0x%02x 0x%02x 0x%02x",
		                addr_stack[0], addr_stack[1], addr_stack[2], addr_stack[3]);
	}
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
}

static uint8_t bidib_get_and_incr_seqnum(uint8_t *seqnum) {
	if (*seqnum == 255) {
		*seqnum = 0x01;
		return 255;
	}
	return (*seqnum)++;
}

uint8_t bidib_node_state_get_and_incr_receive_seqnum(const uint8_t *const addr_stack) {
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	t_bidib_node_state *state = bidib_node_query(addr_stack);
	uint8_t seqnum = bidib_get_and_incr_seqnum(&state->receive_seqnum);
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
	return seqnum;
}

uint8_t bidib_node_state_get_and_incr_send_seqnum(const uint8_t *const addr_stack) {
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	t_bidib_node_state *state = bidib_node_query(addr_stack);
	uint8_t seqnum = bidib_get_and_incr_seqnum(&state->send_seqnum);
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
	return seqnum;
}

void bidib_node_state_set_receive_seqnum(const uint8_t *const addr_stack, uint8_t seqnum) {
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	t_bidib_node_state *state = bidib_node_query(addr_stack);
	state->receive_seqnum = seqnum;
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
}

void bidib_node_state_table_reset(bool lock_node_state_table_access) {
	GHashTableIter iter;
	uint8_t *key;
	t_bidib_node_state *value;
	if (lock_node_state_table_access) {
		pthread_mutex_lock(&bidib_node_state_table_mutex);
	}
	g_hash_table_iter_init(&iter, node_state_table);
	while (g_hash_table_iter_next(&iter, (gpointer) &key, (gpointer) &value)) {
		if (value != NULL) {
			while (!g_queue_is_empty(value->stall_affected_nodes_queue)) {
				t_bidib_stall_queue_entry *elem0 = g_queue_pop_head(value->stall_affected_nodes_queue);
				free(elem0);
			}
			g_queue_free(value->stall_affected_nodes_queue);
			while (!g_queue_is_empty(value->response_queue)) {
				t_bidib_response_queue_entry *elem1 = g_queue_pop_head(value->response_queue);
				free(elem1);
			}
			g_queue_free(value->response_queue);
			while (!g_queue_is_empty(value->message_queue)) {
				t_bidib_message_queue_entry *elem2 = g_queue_pop_head(value->message_queue);
				free(elem2->message);
				free(elem2);
			}
			g_queue_free(value->message_queue);
			free(value);
			value = NULL;
			g_hash_table_iter_remove(&iter);
		}
	}
	if (lock_node_state_table_access) {
		pthread_mutex_unlock(&bidib_node_state_table_mutex);
	}
	syslog_libbidib(LOG_INFO, "Node state table reset");
}

void bidib_node_state_table_free(void) {
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	if (node_state_table != NULL) {
		bidib_node_state_table_reset(false);
		g_hash_table_destroy(node_state_table);
	}
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
	syslog_libbidib(LOG_INFO, "Node state table freed");
}
