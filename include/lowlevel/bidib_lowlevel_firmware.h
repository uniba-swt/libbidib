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

#ifndef BIDIB_LOWLEVEL_FIRMWARE_H
#define BIDIB_LOWLEVEL_FIRMWARE_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Switches node to update mode.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param unique_id the unique id of the node.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_fw_update_op_enter(t_bidib_node_address node_address,
                                   t_bidib_unique_id_mod unique_id, unsigned int action_id);

/**
 * Switches node to normal mode.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_fw_update_op_exit(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Selects the target memory in which the following data should be stored.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param target_range the target, 0 for flash, 1 for EEPROM.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_fw_update_op_setdest(t_bidib_node_address node_address,
                                     uint8_t target_range, unsigned int action_id);

/**
 * Sends data set for currently selected target memory. 'White'-Characters
 * (0x20, 0x09, 0x0D and 0x0a) will not be transmitted.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param data_size the number of data bytes.
 * @param data a line of Intel Hex file.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_fw_update_op_data(t_bidib_node_address node_address, uint8_t data_size,
                                  const uint8_t *const data, unsigned int action_id);

/**
 * No more data available for the currently selected target memory. Tells node
 * to perform the update of the target memory.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_fw_update_op_done(t_bidib_node_address node_address, unsigned int action_id);


#endif
