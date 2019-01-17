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

#ifndef BIDIB_LOWLEVEL_OCCUPANCY_H
#define BIDIB_LOWLEVEL_OCCUPANCY_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Queries the status of all occupancy detectors in a certain area.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param start the start of the queried detector bits. Must be divisible by 8.
 * @param end the (exclusive) end of the queried detector bits. Must be divisible by 8.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_bm_get_range(t_bidib_node_address node_address,
                             uint8_t start, uint8_t end,
                             unsigned int action_id);

/**
 * Transmits the occupancy states back to the detector.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param mnum the base address of the track section. Must be divisible by 8.
 * @param size number of reported bits, range 8...128. Must be divisible by 8.
 * @param data the detector data.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_bm_mirror_multiple(t_bidib_node_address node_address,
                                   uint8_t mnum, uint8_t size,
                                   uint8_t *data, unsigned int action_id);

/**
 * Transmits a single occupancy status back to the detector.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param mnum the base address of the track section. Must be divisible by 8.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_bm_mirror_occ(t_bidib_node_address node_address,
                              uint8_t mnum, unsigned int action_id);

/**
 * Transmits a single free status back to the detector.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param mnum the base address of the track section. Must be divisible by 8.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_bm_mirror_free(t_bidib_node_address node_address,
                               uint8_t mnum, unsigned int action_id);

/**
 * Retransmits the recognized loco addresses for a range of sections.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param start the start index.
 * @param end the (exclusive) end index.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_bm_addr_get_range(t_bidib_node_address node_address,
                                  uint8_t start, uint8_t end,
                                  unsigned int action_id);

/**
 * Queries the current 'quality' of the occupancy detection.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_bm_get_confidence(t_bidib_node_address node_address,
                                  unsigned int action_id);

/**
 * Transmits a position report of a decoder back to the detector.
 *
 * @param node_address the three bytes on top of the address stack.
 * @param type kind of the position information, 0 for local badge, 1...255 reserved.
 * @param location_low lowbyte of the position information.
 * @param location_high highbyte of the position information.
 * @param action_id reference number to a high level function call, 0 to signal
 * no reference.
 */
void bidib_send_msg_bm_mirror_position(t_bidib_node_address node_address, uint8_t type,
                                       uint8_t location_low, uint8_t location_high,
                                       unsigned int action_id);


#endif
