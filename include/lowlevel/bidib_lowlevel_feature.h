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

#ifndef BIDIB_LOWLEVEL_FEATURE_H
#define BIDIB_LOWLEVEL_FEATURE_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Queries the number of feature settings of a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_feature_getall(t_bidib_node_address node_address,
                               unsigned int action_id);

/**
 * Queries the next feature value of a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_feature_getnext(t_bidib_node_address node_address, unsigned int action_id);

/**
 * Queries a single feature value of a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param feature_number the number of the feature.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_feature_get(t_bidib_node_address node_address, uint8_t feature_number,
                            unsigned int action_id);

/**
 * Queries a single feature value of a node.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param feature_number the number of the feature.
 * @param feature_value the value for the future.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_feature_set(t_bidib_node_address node_address,
                            uint8_t feature_number, uint8_t feature_value,
                            unsigned int action_id);


#endif
