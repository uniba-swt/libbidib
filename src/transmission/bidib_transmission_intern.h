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

#ifndef BIDIB_TRANSMISSION_INTERN_H
#define BIDIB_TRANSMISSION_INTERN_H

#include <glib.h>
#include <stdbool.h>
#include <stdint.h>

#include "../../include/definitions/bidib_definitions_custom.h"


typedef struct {
	uint8_t type;
	uint8_t addr[4];
	uint8_t *message;
	unsigned int action_id;
} t_bidib_message_queue_entry;

typedef struct {
	uint8_t type;
	time_t creation_time;
	unsigned int action_id;
} t_bidib_response_queue_entry;

typedef struct {
	uint8_t addr[4];
} t_bidib_stall_queue_entry;

typedef struct {
	char addr[4];
	uint8_t receive_seqnum;
	uint8_t send_seqnum;
	bool stall;
	int current_max_respond;
	GQueue *stall_affected_nodes_queue;
	GQueue *response_queue;
	GQueue *message_queue;
} t_bidib_node_state;


extern pthread_mutex_t bidib_node_state_table_mutex;
extern pthread_mutex_t bidib_uplink_queue_mutex;
extern pthread_mutex_t bidib_uplink_error_queue_mutex;
extern pthread_mutex_t bidib_uplink_intern_queue_mutex;
extern pthread_mutex_t bidib_send_buffer_mutex;

extern const uint8_t bidib_crc_array[256];
extern const char *const bidib_message_string_mapping[0x100];
extern const char *const bidib_cs_state_string_mapping[9];
extern const char *const bidib_boost_state_string_mapping[0x85];
extern const char *const bidib_error_string_mapping[0x31];
extern const char *const bidib_bus_error_string_mapping[7];
extern const int bidib_response_info[0x80][5];
extern volatile bool bidib_running;
extern volatile bool bidib_discard_rx;
extern volatile bool bidib_seq_num_enabled;
extern volatile bool bidib_lowlevel_debug_mode;

/**
 * Puts a message into the send buffer.
 *
 * @param message the message which should be added.
 */
void bidib_add_to_buffer(const uint8_t *const message);

/**
 * Puts a message without any data bytes in the buffer for the receiver node.
 *
 * @param addr_stack the address stack. Index 0 represents the top of the stack,
 * at the latest index 3 must be 0x00.
 * @param msg_type the message type.
 * @param action_id reference number to a high level function call.
 */
void bidib_buffer_message_without_data(const uint8_t *const addr_stack, uint8_t msg_type,
                                       unsigned int action_id);

/**
 * Puts a message with data bytes in the buffer for the receiver node.
 *
 * @param addr_stack the address stack. Index 0 represents the top of the stack,
 * at the latest index 3 must be 0x00.
 * @param msg_type the message type.
 * @param data the data bytes.
 * @param data_length the number of data bytes.
 * @param action_id reference number to a high level function call.
 */
void bidib_buffer_message_with_data(const uint8_t *const addr_stack, uint8_t msg_type,
                                    uint8_t data_length, const uint8_t *const data,
                                    unsigned int action_id);

/**
 * Checks whether a node is ready to receive a message. If not the message will be enqueued.
 *
 * @param addr_stack the address stack. Index 0 represents the top of the stack, at the latest index 3 must be 0x00.
 * @param type the message type.
 * @param message the complete message.
 * @param action_id reference number to a high level function call.
 * @return true if the node is ready, false if not.
 */
bool bidib_node_try_send(const uint8_t *const addr_stack, uint8_t type,
                         const uint8_t *const message, unsigned int action_id);

/**
 * Signals that a message was received from a node to update the node state table.
 *
 * @param addr_stack the address of the sender.
 * @param response_type the received message type.
 * @return the reference number (action id) to a high level function call.
 */
unsigned int bidib_node_state_update(const uint8_t *const addr_stack, uint8_t response_type);

/**
 * Updates the stall status of a node.
 *
 * @param addr_stack the address of the sender.
 * @param stall_status the new stall status.
 */
void bidib_node_update_stall(const uint8_t *const addr_stack, uint8_t stall_status);

/**
 * Gets and increments the sequence number of a node for receiving messages.
 *
 * @param addr_stack the address of the sender.
 * @return the sequence number.
 */
uint8_t bidib_node_state_get_and_incr_receive_seqnum(const uint8_t *const addr_stack);

/**
 * Gets and increments the sequence number of a node for receiving messages.
 *
 * @param addr_stack the address of the receiver.
 * @return the sequence number.
 */
