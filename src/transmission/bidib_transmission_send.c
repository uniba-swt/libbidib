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

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>

#include "bidib_transmission_intern.h"
#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../../include/definitions/bidib_messages.h"

#define PACKET_BUFFER_SIZE 256
#define PACKET_BUFFER_AUX_SIZE 312


pthread_mutex_t bidib_send_buffer_mutex;

static void (*write_bytes)(uint8_t*, int32_t);

volatile bool bidib_seq_num_enabled = true;
static volatile unsigned int pkt_max_cap = 64;
static volatile uint8_t buffer[PACKET_BUFFER_SIZE];
static volatile uint8_t buffer_aux[PACKET_BUFFER_AUX_SIZE];
static volatile size_t buffer_index = 0;

void bidib_set_write_n_dest(void (*write_n)(uint8_t*, int32_t)) {
	write_bytes = write_n;
	syslog_libbidib(LOG_INFO, "write_bytes function was set");
}

void bidib_state_packet_capacity(uint8_t max_capacity) {
	pthread_mutex_lock(&bidib_send_buffer_mutex);
	if (max_capacity <= 64) {
		pkt_max_cap = 64;
	} else {
		pkt_max_cap = max_capacity;
	}
	syslog_libbidib(LOG_INFO, "Maximum packet size was set to %d bytes", 
	                pkt_max_cap);
	pthread_mutex_unlock(&bidib_send_buffer_mutex);
}

/**
 * Will flush the send cache, if possible in one go (as one batch);
 * if batching not possible, then send byte-per-byte.
 * Shall only be called with bidib_send_buffer_mutex locked.
 * 
 * How does it work? It copies the actual buffer (called `buffer`)
 * byte-per-byte to an auxiliary buffer (called `buffer_aux`), 
 * and whilst doing so it computes the crc and adds/inserts the crc and escape
 * related bytes/chars where needed. 
 * Due the need to insert bytes (e.g., related to the crc or escapes), we can't
 * modify the `buffer` in-place. The solution with an auxiliary buffer
 * has been compared against solutions with dynamic memory allocation,
 * where the auxiliary buffer solution has shown to be faster, more timing-predictable,
 * and easier to maintain/less risk of a memory leak.
 * 
 */
static void bidib_flush_impl(void) { 
	struct timespec start, end1, end2;
	
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	if (buffer_index > 0) {
		uint8_t crc = 0;
		int32_t aux_index = 0;
		// BIDIB_PKT_MAGIC marks the (starting) delimiter 
		buffer_aux[aux_index++] = BIDIB_PKT_MAGIC;
		for (size_t i = 0; i < buffer_index; ++i) {
			// At most 2 more chars can be added in one loop iteration
			if (aux_index + 3 >= PACKET_BUFFER_AUX_SIZE) {
				// aux_buffer full. Send its current contents and continue.
				write_bytes((uint8_t*)buffer_aux, aux_index);
				// reset the aux_index but NOT the buffer_index, 
				// as we want to continue sending the rest of the buffer
				aux_index = 0;
			}
			crc = bidib_crc_array[buffer[i] ^ crc];
			if (buffer[i] == BIDIB_PKT_MAGIC || buffer[i] == BIDIB_PKT_ESCAPE) {
				buffer_aux[aux_index++] = BIDIB_PKT_ESCAPE;
				buffer_aux[aux_index++] = buffer[i] ^ (uint8_t) 0x20;
			} else {
				buffer_aux[aux_index++] = buffer[i];
			}
		}
		
		// At most 3 more chars can be added in the code below
		if (aux_index + 4 >= PACKET_BUFFER_AUX_SIZE) {
			// aux_buffer full. Send its current contents and continue.
			write_bytes((uint8_t*)buffer_aux, aux_index);
			// reset the aux_index but NOT the buffer_index, 
			// as we want to continue sending the rest of the buffer
			aux_index = 0;
		}
		
		// send crc byte (+ escape if necessary)
		if (crc == BIDIB_PKT_MAGIC || crc == BIDIB_PKT_ESCAPE) {
			buffer_aux[aux_index++] = BIDIB_PKT_ESCAPE;
			buffer_aux[aux_index++] = crc ^ (uint8_t) 0x20;
		} else {
			buffer_aux[aux_index++] = crc;
		}
		// BIDIB_PKT_MAGIC marks the (end) delimiter 
		buffer_aux[aux_index++] = BIDIB_PKT_MAGIC;
		
		// Batch-write aux buffer and reset the buffer index.
		write_bytes((uint8_t*)buffer_aux, aux_index);
		
		buffer_index = 0;
	}
	clock_gettime(CLOCK_MONOTONIC_RAW, &end1);
	syslog_libbidib(LOG_DEBUG, "Cache flushed");
	clock_gettime(CLOCK_MONOTONIC_RAW, &end2);
	uint64_t flush_us = (end1.tv_sec - start.tv_sec) * 1000000 + (end1.tv_nsec - start.tv_nsec) / 1000;
	uint64_t log_us = (end2.tv_sec - end1.tv_sec) * 1000000 + (end2.tv_nsec - end1.tv_nsec) / 1000;
	// longer than 0.01s
	if (flush_us + log_us > 10000) {
		syslog_libbidib(LOG_WARNING, "bidib_flush_impl took longer than 10000 us");
		syslog_libbidib(LOG_WARNING, "     Flushing took %llu us", flush_us);
		syslog_libbidib(LOG_WARNING, "Logging debug took %llu us", log_us);
	}
}

