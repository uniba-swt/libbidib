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
// Experiment? else delete. 
#define PACKET_BUFFER_AUX_SIZE 312


pthread_mutex_t bidib_send_buffer_mutex;

static void (*write_byte)(uint8_t);
static void (*write_bytes)(uint8_t*, int32_t);

volatile bool bidib_seq_num_enabled = true;
static volatile unsigned int pkt_max_cap = 64;
static volatile uint8_t buffer[PACKET_BUFFER_SIZE];
static volatile uint8_t buffer_aux[PACKET_BUFFER_AUX_SIZE];
static volatile size_t buffer_index = 0;

typedef struct {
	uint8_t *message;
	size_t len;
} t_bidib_send_buff_arr;

void bidib_set_write_dest(void (*write)(uint8_t)) {
	write_byte = write;
	syslog_libbidib(LOG_INFO, "%s", "Write function was set");
}

void bidib_set_write_n_dest(void (*write_n)(uint8_t*, int32_t)) {
	write_bytes = write_n;
	syslog_libbidib(LOG_INFO, "%s", "Write_n function was set");
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

static void bidib_send_byte(uint8_t b) {
	if ((b == BIDIB_PKT_MAGIC) || (b == BIDIB_PKT_ESCAPE)) {
		write_byte(BIDIB_PKT_ESCAPE);
		b = b ^ (uint8_t) 0x20;
	}
	write_byte(b);
}

/*
static size_t bidib_send_to_aux_buffer(uint8_t b, size_t aux_index) {
	if (aux_index+2 >= PACKET_BUFFER_AUX_SIZE) {
		// likely not enough space. (Pessimistic +2 because it might be escaped)
		return 0;
	}
	size_t written = 1;
	if ((b == BIDIB_PKT_MAGIC) || (b == BIDIB_PKT_ESCAPE)) {
		buffer_aux[aux_index++] = (uint8_t) BIDIB_PKT_ESCAPE;
		b = b ^ (uint8_t) 0x20;
		written++;
	}
	buffer_aux[aux_index++] = b;
	return written;
}*/

static t_bidib_send_buff_arr bidib_construct_sendbuffer_for_batch_write(size_t counted_escapes) {
	if (buffer_index <= 0) {
		return (t_bidib_send_buff_arr){NULL, 0};
	}
	// Need to allocate buffer_index + escape_count bytes
	t_bidib_send_buff_arr ret = {NULL, 0};
	ret.len = buffer_index + counted_escapes;
	ret.message = malloc(sizeof(uint8_t) * ret.len);
	if (counted_escapes == 0) {
		memcpy(ret.message, (uint8_t*)buffer, sizeof(uint8_t) * ret.len);
	} else {
		size_t msg_i = 0;
		for (size_t i = 0; i < buffer_index; ++i) {
			uint8_t b = buffer[i];
			if (b == BIDIB_PKT_MAGIC || b == BIDIB_PKT_ESCAPE) {
				ret.message[msg_i++] = BIDIB_PKT_ESCAPE;
				b = b ^ (uint8_t) 0x20;
			}
			ret.message[msg_i++] = b;
		}
	}
	return ret;
}

static void bidib_send_delimiter(void) {
	write_byte(BIDIB_PKT_MAGIC);
}

// Must be called with bidib_send_buffer_mutex locked.
static void bidib_flush_impl(void) {
	if (buffer_index > 0) {
		uint8_t crc = 0;
		bidib_send_delimiter();
		for (size_t i = 0; i < buffer_index; i++) {
			bidib_send_byte(buffer[i]);
			crc = bidib_crc_array[buffer[i] ^ crc];
		}
		bidib_send_byte(crc);
		bidib_send_delimiter();
		// Could be optimized to use end-delimiter of last message also as
		// start-delimiter for next one
		buffer_index = 0;
	}
	//syslog_libbidib(LOG_DEBUG, "%s", "Cache flushed");
}

// Must be called with bidib_send_buffer_mutex locked.
static void bidib_flush_batch_impl0(void) {
	if (buffer_index > 0) {
		uint8_t crc = 0;
		int32_t aux_index = 0;
		buffer_aux[aux_index++] = BIDIB_PKT_MAGIC; // send_delimiter equiv
		for (size_t i = 0; i < buffer_index; ++i) {
			if (aux_index + 2 >= PACKET_BUFFER_AUX_SIZE) {
				// Too big for flush_batch. Fallback to traditional one-by-one send.
				bidib_flush_impl();
				return;
			}
			crc = bidib_crc_array[buffer[i] ^ crc];
			if (buffer[i] == BIDIB_PKT_MAGIC || buffer[i] == BIDIB_PKT_ESCAPE) {
				buffer_aux[aux_index++] = BIDIB_PKT_ESCAPE;
				buffer_aux[aux_index++] = buffer[i] ^ (uint8_t) 0x20;
			} else {
				buffer_aux[aux_index++] = buffer[i];
			}
		}
		// pessimistic remaining size check
		if (aux_index + 3 >= PACKET_BUFFER_AUX_SIZE) {
			// Too big for flush_batch. Fallback to traditional one-by-one send.
			bidib_flush_impl();
			return;
		}
		
		// send crc byte (+ escape if necessary)
		if (crc == BIDIB_PKT_MAGIC || crc == BIDIB_PKT_ESCAPE) {
			buffer_aux[aux_index++] = BIDIB_PKT_ESCAPE;
			buffer_aux[aux_index++] = crc ^ (uint8_t) 0x20;
		} else {
			buffer_aux[aux_index++] = crc;
		}
		buffer_aux[aux_index++] = BIDIB_PKT_MAGIC; // send_delimiter equiv
		
		write_bytes((uint8_t*)buffer_aux, aux_index);
		
		buffer_index = 0;
	}
}

// Must be called with bidib_send_buffer_mutex locked.
static void bidib_flush_batch_impl1(void) {
	if (buffer_index > 0) {
		uint8_t crc = 0;
		bidib_send_delimiter();
		size_t counted_escapes = 0;
		for (size_t i = 0; i < buffer_index; i++) {
			if (buffer[i] == BIDIB_PKT_MAGIC || buffer[i] == BIDIB_PKT_ESCAPE) {
				counted_escapes++;
			}
			crc = bidib_crc_array[buffer[i] ^ crc];
		}
		t_bidib_send_buff_arr send_t = bidib_construct_sendbuffer_for_batch_write(counted_escapes);
		write_bytes(send_t.message, (int32_t)send_t.len);
		bidib_send_byte(crc);
		bidib_send_delimiter();
		if (send_t.message != NULL) {
			free(send_t.message);
		}
		// Could be optimized to use end-delimiter of last message also as
		// start-delimiter for next one
		buffer_index = 0;
	}
	//syslog_libbidib(LOG_DEBUG, "%s", "Cache flushed");
}

// Must be called with bidib_send_buffer_mutex locked.
static void bidib_flush_batch_impl2(void) {
	if (buffer_index > 0) {
		uint8_t crc = 0;
		size_t counted_escapes = 0;
		for (size_t i = 0; i < buffer_index; i++) {
			if (buffer[i] == BIDIB_PKT_MAGIC || buffer[i] == BIDIB_PKT_ESCAPE) {
				counted_escapes++;
			}
			crc = bidib_crc_array[buffer[i] ^ crc];
		}
		
		t_bidib_send_buff_arr send_t = {NULL,0};
		// delim + buffer content + escape bytes
		send_t.len = buffer_index + counted_escapes + 1;
		send_t.message = malloc(sizeof(uint8_t) * send_t.len);
		if (send_t.message == NULL) {
			syslog_libbidib(LOG_ERR, "Cache flush failed, alloc failed");
			return;
		}
		size_t msg_i = 0;
		send_t.message[msg_i++] = (uint8_t) BIDIB_PKT_MAGIC; // equiv bidib_send_delimiter();
		if (counted_escapes == 0) {
			memcpy(&send_t.message[msg_i], (uint8_t*)buffer, sizeof(uint8_t) * buffer_index);
			msg_i = msg_i + buffer_index;
		} else {
			for (size_t i = 0; i < buffer_index; ++i) {
				uint8_t b = buffer[i];
				if (b == BIDIB_PKT_MAGIC || b == BIDIB_PKT_ESCAPE) {
					send_t.message[msg_i++] = BIDIB_PKT_ESCAPE;
					b = b ^ (uint8_t) 0x20;
				}
				send_t.message[msg_i++] = b;
			}
		}
		write_bytes(send_t.message, (int32_t)send_t.len);
		// Can't add crc to send_t.message without potential realloc if CRC happens to 
		// have the value of BIDIB_PKT_MAGIC or BIDIB_PKT_ESCAPE.
		bidib_send_byte(crc);
		bidib_send_delimiter();
		buffer_index = 0;
		if (send_t.message != NULL) {
			free(send_t.message);
		}
	}
	//syslog_libbidib(LOG_DEBUG, "%s", "Cache flushed");
}

// Must be called with bidib_send_buffer_mutex locked.
static void bidib_flush_batch_impl3(void) {
	if (buffer_index > 0) {
		uint8_t crc = 0;
		bidib_send_delimiter();
		size_t last_non_escaped_index = 0;
		for (size_t i = 0; i < buffer_index; i++) {
			if (buffer[i] == BIDIB_PKT_MAGIC || buffer[i] == BIDIB_PKT_ESCAPE) {
				if (last_non_escaped_index == i) {
					// move index further as this index actually has to be escaped.
					last_non_escaped_index = i + 1;
				} else {
					write_bytes((uint8_t*) buffer + last_non_escaped_index, i - last_non_escaped_index);
				}
				
				bidib_send_byte(buffer[i]);
				
				last_non_escaped_index = i + 1;
			}
			crc = bidib_crc_array[buffer[i] ^ crc];
		}
		if (last_non_escaped_index == 0) {
			write_bytes((uint8_t*) buffer, buffer_index);
		} else if (last_non_escaped_index < buffer_index) {
			write_bytes((uint8_t*) buffer+last_non_escaped_index, buffer_index - last_non_escaped_index);
		} else {
			// All bytes from buffer already written.
		}
		bidib_send_byte(crc);
		bidib_send_delimiter();
		// Could be optimized to use end-delimiter of last message also as
		// start-delimiter for next one
		buffer_index = 0;
	}
	//syslog_libbidib(LOG_DEBUG, "%s", "Cache flushed");
}

void bidib_flush(void) {
	struct timespec start, end;
	pthread_mutex_lock(&bidib_send_buffer_mutex);
	clock_gettime(CLOCK_MONOTONIC_RAW, &start);
	bidib_flush_impl();
	clock_gettime(CLOCK_MONOTONIC_RAW, &end);
	pthread_mutex_unlock(&bidib_send_buffer_mutex);
	uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
	syslog_libbidib(LOG_WARNING, "Flush took %llu us\n", delta_us);
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
	syslog_libbidib(LOG_DEBUG, "Message bytes: %s", hex_string);
}

static void bidib_buffer_message(uint8_t seqnum, uint8_t type,
                                 const uint8_t *const message, unsigned int action_id) {
	uint8_t addr[4];
	bidib_extract_address(message, addr);
	bidib_log_send_message(type, addr, seqnum, message, action_id);
	if (bidib_node_try_send(addr, type, message, action_id)) {
		// Put in buffer
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
