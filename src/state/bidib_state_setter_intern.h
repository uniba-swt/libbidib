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

#ifndef BIDIB_STATE_SETTER_INTERN_H
#define BIDIB_STATE_SETTER_INTERN_H

#include <stdint.h>

#include "bidib_state_intern.h"


/**
 * Sets the state of a vendor-specific configuration.
 *
 * @param node_address the node address of the new node.
 * @param length the length of the diagnose list.
 * @param value_list the value list.
 */
void bidib_state_vendor(t_bidib_node_address node_address, uint8_t length,
			            const uint8_t *const value_list, unsigned int action_id);

/**
 * Sets the state of a node to unavailable.
 *
 * @param unique_id the unique id of the lost board.
 */
void bidib_state_node_lost(t_bidib_unique_id_mod unique_id);

/**
 * Sets the state of a node to available / adds new node.
 *
 * @param node_address the node address of the new node.
 * @param local_addr the local address of the node.
 * @param unique_id the unique id of the new node.
 */
void bidib_state_node_new(t_bidib_node_address node_address, uint8_t local_addr,
                          t_bidib_unique_id_mod unique_id);

/**
 * Sets the state of an accessory.
 *
 * @param node_address the node address of the sender.
 * @param number the accessory number.
 * @param aspect the state of the accessory.
 * @param total the number of possible aspects.
 * @param execution the execution state of the accessory.
 * @param wait details of the execution state.
 * @param action_id reference number to a high level function call.
 */
void bidib_state_accessory_state(t_bidib_node_address node_address, uint8_t number,
                                 uint8_t aspect, uint8_t total, uint8_t execution,
                                 uint8_t wait, unsigned int action_id);

/**
 * Sets the state of a booster.
 *
 * @param node_address the node address of the sender.
 * @param state the new power state of the booster.
 */
void bidib_state_boost_state(t_bidib_node_address node_address, uint8_t power_state);

/**
 * Sets the current state of a track output.
 *
 * @param node_address the node address of the sender.
 * @param state the current state of the track output.
 * @param action_id reference number to a high level function call.
 */
void bidib_state_cs_state(t_bidib_node_address node_address, uint8_t state,
                          unsigned int action_id);

/**
 * Sets the ack info for an dcc drive command.
 *
 * @param dcc_address the dcc address of the train.
 * @param ack the acknowledgement.
 * @param action_id reference number to a high level function call.
 */
void bidib_state_cs_drive_ack(t_bidib_dcc_address dcc_address, uint8_t ack,
                              unsigned int action_id);

/**
 * Sets the ack info for an dcc accessory.
 * Must be called with bidib_state_track_rwlock write lock acquired,
 * and bidib_state_boards_rwlock read or write lock acquired.
 *
 * @param node_address the node address of the board.
 * @param dcc_address the dcc address of the accessory.
 * @param ack the acknowledgement.
 */
void bidib_state_cs_accessory_ack(t_bidib_node_address node_address,
                                  t_bidib_dcc_address dcc_address, uint8_t ack);

/**
 * Sets the reported info about a manual train drive operation.
 * Must only be called with bidib_state_trains_rwlock >=read acquired.
 *
 * @param params the parameters for the drive command.
 */
void bidib_state_cs_drive(t_bidib_cs_drive_mod params);

/**
 * Sets the reported info about manual dcc accessory operation.
 * Must only be called with bidib_state_track_rwlock write acquired.
 *
 * @param node_address the node address of the board.
 * @param dcc_address the dcc address of the accessory.
 * @param data the current state of the accessory.
 */
void bidib_state_cs_accessory_manual(t_bidib_node_address node_address,
                                     t_bidib_dcc_address dcc_address, uint8_t data);

/**
 * Sets the new state for a dcc accessory.
 * Must only be called with bidib_state_track_rwlock write acquired,
 * and with bidib_state_boards_rwlock >=read acquired.
 *
 * @param node_address the node address of the board.
 * @param params the parameters for the dcc accessory.
 */
void bidib_state_cs_accessory(t_bidib_node_address node_address,
                              t_bidib_cs_accessory_mod params);

/**
 * Sets the current state of a peripheral port.
 *
 * @param node_address the node address of the board.
 * @param port the port of the peripheral.
 * @param portstat the current state of the peripheral.
 * @param action_id reference number to a high level function call.
 */
void bidib_state_lc_stat(t_bidib_node_address node_address, t_bidib_peripheral_port port,
                         uint8_t portstat, unsigned int action_id);

/**
 * Sets the current wait information of a peripheral port.
 *
 * @param node_address the node address of the board.
 * @param port the port of the peripheral.
 * @param time the current wait time.
 */
void bidib_state_lc_wait(t_bidib_node_address node_address, t_bidib_peripheral_port port,
                         uint8_t time);

/**
 * Sets the occupancy state of a segment.
 * Must be called with none of trains/track/board
 * rwlocks acquired.
 *
 * @param node_address the node address of the board.
 * @param number the number of the segment.
 * @param occ whether occupied or not.
 */
void bidib_state_bm_occ(t_bidib_node_address node_address, uint8_t number, bool occ);

/**
 * Sets the occupancy states of multiple segments.
 * 
 * @param node_address the node address of the board.
 * @param number the number of the first segment.
 * @param size the number of segments.
 * @param data the occupancy data.
 */
void bidib_state_bm_multiple(t_bidib_node_address node_address, uint8_t number,
                             uint8_t size, const uint8_t *const data);

/**
 * Sets the confidence for occupancy reports of the board.
 *
 * @param node_address the node address of the board.
 * @param conf_void the void state.
 * @param freeze the freeze state.
 * @param nosignal the nosignal state.
 * @param action_id reference number to a high level function call.
 */
void bidib_state_bm_confidence(t_bidib_node_address node_address, uint8_t conf_void,
                               uint8_t freeze, uint8_t nosignal, unsigned int action_id);

/**
 * Sets a reported dcc address for a segment.
 *
 * @param node_address the node address of the board.
 * @param number the number of the segment.
 * @param address_count the number of reported addresses.
 * @param addresses the addresses.
 */
void bidib_state_bm_address(t_bidib_node_address node_address, uint8_t number,
                            uint8_t address_count, const uint8_t *const addresses);

/**
 * Sets the current for segment.
 *
 * @param node_address the node address of the board.
 * @param number the number of the segment.
 * @param current the current.
 */
void bidib_state_bm_current(t_bidib_node_address node_address, uint8_t number,
                            uint8_t current);

/**
 * Sets the speed of a train reported by an occupancy detector.
 *
 * @param dcc_address the dcc address of the train.
 * @param speedl the lowbyte of the speed.
 * @param speedh the highbyte of the speed.
 */
void bidib_state_bm_speed(t_bidib_dcc_address dcc_address, uint8_t speedl,
                          uint8_t speedh);

/**
 * Sets the state of a train decoder.
 *
 * @param dcc_address the dcc address of the train.
 * @param dyn_num indicates which state is transmitted.
 * @param value the current state.
 * @param action_id reference number to a high level function call.
 */
void bidib_state_bm_dyn_state(t_bidib_dcc_address dcc_address, uint8_t dyn_num,
                              uint8_t value, unsigned int action_id);

/**
 * Sets the diagnostic state of a booster.
 *
 * @param node_address the node address of the board.
 * @param length the length of the diagnose list.
 * @param diag_list the diagnose list.
 * @param action_id reference number to a high level function call.
 */
void bidib_state_boost_diagnostic(t_bidib_node_address node_address, uint8_t length,
                                  const uint8_t *const diag_list, unsigned int action_id);


#endif