void bidib_flush(void) {
	pthread_mutex_lock(&bidib_send_buffer_mutex);
	bidib_flush_impl();
	pthread_mutex_unlock(&bidib_send_buffer_mutex);
}

void *bidib_auto_flush(void *interval) {
	while (bidib_running) {
		pthread_mutex_lock(&bidib_send_buffer_mutex);
		bidib_flush_impl();
		pthread_mutex_unlock(&bidib_send_buffer_mutex);
		unsigned int interval_ms = 1000 * *((unsigned int *) (interval));
		usleep(interval_ms);
	}
	free(interval);
	return NULL;
}

void bidib_add_to_buffer(const uint8_t *const message) {
	pthread_mutex_lock(&bidib_send_buffer_mutex);
	if (message[0] + 1 + buffer_index > pkt_max_cap) {
		// Not enough space for this message -> flush
		bidib_flush_impl();
	}

	memcpy((uint8_t *)buffer + buffer_index, message, message[0] + 1);
	buffer_index += message[0] + 1;

	if (buffer_index > pkt_max_cap - 4) {
		// Not enough space for another message -> flush
		bidib_flush_impl();
	}
	pthread_mutex_unlock(&bidib_send_buffer_mutex);
}

static void bidib_log_send_message(uint8_t message_type, const uint8_t *const addr_stack,
                                   uint8_t seqnum, const uint8_t *const message,
                                   unsigned int action_id) {
	syslog_libbidib(LOG_DEBUG, "Send to: 0x%02x 0x%02x 0x%02x 0x%02x seq: %d "
	                "type: %s (0x%02x) action id: %d",
	                addr_stack[0], addr_stack[1], addr_stack[2], addr_stack[3], seqnum,
	                bidib_message_string_mapping[message_type], message_type, action_id);
	char hex_string[(message[0] + 1) * 5];
	bidib_build_message_hex_string(message, hex_string);
	syslog_libbidib(LOG_DEBUG, "Message bytes to send: %s", hex_string);
}

static void bidib_buffer_message(uint8_t seqnum, uint8_t type,
                                 const uint8_t *const message, unsigned int action_id) {
	uint8_t addr[4];
	bidib_extract_address(message, addr);
	bidib_log_send_message(type, addr, seqnum, message, action_id);
	if (bidib_node_try_send(addr, type, message, action_id)) {
		// Node is ready -> Put in send buffer
		// If node is not ready, the node enqueues the message so nothing else to do here.
		bidib_add_to_buffer(message);
	}
}

void bidib_buffer_message_without_data(const uint8_t *const addr_stack, uint8_t msg_type,
                                       unsigned int action_id) {
	// Determine message size
	uint8_t message_length = 0;
	uint8_t addr_stack_size = 0;
	if (addr_stack[0] == 0x00) {
		message_length = 4;
		addr_stack_size = 1;
	} else if (addr_stack[1] == 0x00) {
		message_length = 5;
		addr_stack_size = 2;
	} else if (addr_stack[2] == 0x00) {
		message_length = 6;
		addr_stack_size = 3;
	} else {
		message_length = 7;
		addr_stack_size = 4;
	}

	// Build message
	uint8_t message[message_length];
	message[0] = message_length - (uint8_t) 1;
	for (int i = 1; i <= addr_stack_size; i++) {
		message[i] = addr_stack[i - 1];
	}
	uint8_t seqnum = 0x00;
	if (bidib_seq_num_enabled) {
		seqnum = bidib_node_state_get_and_incr_send_seqnum(addr_stack);
	}
	message[addr_stack_size + 1] = seqnum;
	message[addr_stack_size + 2] = msg_type;

	// Buffer message
	bidib_buffer_message(seqnum, msg_type, message, action_id);
}

void bidib_buffer_message_with_data(const uint8_t *const addr_stack, uint8_t msg_type,
                                    uint8_t data_length, const uint8_t *const data,
                                    unsigned int action_id) {
	// Determine message size
	uint8_t message_length = data_length;
	uint8_t addr_stack_size = 0;
	if (addr_stack[0] == 0x00) {
		message_length += 4;
		addr_stack_size = 1;
	} else if (addr_stack[1] == 0x00) {
		message_length += 5;
		addr_stack_size = 2;
	} else if (addr_stack[2] == 0x00) {
		message_length += 6;
		addr_stack_size = 3;
	} else {
		message_length += 7;
		addr_stack_size = 4;
	}

	// Build message
	uint8_t message[message_length];
	message[0] = message_length - (uint8_t) 1;
	for (size_t i = 1; i <= addr_stack_size; i++) {
		message[i] = addr_stack[i - 1];
	}
	uint8_t seqnum = 0x00;
	if (bidib_seq_num_enabled) {
		seqnum = bidib_node_state_get_and_incr_send_seqnum(addr_stack);
	}
	message[addr_stack_size + 1] = seqnum;
	message[addr_stack_size + 2] = msg_type;
	for (size_t i = 0; i < data_length; i++) {
		message[addr_stack_size + 3 + i] = data[i];
	}

	// Buffer message
	bidib_buffer_message(seqnum, msg_type, message, action_id);
}
