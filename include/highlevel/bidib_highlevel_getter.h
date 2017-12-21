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

#ifndef BIDIB_HIGHLEVEL_GETTER_H
#define BIDIB_HIGHLEVEL_GETTER_H

#include "../definitions/bidib_definitions_custom.h"


/**
 * Returns the overall state of the track.
 *
 * @return the overall state of the track. Must be freed by the caller.
 */
t_bidib_track_state bidib_get_state(void);

/**
 * Returns the current state of a point.
 *
 * @param point the id of the point.
 * @return the position of the point. Must be freed by the caller.
 */
t_bidib_unified_accessory_state_query bidib_get_point_state(const char *point);

/**
 * Returns the current state of a signal.
 *
 * @param signal the id of the signal.
 * @return the state of the signal. Must be freed by the caller.
 */
t_bidib_unified_accessory_state_query bidib_get_signal_state(const char *signal);

/**
 * Returns the current state of a peripheral (e.g. light).
 *
 * @param peripheral the id of the peripheral.
 * @return the state of the peripheral. Must be freed by the caller.
 */
t_bidib_peripheral_state_query bidib_get_peripheral_state(const char *peripheral);

/**
 * Returns the occupancy state of a section.
 *
 * @param segment the id of the section.
 * @return the occupancy state. Must be freed by the caller.
 */
t_bidib_segment_state_query bidib_get_segment_state(const char *segment);

/**
 * Returns the unique id of a board.
 *
 * @param board the id of the board.
 * @return the unique id of the board.
 */
t_bidib_unique_id_query bidib_get_uniqueid(const char *board);

/**
 * Returns the unique id of a board.
 *
 * @param node_address the node address of the board.
 * @return the unique id of the board.
 */
t_bidib_unique_id_query bidib_get_uniqueid_by_nodeaddr(t_bidib_node_address node_address);

/**
 * Returns the node address of a board.
 *
 * @param board the id of the board.
 * @return the node address of the board.
 */
t_bidib_node_address_query bidib_get_nodeaddr(const char *board);

/**
 * Returns the node address of a board.
 *
 * @param uid the unique id of the board.
 * @return the node address of the board.
 */
t_bidib_node_address_query bidib_get_nodeaddr_by_uniqueid(t_bidib_unique_id_mod uid);

/**
 * Returns the id of an board.
 *
 * @param unique_id the unique id of the board.
 * @return the id of the board. Must be freed by the caller.
 */
t_bidib_id_query bidib_get_board_id(t_bidib_unique_id_mod unique_id);

/**
 * Returns the ids of all boards.
 *
 * @return all board ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_boards(void);

/**
 * Returns the ids of all connected boards.
 *
 * @return all connected board ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_boards_connected(void);

/**
 * Returns whether a board is connected or not.
 *
 * @param board the id of the board.
 * @return true if connected, otherwise false.
 */
bool bidib_get_board_connected(const char *board);

/**
 * Returns the features of the board, which are set via the config files.
 *
 * @param board the id of the board.
 * @return the features. Must be freed by the caller.
 */
t_bidib_board_features_query bidib_get_board_features(const char *board);

/**
 * Returns all points connected to a board.
 *
 * @param board the id of the board.
 * @return all point ids connected to the board. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_board_points(const char *board);

/**
 * Returns all signals connected to a board.
 *
 * @param board the id of the board.
 * @return all signal ids connected to the board. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_board_signals(const char *board);

/**
 * Returns all peripherals connected to a board.
 *
 * @param board the id of the board.
 * @return all peripheral ids connected to the board. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_board_peripherals(const char *board);

/**
 * Returns all segments connected to a board.
 *
 * @param board the id of the board.
 * @return all segment ids connected to the board. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_board_segments(const char *board);

/**
 * Returns all connected point ids.
 *
 * @return all connected point ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_connected_points(void);

/**
 * Returns all connected signal ids.
 *
 * @return all connected signal ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_connected_signals(void);

/**
 * Returns all connected peripheral ids.
 *
 * @return all connected peripheral ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_connected_peripherals(void);

/**
 * Returns all connected segment ids.
 *
 * @return all connected segment ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_connected_segments(void);

/**
 * Returns all connected booster ids.
 *
 * @return all connected booster ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_connected_boosters(void);

/**
 * Returns all booster ids.
 *
 * @return all booster ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_boosters(void);

/**
 * Returns all track output ids.
 *
 * @return all track output ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_track_outputs(void);

/**
 * Returns all connected track output ids.
 *
 * @return all connected track output ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_connected_track_outputs(void);

/**
 * Returns the state of a booster.
 *
 * @param booster the id of the booster.
 * @return the state of the booster.
 */
t_bidib_booster_state_query bidib_get_booster_state(const char *booster);

