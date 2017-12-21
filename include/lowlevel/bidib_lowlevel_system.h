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

#ifndef BIDIB_LOWLEVEL_SYSTEM_H
#define BIDIB_LOWLEVEL_SYSTEM_H

#include "../definitions/bidib_definitions_custom.h"


/**
 * Requests the system identifier from a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_get_magic(t_bidib_node_address node_address,
                              unsigned int action_id);

/**
 * Requests the supported BiDiB protocol version from a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_get_p_version(t_bidib_node_address node_address,
                                  unsigned int action_id);

/**
 * Enables spontaneous messages.
 * 
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_enable(unsigned int action_id);

/**
 * Disables spontaneous messages.
 * 
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_disable(unsigned int action_id);

/**
 * Requests the unique id from a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_get_unique_id(t_bidib_node_address node_address,
                                  unsigned int action_id);

/**
 * Requests the installed software version from the node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_get_sw_version(t_bidib_node_address node_address,
                                   unsigned int action_id);

/**
 * Sends a ping to a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param ping_byte the byte which the node responds.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_ping(t_bidib_node_address node_address, unsigned char ping_byte,
                         unsigned int action_id);

/**
 * Instructs a node to switch the status of its local identify indicator.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param identify_status the new status of the indicator, 0 for off, 1 for on.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_identify(t_bidib_node_address node_address,
                             unsigned char identify_status, unsigned int action_id);

/**
 * Requests the last occurred error message.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_get_error(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Resets the bidib system, including all buffered messages. NOT threadsafe!
 * Ensure that no other libbidib functions are called during execution.
 * 
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_reset(unsigned int action_id);

/**
 * Tells the interface to transfer current assignment table of unique-id and
 * local address.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_nodetab_getall(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Tells the interface to send next line of node table.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_nodetab_getnext(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Reads maximum message length from the interface.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_get_pkt_capacity(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Sends acknowledgement for new or changed node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param confirmed_number the confirmed sequence number.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_node_changed_ack(t_bidib_node_address node_address,
                                 unsigned char confirmed_number, unsigned int action_id);

/**
 * Transmits the model time.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param tcode0 00mmmmmm, minute indication, value range 0…59.
 * @param tcode1 100HHHHH, hour indication, value range 0…23.
 * @param tcode2 01000www, weekday, 0=Monday, 1=Tuesday, ... 6=Sunday.
 * @param tcode3 110fffff, clock acceleration factor, fffff=0 means clock is stopped.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_sys_clock(t_bidib_node_address node_address, unsigned char tcode0, unsigned char tcode1,
                          unsigned char tcode2, unsigned char tcode3, unsigned int action_id);


#endif
