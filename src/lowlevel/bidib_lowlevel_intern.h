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

#ifndef BIDIB_LOWLEVEL_INTERN_H
#define BIDIB_LOWLEVEL_INTERN_H


#include "../../include/definitions/bidib_definitions_custom.h"
#include <pthread.h>

/**
 * Used to avoid the usage of a recursive rwlock.
 * Must call with param `lock` false if bidib_state_trains_rwlock
 * has already been acquired with >= read.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param cs_drive_params the parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 * @param lock indicates whether the train rwlock should be locked.
 */
void bidib_send_cs_drive_intern(t_bidib_node_address node_address,
                                t_bidib_cs_drive_mod cs_drive_params,
                                unsigned int action_id, bool lock);

/**
 * Issues an accessory command.
 * Shall only be called with trackstate_accessories_mutex acquired,
 * and with bidib_state_boards_rwlock >=read acquired.
 * 
 * Old: Must only be called with bidib_state_track_rwlock write acquired,
 * and with bidib_state_boards_rwlock >=read acquired.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param cs_accessory_params the parameters.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_cs_accessory_intern(t_bidib_node_address node_address,
                                    t_bidib_cs_accessory_mod cs_accessory_params,
                                    unsigned int action_id);

#endif
