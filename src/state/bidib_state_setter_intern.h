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

#ifndef BIDIB_STATE_SETTER_INTERN_H
#define BIDIB_STATE_SETTER_INTERN_H

#include "../../include/bidib.h"
#include "bidib_state_intern.h"


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
void bidib_state_node_new(t_bidib_node_address node_address, unsigned char local_addr,
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
 */
void bidib_state_accessory_state(t_bidib_node_address node_address, unsigned char number,
                                 unsigned char aspect, unsigned char total,
                                 unsigned char execution, unsigned char wait);

/**
 * Sets the state of a booster.
 *
 * @param node_address the node address of the sender.
 * @param state the new power state of the booster.
 */
void bidib_state_boost_stat(t_bidib_node_address node_address, unsigned char power_state);

/**
 * Sets the current state of a track output.
 *
 * @param node_address the node address of the sender.
 * @param state the current state of the track output.
 */
void bidib_state_cs_state(t_bidib_node_address node_address, unsigned char state);

/**
 * Sets the ack info for an dcc drive command.
 *
 * @param dcc_address the dcc address of the train.
 * @param ack the acknowledgement.
 */
void bidib_state_cs_drive_ack(t_bidib_dcc_address dcc_address, unsigned char ack);

/**
 * Sets the ack info for an dcc accessory.
 *
 * @param node_address the node address of the board.
 * @param dcc_address the dcc address of the accessory.
 * @param ack the acknowledgement.
 */
void bidib_state_cs_accessory_ack(t_bidib_node_address node_address,
                                  t_bidib_dcc_address dcc_address, unsigned char ack);

/**
 * Sets the reported info about a manual train drive operation.
 *
 * @param params the parameters for the drive command.
 */
void bidib_state_cs_drive(t_bidib_cs_drive_mod params);

/**
 * Sets the reported info about manual dcc accessory operation.
 *
 * @param node_address the node address of the board.
 * @param dcc_address the dcc address of the accessory.
 * @param data the current state of the accessory.
 */
void bidib_state_cs_accessory_manual(t_bidib_node_address node_address,
                                     t_bidib_dcc_address dcc_address, unsigned char data);

/**
 * Sets the new state for a dcc accessory.
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
 */
void bidib_state_lc_stat(t_bidib_node_address node_address, t_bidib_peripheral_port port,
                         unsigned char portstat);

/**
 * Sets the current wait information of a peripheral port.
 *
 * @param node_address the node address of the board.
 * @param port the port of the peripheral.
 * @param time the current wait time.
 */
void bidib_state_lc_wait(t_bidib_node_address node_address, t_bidib_peripheral_port port,
                         unsigned char time);

/**
 * Sets the occupancy state of a segment.
 *
 * @param node_address the node address of the board.
 * @param number the number of the segment.
 * @param occ whether occupied or not.
 */
void bidib_state_bm_occ(t_bidib_node_address node_address, unsigned char number, bool occ);

/**
 * Sets the occupancy states of multiple segments.
 *
 * @param node_address the node address of the board.
 * @param number the number of the first segment.
 * @param size the number of segments.
 * @param data the occupancy data.
 */
void bidib_state_bm_multiple(t_bidib_node_address node_address, unsigned char number,
                             unsigned char size, unsigned char *data);

/**
 * Sets the confidence for occupancy reports of the board.
 *
 * @param node_address the node address of the board.
 * @param conf_void the void state.
 * @param freeze the freeze state.
 * @param nosignal the nosignal state.
 */
void bidib_state_bm_confidence(t_bidib_node_address node_address, unsigned char conf_void,
                               unsigned char freeze, unsigned char nosignal);

/**
 * Sets a reported dcc address for a segment.
 *
 * @param node_address the node address of the board.
 * @param number the number of the segment.
 * @param address_count the number of reported addresses.
 * @param addresses the addresses.
 */
void bidib_state_bm_address(t_bidib_node_address node_address, unsigned char number,
                            unsigned char address_count, unsigned char *addresses);

/**
 * Sets the current for segment.
 *
 * @param node_address the node address of the board.
 * @param number the number of the segment.
 * @param current the current.
 */
void bidib_state_bm_current(t_bidib_node_address node_address, unsigned char number,
                            unsigned char current);

/**
 * Sets the speed of a train reported by an occupancy detector.
 *
 * @param dcc_address the dcc address of the train.
 * @param speedl the lowbyte of the speed.
 * @param speedh the highbyte of the speed.
 */
void bidib_state_bm_speed(t_bidib_dcc_address dcc_address, unsigned char speedl,
                          unsigned char speedh);

/**
 * Sets the state of a train decoder.
 *
 * @param dcc_address the dcc address of the train.
 * @param dyn_num indicates which state is transmitted.
 * @param value the current state.
 */
void bidib_state_bm_dyn_state(t_bidib_dcc_address dcc_address, unsigned char dyn_num,
                              unsigned char value);

/**
 * Sets the diagnostic state of a booster.
 *
 * @param node_address the node address of the board.
 * @param length the length of the diagnose list.
 * @param diag_list the diagnose list.
 */
void bidib_state_boost_diagnostic(t_bidib_node_address node_address, unsigned char length,
                                  unsigned char *diag_list);


#endif
