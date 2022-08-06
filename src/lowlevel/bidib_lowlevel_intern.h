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

#ifndef BIDIB_LOWLEVEL_INTERN_H
#define BIDIB_LOWLEVEL_INTERN_H


#include "../../include/bidib.h"
#include <pthread.h>

extern pthread_rwlock_t bidib_state_trains_rwlock;

/**
 * Used to avoid the usage of a recursive rwlock.
 * Must call with with param `lock` false if bidib_state_trains_rwlock
 * is already >= read acquired (dev todo: write acquired?).
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


#endif
