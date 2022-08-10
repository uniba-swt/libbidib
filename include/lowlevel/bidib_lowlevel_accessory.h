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

#ifndef BIDIB_LOWLEVEL_ACCESSORY_H
#define BIDIB_LOWLEVEL_ACCESSORY_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Applies a setting to an accessory.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param anum the object identifier at the node, range 0...127.
 * @param aspect the state of the object, range 0...127.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_accessory_set(t_bidib_node_address node_address, uint8_t anum,
                              uint8_t aspect, unsigned int action_id);

/**
 * Queries the state of an object at an accessory.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param anum the object identifier at the node, range 0...127.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_accessory_get(t_bidib_node_address node_address, uint8_t anum,
                              unsigned int action_id);

/**
 * Sets the operating mode parameter of an accessory object.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param anum the object identifier at the node, range 0...127.
 * @param anum_op the identifier of the object, that represents the operating mode.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_accessory_para_set_opmode(t_bidib_node_address node_address, uint8_t anum,
                                          uint8_t anum_op, unsigned int action_id);

/**
 * Defines the the behaviour of the accessory at startup and reset.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param anum the object identifier at the node, range 0...127.
 * @param startup_behaviour the startup behaviour.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_accessory_para_set_startup(t_bidib_node_address node_address, uint8_t anum,
                                           uint8_t startup_behaviour, unsigned int action_id);

/**
 * Assigns macros to aspects.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param anum the object identifier at the node, range 0...127.
 * @param data_size the number of data bytes, max 16.
 * @param data the data bytes, last byte must be 0xFF.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_accessory_para_set_macromap(t_bidib_node_address node_address, uint8_t anum,
                                            uint8_t data_size, const uint8_t *const data,
                                            unsigned int action_id);

/**
 * Sets the switch time of an accessory object.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param anum the object identifier at the node, range 0...127.
 * @param time the switch time.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_accessory_para_set_switch_time(t_bidib_node_address node_address, uint8_t anum,
                                               uint8_t time, unsigned int action_id);

/**
 * Queries a configuration parameter of an accessory object.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param anum the object identifier at the node, range 0...127.
 * @param para_num the configuration parameter which is queried.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_accessory_para_get(t_bidib_node_address node_address, uint8_t anum,
                                   uint8_t para_num, unsigned int action_id);


#endif
