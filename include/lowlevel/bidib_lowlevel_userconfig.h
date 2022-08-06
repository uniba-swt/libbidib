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

#ifndef BIDIB_LOWLEVEL_USERCONFIG_H
#define BIDIB_LOWLEVEL_USERCONFIG_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Enables vendor configuration mode for node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param unique_id the unique id of the node.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_vendor_enable(t_bidib_node_address node_address,
                              t_bidib_unique_id_mod unique_id, unsigned int action_id);

/**
 * Disables vendor configuration mode for node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_vendor_disable(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Sets a vendor configuration option.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param vendor_data the configuration option to set.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_vendor_set(t_bidib_node_address node_address,
                           t_bidib_vendor_data vendor_data, unsigned int action_id);

/**
 * Gets a vendor configuration option.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param name_length the length of the configuration option name.
 * @param name the configuration option name.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_vendor_get(t_bidib_node_address node_address, uint8_t name_length,
                           const uint8_t *const name, unsigned int action_id);

/**
 * Sets a string variable inside a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param namespace the addressed namespace inside the node.
 * @param string_id the addressed string inside the namespace.
 * @param string_size the length of the string.
 * @param string the string which should be set.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_string_set(t_bidib_node_address node_address, uint8_t namespace,
                           uint8_t string_id, uint8_t string_size,
                           const uint8_t *const string, unsigned int action_id);

/**
 * Gets a string variable inside a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param namespace the addressed namespace inside the node.
 * @param string_id the addressed string inside the namespace.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_string_get(t_bidib_node_address node_address, uint8_t namespace,
                           uint8_t string_id, unsigned int action_id);


#endif
