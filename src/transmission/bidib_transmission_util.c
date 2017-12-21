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

#include <zconf.h>
#include <stdlib.h>
#include <syslog.h>
#include <stdio.h>
#include "bidib_transmission_intern.h"
#include "../../include/bidib.h"


unsigned char bidib_extract_msg_type(unsigned char *message) {
	int i = 0;
	do {
		i++;
	} while (message[i] != 0x00);
	return message[i + 2];
}

void bidib_extract_address(unsigned char *message, unsigned char *dest) {
	int i = 0;
	do {
		dest[i] = message[i + 1];
		i++;
	} while (message[i] != 0x00);
	while (i < 4) {
		dest[i] = 0x00;
		i++;
	}
}

unsigned char bidib_extract_seq_num(unsigned char *message) {
	int i = 1;
	while (message[i] != 0x00) {
		i++;
	}
	return message[++i];
}

int bidib_first_data_byte_index(unsigned char *message) {
	for (int i = 1; i <= message[0] - 3; i++) {
		if (message[i] == 0x00) {
			return i + 3;
		}
	}
	return -1;
}

bool bidib_communication_works(void) {
	t_bidib_node_address interface = {0x00, 0x00, 0x00};
	bidib_seq_num_enabled = false;
	bidib_send_sys_disable(0);
	bidib_flush();
	usleep(500000);
	bidib_send_sys_get_magic(interface, 0);
	bidib_flush();
	usleep(250000);
	unsigned char *message;
	while ((message = bidib_read_intern_message()) != NULL) {
		if (message[1] == 0x00 && bidib_extract_msg_type(message) == MSG_SYS_MAGIC) {
			free(message);
			syslog(LOG_INFO, "Connection to interface established");
			bidib_seq_num_enabled = true;
			return true;
		}
		free(message);
	}
	syslog(LOG_ERR, "Connection to interface could not be established");
	return false;
}

void bidib_build_message_hex_string(unsigned char *message, char *dest) {
	for (size_t i = 0; i <= message[0]; i++) {
		if (i != 0) {
			dest += sprintf(dest, " ");
		}
		dest += sprintf(dest, "0x%02x", message[i]);
	}
}
