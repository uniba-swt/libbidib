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

#ifndef BIDIB_HIGHLEVEL_SETTER_H
#define BIDIB_HIGHLEVEL_SETTER_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Switches a point.
 *
 * @param point the id of the point.
 * @param aspect the new position of the point.
 * @return 0 for valid params, otherwise 1.
 */
int bidib_switch_point(const char *point, const char *aspect);

/**
 * Sets a signal.
 *
 * @param signal the id of the signal.
 * @param aspect the new state.
 * @return 0 for valid params, otherwise 1.
 */
int bidib_set_signal(const char *signal, const char *aspect);

/**
 * Sets a peripheral on the track.
 *
 * @param peripheral the id of the peripheral.
 * @param aspect the new state.
 * @return 0 for valid params, otherwise 1.
 */
int bidib_set_peripheral(const char *peripheral, const char *aspect);

/**
 * Sets the speed of a train.
 *
 * @param train the id of the train.
 * @param speed the new speed, -126..0..126.
 * @param track_output the track output node.
 * @return 0 for valid params, otherwise 1.
 */
int bidib_set_train_speed(const char *train, int speed, const char *track_output);

/**
 * Sets the calibrated speed of a train.
 *
 * @param train the id of the calibrated train.
 * @param speed the new speed, -9..0..9.
 * @param track_output the track output node.
 * @return 0 for valid params, otherwise 1.
 */
int bidib_set_calibrated_train_speed(const char *train, int speed, const char *track_output);

/**
 * Activates the emergency stop for a train.
 *
 * @param train the id of the train.
 * @param track_output the id of the track output.
 * @return 0 if train and track output is known, otherwise 1.
 */
int bidib_emergency_stop_train(const char *train, const char *track_output);

/**
 * Sets a peripheral of a train.
 *
 * @param train the id of the train.
 * @param peripheral the id of the peripheral.
 * @param state the new state, 0/1.
 * @param track_output the track output node.
 * @return 0 for valid params, otherwise 1.
 */
int bidib_set_train_peripheral(const char *train, const char *peripheral,
                               uint8_t state, const char *track_output);

/**
 * Sets the power state of a booster.
 *
 * @param booster the id of the booster.
 * @param on true for on, false for off.
 * @return 0 if booster is known, otherwise 1.
 */
int bidib_set_booster_power_state(const char *booster, bool on);

/**
 * Sets the state of a track output.
 *
 * @param track_output the id of the track output.
 * @param state the new state for the track output.
 * @return 0 if track output is known, otherwise 1.
 */
int bidib_set_track_output_state(const char *track_output, t_bidib_cs_state state);

/**
 * Sets the state of all track outputs.
 *
 * @param state the new state for all track outputs.
 */
void bidib_set_track_output_state_all(t_bidib_cs_state state);


#endif
