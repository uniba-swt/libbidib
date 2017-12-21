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

#ifndef BIDIB_LOWLEVEL_BOOSTER_H
#define BIDIB_LOWLEVEL_BOOSTER_H

#include "../definitions/bidib_definitions_custom.h"


/**
 * Switches on the track power.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param unicast the distribution, 0 means broadcast, 1 means unicast.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_boost_on(t_bidib_node_address node_address, unsigned char unicast,
                         unsigned int action_id);

/**
 * Switches off the track power.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param unicast the distribution, 0 means broadcast, 1 means unicast.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_boost_off(t_bidib_node_address node_address, unsigned char unicast,
                          unsigned int action_id);

/**
 * Queries the status of a booster.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_boost_query(t_bidib_node_address node_address, unsigned int action_id);


#endif
