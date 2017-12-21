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

#include <syslog.h>

#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"


void bidib_send_boost_on(t_bidib_node_address node_address,
                         unsigned char unicast, unsigned int action_id) {
	if (unicast > 1) {
		syslog(LOG_ERR, "%s%02x", "MSG_BOOST_ON called with invalid parameter unicast = ", unicast);
		return;
	}
	unsigned char addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	unsigned char data[] = {unicast};
	bidib_buffer_message_with_data(addr_stack, MSG_BOOST_ON, 1, data, action_id);
}

void bidib_send_boost_off(t_bidib_node_address node_address,
                          unsigned char unicast, unsigned int action_id) {
	if (unicast > 1) {
		syslog(LOG_ERR, "%s%02x", "MSG_BOOST_OFF called with invalid parameter unicast = ", unicast);
		return;
	}
	unsigned char addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	unsigned char data[] = {unicast};
	bidib_buffer_message_with_data(addr_stack, MSG_BOOST_OFF, 1, data, action_id);
}

void bidib_send_boost_query(t_bidib_node_address node_address, unsigned int action_id) {
	unsigned char addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_BOOST_QUERY, action_id);
}
