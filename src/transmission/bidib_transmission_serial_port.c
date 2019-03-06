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

#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <termios.h>
#include <stdlib.h>

#include "../../include/definitions/bidib_definitions_custom.h"
#include "bidib_transmission_intern.h"
#include "../../include/bidib.h"

/*
 * Credits: https://www.cmrr.umn.edu/~strupp/serial.html
 */

#define BUFF_LEN 1


static int fd;
static unsigned char buff[BUFF_LEN];


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
	tcsetattr(fd, TCSANOW, &options);
}

int bidib_detect_baudrate(void) {
	size_t remaining_tries = 3;
	syslog(LOG_INFO, "Trying baud rate 1000000");
	while (remaining_tries--) {
		if (bidib_communication_works()) {
			break;
		} else {
			if (remaining_tries == 2) {
				bidib_node_state_table_reset();
				syslog(LOG_INFO, "Trying baud rate 115200");
				bidib_serial_port_set_options(B115200);
			} else if (remaining_tries == 1) {
				bidib_node_state_table_reset();
				syslog(LOG_INFO, "Trying baud rate 19200");
				bidib_serial_port_set_options(B19200);
			} else if (remaining_tries == 0) {
				syslog(LOG_ERR, "%s", "Couldn't find working baud rate");
				return 1;
			}
		}
	}
	return 0;
}

int bidib_serial_port_init(const char *device) {
	fd = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK | O_SYNC);
	if (fd < 0) {
		syslog(LOG_ERR, "%s", "Error while opening serial port");
		return 1;
	} else {
		#ifndef __APPLE__
			bidib_serial_port_set_options(B1000000);
		#else
			bidib_serial_port_set_options(B115200);
		#endif
		syslog(LOG_INFO, "%s", "Serial port opened");
		return 0;
	}
}

unsigned char bidib_serial_port_read(int *byte_read) {
	*byte_read = (read(fd, buff, BUFF_LEN) == 1);
	return buff[0];
}

void bidib_serial_port_write(unsigned char msg) {
	if (write(fd, &msg, 1) != 1) {
		syslog(LOG_ERR, "%s", "Error while sending data via serial port");
	}
}

void bidib_serial_port_close(void) {
	if (fd != 0) {
		close(fd);
		fd = 0;
		syslog(LOG_INFO, "%s", "Serial port closed");
	}
}
