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

#include <yaml.h>
#include <syslog.h>
#include <stdbool.h>

#include "../state/bidib_state_intern.h"
#include "bidib_config_parser_intern.h"

/*
 * Credits: https://www.wpsoftware.net/andrew/pages/libyaml.html
 */

bool bidib_config_init_parser(const char *config_dir, const char *config_file,
                              FILE **fh, yaml_parser_t *parser) {
	size_t full_path_length = strlen(config_dir) + strlen(config_file) + 1;
	char full_path[full_path_length];
	snprintf(full_path, full_path_length, "%s%s", config_dir, config_file);
	*fh = fopen(full_path, "r");

	if (*fh == NULL) {
		syslog(LOG_ERR, "%s", "Failed to open board-config");
		return true;
	} else if (!yaml_parser_initialize(parser)) {
		fclose(*fh);
		syslog(LOG_ERR, "%s", "Failed to initialize config parser");
		return true;
	}

	yaml_parser_set_input_file(parser, *fh);
	return false;
}

bool bidib_string_to_byte(char *string, uint8_t *byte) {
	if (string != NULL && strlen(string) > 0) {
		char *end;
		long number;
		if (strlen(string) > 2 && string[0] == '0' && string[1] == 'x') {
			number = strtol(string + 2, &end, 16);
		} else {
			number = strtol(string, &end, 10);
		}
		if (*end != '\0' || number > 255 || number < 0) {
			return true;
		} else {
			*byte = (uint8_t) number;
			return false;
		}
	}
	return true;
}

bool bidib_string_to_uid(char *string, t_bidib_unique_id_mod *uid) {
	if (string == NULL || strlen(string) != 16 || string[0] != '0' ||
	    string[1] != 'x') {
		return true;
	}
	char single_bytes[7][5];
	for (size_t i = 2; i < 16; i += 2) {
		size_t current_index = (i / 2) - 1;
		single_bytes[current_index][0] = '0';
		single_bytes[current_index][1] = 'x';
		single_bytes[current_index][2] = string[i];
		single_bytes[current_index][3] = string[i + 1];
		single_bytes[current_index][4] = '\0';
	}
	if (bidib_string_to_byte(single_bytes[0], &uid->class_id) ||
	    bidib_string_to_byte(single_bytes[1], &uid->class_id_ext) ||
	    bidib_string_to_byte(single_bytes[2], &uid->vendor_id) ||
	    bidib_string_to_byte(single_bytes[3], &uid->product_id1) ||
	    bidib_string_to_byte(single_bytes[4], &uid->product_id2) ||
	    bidib_string_to_byte(single_bytes[5], &uid->product_id3) ||
	    bidib_string_to_byte(single_bytes[6], &uid->product_id4)) {
		return true;
	}
	return false;
}

bool bidib_string_to_dccaddr(char *string, t_bidib_dcc_address *dcc_address) {
	if (string == NULL || strlen(string) != 6 || string[0] != '0' ||
	    string[1] != 'x') {
		return true;
	}
	char fst_byte[5] = {'0', 'x', string[2], string[3], '\0'};
	char snd_byte[5] = {'0', 'x', string[4], string[5], '\0'};
	if (bidib_string_to_byte(fst_byte, &dcc_address->addrh) ||
	    bidib_string_to_byte(snd_byte, &dcc_address->addrl)) {
		return true;
	}
	return false;
}

bool bidib_string_to_port(char *string, t_bidib_peripheral_port *port) {
	if (string == NULL || strlen(string) != 6 || string[0] != '0' ||
	    string[1] != 'x') {
		return true;
	}
	char fst_byte[5] = {'0', 'x', string[2], string[3], '\0'};
	char snd_byte[5] = {'0', 'x', string[4], string[5], '\0'};
	if (bidib_string_to_byte(fst_byte, &port->port1) ||
	    bidib_string_to_byte(snd_byte, &port->port0)) {
		return true;
	}
	return false;
}

bool bidib_config_parse_scalar_then_section(yaml_parser_t *parser, char *scalar,
                                            bool (*section_elem_action)(yaml_parser_t *)) {
	yaml_event_t event;
	bool error = false;
	bool boards_read = false;
	bool seq_read = false;

	while (!error) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		} else if (event.type == YAML_STREAM_END_EVENT) {
			yaml_event_delete(&event);
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				// ignore
				break;
			case YAML_STREAM_END_EVENT:
				// checked before switch case
				break;
			case YAML_DOCUMENT_START_EVENT:
				if (boards_read) {
					error = true;
				}
				break;
			case YAML_DOCUMENT_END_EVENT:
				if (!boards_read || !seq_read) {
					error = true;
				}
				break;
			case YAML_SEQUENCE_START_EVENT:
				if (!boards_read || seq_read) {
					error = true;
				} else {
					seq_read = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (!boards_read || !seq_read) {
					error = true;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (boards_read && !seq_read) {
					error = true;
				} else if (boards_read) {
					error = section_elem_action(parser);
				}
				break;
			case YAML_MAPPING_END_EVENT:
				if (!boards_read || !seq_read) {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				if (!boards_read && !seq_read &&
				    !strcmp((char *) event.data.scalar.value, scalar)) {
					boards_read = true;
				} else {
					error = true;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	return error;
}

bool bidib_config_parse(const char *config_dir) {
	if (config_dir == NULL) {
		syslog(LOG_INFO, "%s", "No config loaded, because no directory submitted");
		return false;
	} else if (bidib_config_parse_board_config(config_dir) ||
	           bidib_config_parse_track_config(config_dir) ||
	           bidib_config_parse_train_config(config_dir)) {
		return true;
	}
	syslog(LOG_INFO, "%s", "Config loaded successfully");
	return false;
}
