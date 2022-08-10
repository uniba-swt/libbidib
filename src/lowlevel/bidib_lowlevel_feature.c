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

#include <stdint.h>

#include "../../include/lowlevel/bidib_lowlevel_feature.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"
#include "../../include/definitions/bidib_definitions_custom.h"


void bidib_send_feature_getall(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_FEATURE_GETALL, action_id);
}

void bidib_send_feature_getnext(t_bidib_node_address node_address, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	bidib_buffer_message_without_data(addr_stack, MSG_FEATURE_GETNEXT, action_id);
}

void bidib_send_feature_get(t_bidib_node_address node_address, uint8_t feature_number,
                            unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {feature_number};
	bidib_buffer_message_with_data(addr_stack, MSG_FEATURE_GET, 1, data, action_id);
}

void bidib_send_feature_set(t_bidib_node_address node_address, uint8_t feature_number,
                            uint8_t feature_value, unsigned int action_id) {
	uint8_t addr_stack[] = {node_address.top, node_address.sub, node_address.subsub, 0x00};
	uint8_t data[] = {feature_number, feature_value};
	bidib_buffer_message_with_data(addr_stack, MSG_FEATURE_SET, 2, data, action_id);
}