uint8_t bidib_node_state_get_and_incr_send_seqnum(const uint8_t *const addr_stack);

/**
 * Sets the sequence number of a node for receiving messages.
 *
 * @param addr_stack the address of the sender.
 * @param seqnum the new sequence number.
 */
void bidib_node_state_set_receive_seqnum(const uint8_t *const addr_stack, uint8_t seqnum);

/**
 * Resets the node state table.
 * 
 * @param lock_node_state_table_access whether the mutex for accessing the node state table
 * shall be locked and unlocked in this method. Use true if your are not already locking the mutex.
 */
void bidib_node_state_table_reset(bool lock_node_state_table_access);

/**
 * Clears the node state table.
 */
void bidib_node_state_table_free(void);

/**
 * Resets the response message queue.
 */
void bidib_uplink_queue_reset(bool lock_mutex);

/**
 * Clears the response message queue.
 */
void bidib_uplink_queue_free(void);

/**
 * Resets the error message queue.
 */
void bidib_uplink_error_queue_reset(bool lock_mutex);

/**
 * Clears the error message queue.
 */
void bidib_uplink_error_queue_free(void);

/**
 * Resets the intern message queue.
 */
void bidib_uplink_intern_queue_reset(bool lock_mutex);

/**
 * Clears the intern message queue.
 */
void bidib_uplink_intern_queue_free(void);

/**
* Returns and removes the oldest received message from the intern queue. It's
* the calling function's responsibility to free the memory of the message.
*
* @return NULL if there is no message, otherwise the oldest message.
*/
uint8_t *bidib_read_intern_message(void);

/**
 * Sets the input of libbidib.
 *
 * @param read a pointer to a function, which reads a byte from the
 * connected BiDiB interface.
 */
void bidib_set_read_src(uint8_t (*read)(int *));

/**
 * Sets the output of libbidib.
 *
 * @param write a pointer to a function, which sends a byte to the connected
 * BiDiB interface.
 */
void bidib_set_write_dest(void (*write)(uint8_t));

/**
 * Sets the maximum capacity for a packet. Default is 64. Max is 256.
 *
 * @param max_capacity the new maximum capacity.
 */
void bidib_state_packet_capacity(uint8_t max_capacity);

/**
 * Extracts the type from a message.
 *
 * @param message the message.
 * @return the message type.
 */
uint8_t bidib_extract_msg_type(const uint8_t *const message);

/**
 * Extracts the address from a message and fills up with 0's (to reach length 4).
 *
 * @param message the message.
 * @param dest where the address should be stored. Must hold 4*(sizeof(char)).
 */
void bidib_extract_address(const uint8_t *const message, uint8_t *dest);

/**
 * Extracts the sequence number from a message.
 *
 * @param message the message.
 * @return the sequence number.
 */
uint8_t bidib_extract_seq_num(const uint8_t *const message);

/**
 * Returns the index of the first data byte.
 *
 * @param message the message.
 * @return the index of the first data byte, -1 if no data bytes were found.
 */
int bidib_first_data_byte_index(const uint8_t *const message);

/**
 * Builds a string of the hex values of a BiDiB message.
 *
 * @param message the message.
 * @param dest the destination for the string.
 */
void bidib_build_message_hex_string(const uint8_t *const message, char *dest);

/**
 * Checks whether an interface is connected.
 *
 * @return true if an interface is connected, otherwise false.
 */
bool bidib_communication_works(void);

/**
 * Receives messages in an infinite loop.
 *
 * @return NULL.
 */
void *bidib_auto_receive(void *);

/**
 * Automatically flushes every x ms.
 *
 * @param interval the flush interval in ms.
 * @return NULL.
 */
void *bidib_auto_flush(void *interval);

/**
 * Initializes the node state table.
 */
void bidib_node_state_table_init(void);

/**
 * Sets the debug mode. If debug mode is on, the library will put all uplink
 * messages (except of MSG_STALL) in the message queue and will neither create
 * the allocation table nor set features and initial values.
 *
 * @param uplink_debug_mode_on the state for the debug mode.
 */
void bidib_set_lowlevel_debug_mode(bool uplink_debug_mode_on);

/**
 * Decides how each message should be processed.
 *
 * @param message the message received from a node.
 * @param type the message type.
 * @param addr_stack the address of the sender.
 * @param seqnum the new sequence number.
 * @param action_id reference number to a high level function call.
 */
void bidib_handle_received_message(uint8_t *message, uint8_t type,
                                   const uint8_t *const addr_stack, uint8_t seqnum,
                                   unsigned int action_id);


#endif
