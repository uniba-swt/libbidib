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

#ifndef BIDIB_LOWLEVEL_PORTCONFIG_H
#define BIDIB_LOWLEVEL_PORTCONFIG_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Issues a direct output operation.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param port0 the first byte of the port address.
 * @param port1 the second byte of the port address.
 * @param portstat the new status for the port.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_output(t_bidib_node_address node_address, uint8_t port0,
                          uint8_t port1, uint8_t portstat,
                          unsigned int action_id);

/**
 * Queries the state of a port.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param port0 the first byte of the port address.
 * @param port1 the second byte of the port address.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_port_query(t_bidib_node_address node_address, uint8_t port0,
                              uint8_t port1, unsigned int action_id);

/**
 * Queries the states of multiple ports.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param query_params the query parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_port_query_all(t_bidib_node_address node_address,
                                  t_bidib_port_query_params query_params,
                                  unsigned int action_id);

/**
 * Configures a port.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param port0 the first byte of the port address.
 * @param port1 the second byte of the port address.
 * @param pairs_num the number of key-value pairs, range 1...8.
 * @param pairs the pairs.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_configx_set(t_bidib_node_address node_address, uint8_t port0,
                               uint8_t port1, uint8_t pairs_num,
                               uint8_t *pairs, unsigned int action_id);

/**
 * Queries the configuration of a port.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param port0 the first byte of the port address.
 * @param port1 the second byte of the port address.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_configx_get(t_bidib_node_address node_address, uint8_t port0,
                               uint8_t port1, unsigned int action_id);

/**
 * Queries the configuration of multiple ports.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param port0 the first byte of the port address.
 * @param port1 the second byte of the port address.
 * @param address_range the port address range to query.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_configx_get_all(t_bidib_node_address node_address, uint8_t port0,
                                   uint8_t port1,
                                   t_bidib_port_query_address_range address_range,
                                   unsigned int action_id);

/**
 * Handles a macro.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param macro_index the index of the macro.
 * @param opcode the operation for the macro.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_macro_handle(t_bidib_node_address node_address, uint8_t macro_index,
                                uint8_t opcode, unsigned int action_id);

/**
 * Sets a macro switch point.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param macro_params the parameters for the switch point.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_macro_set(t_bidib_node_address node_address,
                             t_bidib_macro_params macro_params, unsigned int action_id);

/**
 * Queries a macro point.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param macro_index the index of the macro.
 * @param point_index the index of the switching point within the macro.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_macro_get(t_bidib_node_address node_address, uint8_t macro_index,
                             uint8_t point_index, unsigned int action_id);

/**
 * Sets a general parameter for a macro.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param macro_params the parameter for the macro.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_macro_para_set(t_bidib_node_address node_address,
                                  t_bidib_macro_params macro_params, unsigned int action_id);

/**
 * Queries a macro parameter.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param macro_index the index of the macro.
 * @param param_index the index of the parameter.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_lc_macro_para_get(t_bidib_node_address node_address, uint8_t macro_index,
                                  uint8_t param_index, unsigned int action_id);


#endif
