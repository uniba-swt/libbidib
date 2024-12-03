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

#ifndef BIDIB_HIGHLEVEL_INTERN_H
#define BIDIB_HIGHLEVEL_INTERN_H

#include <pthread.h>

#include "../../include/definitions/bidib_definitions_custom.h"


extern pthread_rwlock_t bidib_state_trains_rwlock;
extern pthread_rwlock_t bidib_state_boards_rwlock;
extern pthread_mutex_t bidib_action_id_mutex;

extern pthread_mutex_t trackstate_accessories_mutex;
extern pthread_mutex_t trackstate_peripherals_mutex;
extern pthread_mutex_t trackstate_segments_mutex;
extern pthread_mutex_t trackstate_reversers_mutex;
extern pthread_mutex_t trackstate_trains_mutex;
extern pthread_mutex_t trackstate_boosters_mutex;
extern pthread_mutex_t trackstate_track_outputs_mutex;

/**
 * Returns the next action id and increments it afterwards.
 *
 * @return the action id.
 */
unsigned int bidib_get_and_incr_action_id(void);

/**
 * Used only internally in bidib_state_update_train_available and
 * bidib_get_train_position to avoid the usage of a recursive mutex.
 * 
 * Shall only be called with with bidib_state_trains_rwlock >= read acquired,
 * and with trackstate_segments_mutex and trackstate_trains_mutex acquired.
 *
 * @param train the id of the train.
 * @return the train position. Must be freed by the caller.
 */
t_bidib_train_position_query bidib_get_train_position_intern(const char *train);

/**
 * To be run in separate thread; when bidib is running, 
 * logs a heartbeat message with a timestep in
 * a 2-second interval to syslog.
 * 
 * @param __attribute__ unused
 * @return void* 
 */
void *bidib_heartbeat_log(void *par __attribute__((unused)));

#endif
