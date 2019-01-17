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

#ifndef BIDIB_LOWLEVEL_TRACK_H
#define BIDIB_LOWLEVEL_TRACK_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"
#include "../definitions/bidib_messages.h"


/**
 * Locks track-output node for 2 seconds to only receive commands from the
 * sender of this message.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_allocate(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Sets the state of the track output.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param state the new state.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_set_state(t_bidib_node_address node_address,
                             uint8_t state, unsigned int action_id);

/**
 * Issues a motion command.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param cs_drive_params the parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_drive(t_bidib_node_address node_address,
                         t_bidib_cs_drive_mod cs_drive_params, unsigned int action_id);

/**
 * Issues an accessory command.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param cs_accessory_params the parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_accessory(t_bidib_node_address node_address,
                             t_bidib_cs_accessory_mod cs_accessory_params,
                             unsigned int action_id);

/**
 * Issues a programming command for the main track.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param cs_pom_params the parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_pom(t_bidib_node_address node_address,
                       t_bidib_cs_pom_mod cs_pom_params, unsigned int action_id);

/**
 * Activates an individual action at the decoder.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param bin_state_params the parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_bin_state(t_bidib_node_address node_address,
                             t_bidib_bin_state_mod bin_state_params, unsigned int action_id);

/**
 * Issues service mode commands.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param cs_prog_params the parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_prog(t_bidib_node_address node_address,
                        t_bidib_cs_prog_mod cs_prog_params, unsigned int action_id);

/**
 * Queries the current TID of a track output device.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_get_id(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Sets the TID of a track output device.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param rcplus_tid the TID which should be set.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_set_id(t_bidib_node_address node_address,
                                 t_rcplus_tid rcplus_tid, unsigned int action_id);

/**
 * Enables permanent transmission of the TID.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param interval the interval in 100ms. 0 disables the transmission.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_ping(t_bidib_node_address node_address, uint8_t interval,
                               unsigned int action_id);

/**
 * Queries singular transmission of the TID.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_ping_once_p0(t_bidib_node_address node_address,
                                       unsigned int action_id);

/**
 * Queries singular transmission of the TID.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_ping_once_p1(t_bidib_node_address node_address,
                                       unsigned int action_id);

/**
 * Assigns a decoder address.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param rcplus_unique_id the decoder ID.
 * @param new_addrl the lower byte of the new address.
 * @param new_addrh the upper byte of the new address.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_bind(t_bidib_node_address node_address,
                               t_rcplus_unique_id rcplus_unique_id, uint8_t new_addrl,
                               uint8_t new_addrh, unsigned int action_id);

/**
 * Issues track output command FIND.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param rcplus_unique_id the decoder ID.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_find_p0(t_bidib_node_address node_address,
                                  t_rcplus_unique_id rcplus_unique_id, unsigned int action_id);

/**
 * Issues track output command FIND.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param rcplus_unique_id the decoder ID.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_rcplus_find_p1(t_bidib_node_address node_address,
                                  t_rcplus_unique_id rcplus_unique_id, unsigned int action_id);


#endif
