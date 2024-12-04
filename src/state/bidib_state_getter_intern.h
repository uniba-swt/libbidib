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

#ifndef BIDIB_STATE_GETTER_INTERN_H
#define BIDIB_STATE_GETTER_INTERN_H

#include <stdint.h>

#include "bidib_state_intern.h"


/**
 * Returns the reference to the board with the given id.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param board the id of the board.
 * @return NULL if not found, otherwise the reference to the board.
 */
t_bidib_board *bidib_state_get_board_ref(const char *board);

/**
 * Returns the reference to the board with the given unique id.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param unique_id the unique id of the board.
 * @return NULL if not found, otherwise the reference to the board.
 */
t_bidib_board *bidib_state_get_board_ref_by_uniqueid(t_bidib_unique_id_mod unique_id);

/**
 * Returns the reference to the booster state with the given id.
 * Shall only be called with trackstate_boosters_mutex acquired.
 *
 * @param booster the id of the booster.
 * @return NULL if not found, otherwise the reference to the booster state.
 */
t_bidib_booster_state *bidib_state_get_booster_state_ref(const char *booster);

/**
 * Returns the reference to the track output state with the given id.
 * Shall only be called with trackstate_track_outputs_mutex acquired.
 *
 * @param track_output the id of the track output state.
 * @return NULL if not found, otherwise the reference to the track output state.
 */
t_bidib_track_output_state *bidib_state_get_track_output_state_ref(const char *track_output);

/**
 * Returns the reference to the board accessory mapping with the given id.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param accessory the id of the board accessory.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the board accessory mapping.
 */
t_bidib_board_accessory_mapping *bidib_state_get_board_accessory_mapping_ref(
		const char *accessory, bool point);

/**
 * Returns the reference to the board accessory mapping with the given number.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param node_address the node address of the board.
 * @param number the number of the accessory.
 * @param point used as return value to indicate whether the accessory is a point
 * or a signal.
 * @return NULL if not found, otherwise the reference to the board accessory mapping.
 */
t_bidib_board_accessory_mapping *bidib_state_get_board_accessory_mapping_ref_by_number(
		t_bidib_node_address node_address, uint8_t number, bool *point);

/**
 * Returns the reference to the board accessory state with the given id.
 * Shall only be called with trackstate_accessories_mutex acquired.
 *
 * @param accessory the id of the board accessory state.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the board accessory state.
 */
t_bidib_board_accessory_state *bidib_state_get_board_accessory_state_ref(const char *accessory,
                                                                         bool point);

/**
 * Returns the reference to the dcc accessory mapping with the given id.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param accessory the id of the dcc accessory.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the dcc accessory mapping.
 */
t_bidib_dcc_accessory_mapping *bidib_state_get_dcc_accessory_mapping_ref(
		const char *accessory, bool point);

/**
 * Returns the reference to the dcc accessory mapping with the given dcc address.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param node_address the node address of the board.
 * @param dcc_address the dcc address of the accessory.
 * @param point used as return value to indicate whether the accessory is a point
 * or a signal.
 * @return NULL if not found, otherwise the reference to the dcc accessory mapping.
 */
t_bidib_dcc_accessory_mapping *bidib_state_get_dcc_accessory_mapping_ref_by_dccaddr(
		t_bidib_node_address node_address, t_bidib_dcc_address dcc_address, bool *point);

/**
 * Returns the reference to the dcc accessory state with the given id.
 * Shall only be called with trackstate_accessories_mutex acquired.
 *
 * @param accessory the id of the dcc accessory state.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the dcc accessory state.
 */
t_bidib_dcc_accessory_state *bidib_state_get_dcc_accessory_state_ref(const char *accessory,
                                                                     bool point);

/**
 * Returns the reference to the peripheral mapping with the given id.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param peripheral the id of the peripheral.
 * @return NULL if not found, otherwise the reference to the peripheral mapping.
 */
t_bidib_peripheral_mapping *bidib_state_get_peripheral_mapping_ref(const char *peripheral);

/**
 * Returns the reference to the peripheral mapping with the given port.
 * Shall only be called with the bidib_boards_rwlock >= read acquired.
 *
 * @param node_address the node address of the board.
 * @param port the port of the peripheral.
 * @return NULL if not found, otherwise the reference to the peripheral mapping.
 */
t_bidib_peripheral_mapping *bidib_state_get_peripheral_mapping_ref_by_port(
		t_bidib_node_address node_address, t_bidib_peripheral_port port);

/**
 * Returns the reference to the peripheral state with the given id.
 * Shall only be called with trackstate_peripherals_mutex acquired.
 *
 * @param peripheral the id of the peripheral.
 * @return NULL if not found, otherwise the reference to the peripheral state.
 */
t_bidib_peripheral_state *bidib_state_get_peripheral_state_ref(const char *peripheral);

