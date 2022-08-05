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

#ifndef BIDIB_STATE_GETTER_INTERN_H
#define BIDIB_STATE_GETTER_INTERN_H

#include <stdint.h>

#include "../../include/bidib.h"
#include "bidib_state_intern.h"


/**
 * Returns the reference to the board with the given id.
 *
 * @param board the id of the board.
 * @return NULL if not found, otherwise the reference to the board.
 */
t_bidib_board *bidib_state_get_board_ref(const char *board);

/**
 * Returns the reference to the board with the given unique id.
 *
 * @param unique_id the unique id of the board.
 * @return NULL if not found, otherwise the reference to the board.
 */
t_bidib_board *bidib_state_get_board_ref_by_uniqueid(t_bidib_unique_id_mod unique_id);

/**
 * Returns the reference to the booster state with the given id.
 *
 * @param booster the id of the booster.
 * @return NULL if not found, otherwise the reference to the booster state.
 */
t_bidib_booster_state *bidib_state_get_booster_state_ref(const char *booster);

/**
 * Returns the reference to the track output state with the given id.
 *
 * @param track_output the id of the track output state.
 * @return NULL if not found, otherwise the reference to the track output state.
 */
t_bidib_track_output_state *bidib_state_get_track_output_state_ref(const char *track_output);

/**
 * Returns the reference to the board accessory mapping with the given id.
 *
 * @param accessory the id of the board accessory.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the board accessory mapping.
 */
t_bidib_board_accessory_mapping *bidib_state_get_board_accessory_mapping_ref(
		const char *accessory, bool point);

/**
 * Returns the reference to the board accessory mapping with the given number.
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
 *
 * @param accessory the id of the board accessory state.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the board accessory state.
 */
t_bidib_board_accessory_state *bidib_state_get_board_accessory_state_ref(const char *accessory,
                                                                         bool point);

/**
 * Returns the reference to the dcc accessory mapping with the given id.
 *
 * @param accessory the id of the dcc accessory.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the dcc accessory mapping.
 */
t_bidib_dcc_accessory_mapping *bidib_state_get_dcc_accessory_mapping_ref(
		const char *accessory, bool point);

/**
 * Returns the reference to the dcc accessory mapping with the given dcc address.
 * Must be called with bidib_state_boards_rwlock read lock acquired.
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
 * Must be called with bidib_state_track_rwlock read lock acquired.
 *
 * @param accessory the id of the dcc accessory state.
 * @param point whether the accessory is a point or a signal.
 * @return NULL if not found, otherwise the reference to the dcc accessory state.
 */
t_bidib_dcc_accessory_state *bidib_state_get_dcc_accessory_state_ref(const char *accessory,
                                                                     bool point);

/**
 * Returns the reference to the peripheral mapping with the given id.
 *
 * @param peripheral the id of the peripheral.
 * @return NULL if not found, otherwise the reference to the peripheral mapping.
 */
t_bidib_peripheral_mapping *bidib_state_get_peripheral_mapping_ref(const char *peripheral);

/**
 * Returns the reference to the peripheral mapping with the given port.
 * Must be called with the bidib_state_boards_rwlock acquired.
 *
 * @param node_address the node address of the board.
 * @param port the port of the peripheral.
 * @return NULL if not found, otherwise the reference to the peripheral mapping.
 */
t_bidib_peripheral_mapping *bidib_state_get_peripheral_mapping_ref_by_port(
		t_bidib_node_address node_address, t_bidib_peripheral_port port);

/**
 * Returns the reference to the peripheral state with the given id.
 *
 * @param peripheral the id of the peripheral.
 * @return NULL if not found, otherwise the reference to the peripheral state.
 */
t_bidib_peripheral_state *bidib_state_get_peripheral_state_ref(const char *peripheral);

/**
 * Returns the reference to the segment state with the given id.
 *
 * @param segment the id of the segment.
 * @return NULL if not found, otherwise the reference to the segment state.
 */
t_bidib_segment_state_intern *bidib_state_get_segment_state_ref(const char *segment);

/**
 * Returns the occupancy state of a section.
 *
 * @param segment the id of the segment.
 * @return the occupancy state. Must be freed by the caller.
 */
t_bidib_segment_state_intern bidib_state_get_segment_state(const t_bidib_segment_state_intern *segment);


/**
 * Returns the reference to the segment state with the given id.
 * Must be called with bidib_track_state_rwlock read acquired.
 *
 * @param node_address the node address of the board.
 * @param number the number on the board.
 * @return NULL if not found, otherwise the reference to the segment state.
 */
t_bidib_segment_state_intern *bidib_state_get_segment_state_ref_by_nodeaddr(
		t_bidib_node_address node_address, uint8_t number);

/**
 * Returns the reference to the board with the given node address.
 *
 * @param node_address the node address of the board.
 * @return NULL if not found, otherwise the reference to the board.
 */
t_bidib_board *bidib_state_get_board_ref_by_nodeaddr(t_bidib_node_address node_address);

/**
 * Returns the reference to the train with the given id.
 *
 * @param train the id of the train.
 * @return NULL if not found, otherwise the reference to the train.
 */
t_bidib_train *bidib_state_get_train_ref(const char *train);

/**
 * Returns the reference to the train state with the given dcc address.
 *
 * @param dcc_address the dcc address of the train.
 * @return NULL if not found, otherwise the reference to the train state.
 */
t_bidib_train_state_intern *bidib_state_get_train_state_ref_by_dccaddr(
		t_bidib_dcc_address dcc_address);

/**
 * Returns the reference to the train peripheral state with the given bit.
 *
 * @param train_state the train state.
 * @param bit the bit of the peripheral.
 * @return NULL if not found, otherwise the reference to the train peripheral state.
 */
t_bidib_train_peripheral_state *bidib_state_get_train_peripheral_state_by_bit(
		t_bidib_train_state_intern *train_state, uint8_t bit);

/**
 * Returns the reference to the train state with the given id.
 *
 * @param train the id of the train state.
 * @return NULL if not found, otherwise the reference to the train state.
 */
t_bidib_train_state_intern *bidib_state_get_train_state_ref(const char *train);

/**
 * Returns the reference to the booster state with the given node address.
 * Must be called with bidib_track_state_rwlock read acquired.
 *
 * @param node_address the node address of the board.
 * @return NULL if not found, otherwise the reference to the booster state.
 */
t_bidib_booster_state *bidib_state_get_booster_state_ref_by_nodeaddr(
		t_bidib_node_address node_address);

/**
 * Returns the reference to the track output state with the given node address.
 * Must be called with bidib_track_state_rwlock read acquired.
 *
 * @param node_address the node address of the board.
 * @return NULL if not found, otherwise the reference to the track output state.
 */
t_bidib_track_output_state *bidib_state_get_track_output_state_ref_by_nodeaddr(
		t_bidib_node_address node_address);


#endif
