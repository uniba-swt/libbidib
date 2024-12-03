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

#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <stdint.h>

#include "bidib_transmission_serial_port_intern.h"
#include "bidib_transmission_intern.h"
#include "../../include/highlevel/bidib_highlevel_util.h"

/*
 * Credits: https://www.cmrr.umn.edu/~strupp/serial.html
 */

#define BUFF_LEN 1


static int fd;
static uint8_t buff[BUFF_LEN];


static void bidib_serial_port_set_options(speed_t baudrate) {
	struct termios options;
	tcgetattr(fd, &options);
	cfsetispeed(&options, baudrate);
	cfsetospeed(&options, baudrate);
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= (CLOCAL | CREAD | CRTSCTS | CS8);
	options.c_iflag = 0;
	options.c_iflag |= (IGNPAR | IMAXBEL);
	options.c_oflag = 0;
	options.c_lflag = 0;
	#ifndef __APPLE__
		options.c_line = 0;
	#endif
	tcsetattr(fd, TCSAFLUSH, &options);
}

int bidib_detect_baudrate(void) {
	size_t remaining_tries;
	#ifndef __APPLE__
		remaining_tries = 3;
		syslog_libbidib(LOG_INFO, "Trying baud rate 1000000");
	#else
		remaining_tries = 2;
		syslog_libbidib(LOG_INFO, "Trying baud rate 115200");
	#endif
	while (remaining_tries--) {
		if (bidib_communication_works()) {
			break;
		} else {
			if (remaining_tries == 2) {
				bidib_node_state_table_reset(true);
				syslog_libbidib(LOG_INFO, "Trying baud rate 115200");
				bidib_serial_port_set_options(B115200);
			} else if (remaining_tries == 1) {
				bidib_node_state_table_reset(true);
				syslog_libbidib(LOG_INFO, "Trying baud rate 19200");
				bidib_serial_port_set_options(B19200);
			} else if (remaining_tries == 0) {
				syslog_libbidib(LOG_ERR, "Couldn't find working baud rate");
				return 1;
			}
		}
	}
	return 0;
}

int bidib_serial_port_init(const char *device) {
	fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC);
	if (fd < 0) {
		syslog_libbidib(LOG_ERR, "Error while opening serial port");
		return 1;
	} else {
		#ifndef __APPLE__
			bidib_serial_port_set_options(B1000000);
		#else
			bidib_serial_port_set_options(B115200);
		#endif
		syslog_libbidib(LOG_INFO, "Serial port opened");
		return 0;
	}
}

uint8_t bidib_serial_port_read(int *byte_read) {
	*byte_read = (read(fd, buff, BUFF_LEN) == 1);
	return buff[0];
}

void bidib_serial_port_write(uint8_t msg) {
	if (write(fd, &msg, 1) != 1) {
		syslog_libbidib(LOG_ERR, "Error while sending data via serial port (single byte write)");
	}
}

void bidib_serial_port_write_n(uint8_t *msg, int32_t len) {
	if (msg == NULL || len <= 0 || (write(fd, msg, len) != len)) {
		syslog_libbidib(LOG_ERR, "Error while sending data via serial port (%d-byte write)", len);
	}
}

void bidib_serial_port_close(void) {
	if (fd != 0) {
		close(fd);
		fd = 0;
		syslog_libbidib(LOG_INFO, "Serial port closed");
	}
}