/**
 * Returns the reference to the segment state with the given id.
 * Shall only be called with trackstate_segments_mutex acquired.
 * 
 * @param segment the id of the segment.
 * @return NULL if not found, otherwise the reference to the segment state.
 */
t_bidib_segment_state_intern *bidib_state_get_segment_state_ref(const char *segment);

/**
 * Returns a deep copy of the segment state.
 *
 * @param segment the segment state
 * @return the segment state. Pointer contents must be freed by the caller.
 */
t_bidib_segment_state_intern bidib_state_get_segment_state(
		const t_bidib_segment_state_intern *const segment);

/**
 * Returns the reference to the segment state with the given id.
 * Shall only be called with trackstate_segments_mutex acquired.
 * Note: uses bidib_boards_rwlock internally, so shall not be called
 * with bidib_boards_rwlock already acquired.
 *
 * @param node_address the node address of the board.
 * @param number the number on the board.
 * @return NULL if not found, otherwise the reference to the segment state.
 */
t_bidib_segment_state_intern *bidib_state_get_segment_state_ref_by_nodeaddr(
		t_bidib_node_address node_address, uint8_t number);

/**
 * Returns the reference to the reverser state with the given id.
 * Shall only be called with trackstate_reversers_mutex acquired.
 *
 * @param reverser the id of the reverser.
 * @return NULL if not found, otherwise the reference to the reverser state.
 */
t_bidib_reverser_state *bidib_state_get_reverser_state_ref(const char *reverser);

/**
 * Returns the reference to the reverser mapping with the given CV.
 * Shall only be called with the bidib_boards_rwlock >= read acquired.
 *
 * @param node_address the node address of the board.
 * @param cv the CV of the reverser.
 * @return NULL if not found, otherwise the reference to the reverser mapping.
 */
t_bidib_reverser_mapping *bidib_state_get_reverser_mapping_ref_by_cv(
		t_bidib_node_address node_address, const char *cv);

/**
 * Returns the reference to the reverser mapping with the given id.
 * Shall only be called with the bidib_boards_rwlock >= read acquired.
 *
 * @param id the name of the reverser.
 * @return NULL if not found, otherwise the reference to the reverser mapping.
 */
t_bidib_reverser_mapping *bidib_state_get_reverser_mapping_ref(const char *reverser);

/**
 * Returns the reference to the board with the given node address.
 * Shall only be called with bidib_boards_rwlock >= read acquired.
 *
 * @param node_address the node address of the board.
 * @return NULL if not found, otherwise the reference to the board.
 */
t_bidib_board *bidib_state_get_board_ref_by_nodeaddr(t_bidib_node_address node_address);

/**
 * Returns the reference to the train with the given id.
 * Shall only be called with bidib_trains_rwlock >= read acquired.
 *
 * @param train the id of the train.
 * @return NULL if not found, otherwise the reference to the train.
 */
t_bidib_train *bidib_state_get_train_ref(const char *train);

/**
 * Returns the reference to the train state with the given dcc address.
 * Shall only be called with bidib_trains_rwlock >= read acquired,
 * and with trackstate_trains_mutex acquired.
 *
 * @param dcc_address the dcc address of the train.
 * @return NULL if not found, otherwise the reference to the train state.
 */
t_bidib_train_state_intern *bidib_state_get_train_state_ref_by_dccaddr(
		t_bidib_dcc_address dcc_address);

/**
 * Returns the reference to the train peripheral state with the given bit.
 * Shall only be called with bidib_trains_rwlock >= read acquired.
 *
 * @param train_state the train state.
 * @param bit the bit of the peripheral.
 * @return NULL if not found, otherwise the reference to the train peripheral state.
 */
t_bidib_train_peripheral_state *bidib_state_get_train_peripheral_state_by_bit(
		const t_bidib_train_state_intern *train_state, uint8_t bit);

/**
 * Returns the reference to the train state with the given id.
 * Shall only be called with trackstate_trains_mutex acquired.
 *
 * @param train the id of the train state.
 * @return NULL if not found, otherwise the reference to the train state.
 */
t_bidib_train_state_intern *bidib_state_get_train_state_ref(const char *train);

/**
 * Returns the reference to the booster state with the given node address.
 * Shall only be called with trackstate_boosters_mutex acquired.
 * Note: uses bidib_boards_rwlock internally.
 *
 * @param node_address the node address of the board.
 * @return NULL if not found, otherwise the reference to the booster state.
 */
t_bidib_booster_state *bidib_state_get_booster_state_ref_by_nodeaddr(
		t_bidib_node_address node_address);

/**
 * Returns the reference to the track output state with the given node address.
 * Shall only be called with trackstate_track_outputs_mutex acquired.
 * Note: uses bidib_boards_rwlock internally. 
 *
 * @param node_address the node address of the board.
 * @return NULL if not found, otherwise the reference to the track output state.
 */
t_bidib_track_output_state *bidib_state_get_track_output_state_ref_by_nodeaddr(
		t_bidib_node_address node_address);


#endif
