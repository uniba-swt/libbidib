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

#ifndef BIDIB_TRANSMISSION_SERIAL_PORT_INTERN_H
#define BIDIB_TRANSMISSION_SERIAL_PORT_INTERN_H

#include <termios.h>


/**
 * Initializes the serial port for sending and receiving.
 *
 * @param device the path to the serial device.
 * @return 0 if successful, otherwise 1.
 */
int bidib_serial_port_init(const char *device);

/**
 * Detects the baud rate for the serial port.
 *
 * @return 0 if successful, otherwise 1.
 */
int bidib_detect_baudrate(void);

/**
 * Sends a byte via the serial port to the BiDiB interface.
 *
 * @param msg the byte.
 */
void bidib_serial_port_write(unsigned char msg);

/**
 * Reads a byte from the serial port where the BiDiB interface is connected to.
 * The method blocks until a message is received.
 *
 * @param byte_read set to true if a byte was read.
 * @return the byte.
 */
unsigned char bidib_serial_port_read(int *byte_read);

/**
 * Closes the serial port.
 */
void bidib_serial_port_close(void);


#endif