/**
 * Returns the state of a track output.
 *
 * @param track_output the id of the track output.
 * @return the state of the track output.
 */
t_bidib_track_output_state_query bidib_get_track_output_state(const char *track_output);

/**
 * Returns all train ids regardless if available or not.
 *
 * @return all train ids. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_trains(void);

/**
 * Returns all train ids which are on the track.
 *
 * @return all train ids which are on the track. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_trains_on_track(void);

/**
 * Returns all peripherals of a train.
 *
 * @param train the id of the train.
 * @return all peripheral ids of the train. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_train_peripherals(const char *train);

/**
 * Returns the id of a train.
 *
 * @param dcc_address the dcc address of the train.
 * @return the id of the train.
 */
t_bidib_id_query bidib_get_train_id(t_bidib_dcc_address dcc_address);

/**
 * Returns the DCC address of a train.
 *
 * @param train the id of the train.
 * @return the dcc address of the train.
 */
t_bidib_dcc_address_query bidib_get_train_dcc_addr(const char *train);

/**
 * Returns the state of a train.
 *
 * @param train the id of the train.
 * @return the state of the train. Must be freed by the caller.
 */
t_bidib_train_state_query bidib_get_train_state(const char *train);

/**
 * Returns the current state of a train peripheral.
 *
 * @param train the id of the train.
 * @param peripheral the id of the peripheral.
 * @return the state of the peripheral.
 */
t_bidib_train_peripheral_state_query bidib_get_train_peripheral_state(const char *train,
                                                                      const char *peripheral);

/**
 * Returns the current position of a train.
 *
 * @param train the id of the train.
 * @return the train position. Must be freed by the caller.
 */
t_bidib_train_position_query bidib_get_train_position(const char *train);

/**
 * Returns the current speed step of a train.
 *
 * @param train the id of the train.
 * @return the current speed step.
 */
t_bidib_train_speed_step_query bidib_get_train_speed_step(const char *train);

/**
 * Returns the current speed in kmh of a train.
 *
 * @param train the id of the train.
 * @return the current speed in kmh.
 */
t_bidib_train_speed_kmh_query bidib_get_train_speed_kmh(const char *train);

/**
 * Returns whether a train is on the track or not.
 *
 * @param train the id of the train.
 * @return whether the train is on the track or not.
 */
bool bidib_get_train_on_track(const char *train);

/**
 * Returns all aspects of a point.
 *
 * @param point the id of the point.
 * @return all aspect ids of the point. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_point_aspects(const char *point);

/**
 * Returns all aspects of a signal.
 *
 * @param signal the id of the signal.
 * @return all aspect ids of the signal. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_signal_aspects(const char *signal);

/**
 * Returns all aspects of a peripheral.
 *
 * @param peripheral the id of the peripheral.
 * @return all aspect ids of the peripheral. Must be freed by the caller.
 */
t_bidib_id_list_query bidib_get_peripheral_aspects(const char *peripheral);

/**
 * Frees the memory allocated by a track state.
 *
 * @param track_state the track state which values should be freed.
 */
void bidib_free_track_state(t_bidib_track_state track_state);

/**
 * Frees the memory allocated by an unified accessory state query.
 *
 * @param query the unified accessory state query which values should be freed.
 */
void bidib_free_unified_accessory_state_query(t_bidib_unified_accessory_state_query query);

/**
 * Frees the memory allocated by a peripheral state query.
 *
 * @param query the peripheral state query which values should be freed.
 */
void bidib_free_peripheral_state_query(t_bidib_peripheral_state_query query);

/**
 * Frees the memory allocated by a segment state query.
 *
 * @param query the segment state query which values should be freed.
 */
void bidib_free_segment_state_query(t_bidib_segment_state_query query);

/**
 * Frees the memory allocated by an id query.
 *
 * @param query the id query which values should be freed.
 */
void bidib_free_id_query(t_bidib_id_query query);

/**
 * Frees the memory allocated by an id list query.
 *
 * @param query the id list query which values should be freed.
 */
void bidib_free_id_list_query(t_bidib_id_list_query query);

/**
 * Frees the memory allocated by an unique id list query.
 *
 * @param query the unique id list query which values should be freed.
 */
void bidib_free_unique_id_list_query(t_bidib_unique_id_list_query query);

/**
 * Frees the memory allocated by a features query.
 *
 * @param query the features query which values should be freed.
 */
void bidib_free_board_features_query(t_bidib_board_features_query query);

/**
 * Frees the memory allocated by a train position query.
 *
 * @param query the train position query which values should be freed.
 */
void bidib_free_train_position_query(t_bidib_train_position_query query);

/**
 * Frees the memory allocated by a train state query.
 *
 * @param query the train state query which values should be freed.
 */
void bidib_free_train_state_query(t_bidib_train_state_query query);


#endif
