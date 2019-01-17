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

#ifndef BIDIB_HIGHLEVEL_ADMIN_H
#define BIDIB_HIGHLEVEL_ADMIN_H

#include <stdint.h>


/**
 * Sends a ping to a BiDiB board.
 *
 * @param board the id of the board.
 * @param ping_byte the byte which is send to the board.
 * @return 0 if board is known, otherwise 1.
 */
int bidib_ping(const char *board, uint8_t ping_byte);

/**
 * Sets local identify indicator of a BiDiB board.
 *
 * @param board the id of the board.
 * @param state the state of the indicator, 0 for off, 1 for on.
 * @return 0 if board is known, otherwise 1.
 */
int bidib_identify(const char *board, uint8_t state);

/**
 * Queries the supported protocol version of a BiDiB board.
 *
 * @param board the id of the board.
 * @return 0 if board is known, otherwise 1.
 */
int bidib_get_protocol_version(const char *board);

/**
 * Queries the installed software version of a BiDiB board.
 *
 * @param board the id of the board.
 * @return 0 if board is known, otherwise 1.
 */
int bidib_get_software_version(const char *board);


#endif
