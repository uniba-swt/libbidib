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

#ifndef BIDIB_HIGHLEVEL_UTIL_H
#define BIDIB_HIGHLEVEL_UTIL_H

#include <stdint.h>

#include "../definitions/bidib_definitions_custom.h"


/**
 * Starts the system, handles the connection via two function pointers. Also
 * configures the syslog file. This must be run before all other usages of the
 * library.
 *
 * @param read a pointer to a function, which reads a byte from the connected
 * BiDiB interface. This function must not be blocking.
 * @param write a pointer to a function, which sends a byte to the connected
 * BiDiB interface.
 * @param config_dir the directory in which the config files are stored, use
 * NULL if no configs should be used.
 * @param flush_interval the interval for automatic flushing in ms. If 0,
 * automatic flushing is disabled.
 * @return 0 if configs are valid, otherwise 1.
 */
int bidib_start_pointer(uint8_t (*read)(int *), void (*write)(uint8_t),
                        const char *config_dir, unsigned int flush_interval);

/**
 * Starts the system, handles the connection via a serial port. Also configures
 * the syslog file. This must be run before all other usages of the library.
 *
 * @param device the path to the device where the bidib interface is connected.
 * @param config_dir the directory in which the config files are stored, use
 * NULL if no configs should be used.
 * @param flush_interval the interval for automatic flushing in ms. If 0,
 * automatic flushing is disabled.
 * @return 0 if serial port connection was successful and configs are valid,
 * otherwise 1.
 */
int bidib_start_serial(const char *device, const char *config_dir,
                       unsigned int flush_interval);

/**
 * Clears the memory allocated by the BiDiB library and closes the log.
 * Run this before your application terminates to free allocated memory.
 */
void bidib_stop(void);

/**
 * Returns and removes the oldest received message from the queue. It's the
 * calling function's responsibility to free the memory of the message.
 *
 * @return NULL if there is no message, otherwise the oldest message.
 */
uint8_t *bidib_read_message(void);

/**
 * Returns and removes the oldest error message from the queue. It's the calling
 * function's responsibility to free the memory of the error message.
 *
 * @return NULL if there is no error message, otherwise the oldest error
 * message.
 */
uint8_t *bidib_read_error_message(void);

/**
 * Sends all cached messages. Call this method every x time units to be sure
 * messages aren't cached too long.
 */
void bidib_flush(void);


#endif
