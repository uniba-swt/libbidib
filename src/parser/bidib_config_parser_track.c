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
#include <stdbool.h>

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../state/bidib_state_intern.h"
#include "bidib_config_parser_intern.h"
#include "../../include/bidib.h"
#include "../state/bidib_state_getter_intern.h"
#include "../../include/definitions/bidib_definitions_custom.h"


static bool initial_value_valid(GArray *aspect_list, const char *value,
                                bool dcc_aspects) {
	if (aspect_list == NULL || value == NULL) {
		return false;
	}
	for (size_t i = 0; i < aspect_list->len; i++) {
		if (dcc_aspects) {
			if (!strcmp(g_array_index(aspect_list, t_bidib_dcc_aspect, i).id->str,
			            value)) {
				return true;
			}
		} else {
			if (!strcmp(g_array_index(aspect_list, t_bidib_aspect, i).id->str,
			            value)) {
				return true;
			}
		}
	}
	syslog_libbidib(LOG_ERR, "Initial value %s not defined in aspects", value);
	return false;
}

typedef enum {
	ASPECT_START,
	ASPECT_ID_KEY, ASPECT_ID_VALUE,
	ASPECT_VALUE_KEY, ASPECT_VALUE_VALUE
} t_bidib_parser_aspect_scalar;

static bool bidib_config_parse_aspect(yaml_parser_t *parser, GArray *aspect_list) {
	yaml_event_t event;
	t_bidib_aspect aspect = {NULL, 0x00};
	bool error = false;
	bool done = false;
	t_bidib_parser_aspect_scalar last_scalar = ASPECT_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_END_EVENT:
				error = true;
				break;
			case YAML_MAPPING_START_EVENT:
				error = true;
				break;
			case YAML_MAPPING_END_EVENT:
				if (last_scalar == ASPECT_VALUE_VALUE) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				switch (last_scalar) {
					case ASPECT_START:
						if (!strcmp((char *) event.data.scalar.value, "id")) {
							last_scalar = ASPECT_ID_KEY;
						} else {
							error = true;
						}
						break;
					case ASPECT_ID_KEY:
						aspect.id = g_string_new((const gchar *) event.data.scalar.value);
						last_scalar = ASPECT_ID_VALUE;
						break;
					case ASPECT_ID_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "value")) {
							last_scalar = ASPECT_VALUE_KEY;
						} else {
							error = true;
						}
						break;
					case ASPECT_VALUE_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &aspect.value)) {
							error = true;
							syslog_libbidib(LOG_ERR, "Value of aspect %s is in wrong format",
							                aspect.id->str);
						} else {
							last_scalar = ASPECT_VALUE_VALUE;
						}
						break;
					case ASPECT_VALUE_VALUE:
						error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	t_bidib_aspect tmp;
	for (size_t i = 0; i < aspect_list->len; i++) {
		tmp = g_array_index(aspect_list, t_bidib_aspect, i);
		if (tmp.value == aspect.value || !strcmp(tmp.id->str, aspect.id->str)) {
			syslog_libbidib(LOG_ERR, "Aspect %s configured with same id or value "
			                "as another aspect", aspect.id->str);
			error = true;
			break;
		}
	}
	g_array_append_val(aspect_list, aspect);
	return error;
}

typedef enum {
	BOARD_SETUP_START,
	BOARD_SETUP_ID_KEY, BOARD_SETUP_ID_VALUE,
	BOARD_SETUP_POINTS_BOARD_KEY, BOARD_SETUP_POINTS_BOARD_VALUE,
	BOARD_SETUP_POINTS_DCC_KEY, BOARD_SETUP_POINTS_DCC_VALUE,
	BOARD_SETUP_SIGNALS_BOARD_KEY, BOARD_SETUP_SIGNALS_BOARD_VALUE,
	BOARD_SETUP_SIGNALS_DCC_KEY, BOARD_SETUP_SIGNALS_DCC_VALUE,
	BOARD_SETUP_PERIPHERALS_KEY, BOARD_SETUP_PERIPHERALS_VALUE,
	BOARD_SETUP_SEGMENTS_KEY, BOARD_SETUP_SEGMENTS_VALUE
} t_bidib_parser_board_setup_scalar;

typedef enum {
	BOARD_ACCESSORY_START,
	BOARD_ACCESSORY_ID_KEY, BOARD_ACCESSORY_ID_VALUE,
	BOARD_ACCESSORY_NUMBER_KEY, BOARD_ACCESSORY_NUMBER_VALUE,
	BOARD_ACCESSORY_ASPECTS_KEY, BOARD_ACCESSORY_ASPECTS_VALUE,
	BOARD_ACCESSORY_INITIAL_KEY, BOARD_ACCESSORY_INITIAL_VALUE
} t_bidib_parser_board_accessory_scalar;

static bool bidib_config_parse_single_board_accessory(yaml_parser_t *parser,
                                                      t_bidib_board *board,
                                                      t_bidib_parser_board_setup_scalar type) {
	yaml_event_t event;
	t_bidib_board_accessory_state accessory_state = {NULL, {NULL, 0x00, BIDIB_EXEC_STATE_REACHED, 0x00}};
	t_bidib_board_accessory_mapping mapping = {NULL, 0x00, NULL};
	t_bidib_state_initial_value initial_value = {NULL, NULL};
	bool error = false;
	bool done = false;
	bool in_seq = false;
	t_bidib_parser_board_accessory_scalar last_scalar = BOARD_ACCESSORY_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				if (in_seq) {
					error = true;
				} else {
					in_seq = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (in_seq && last_scalar == BOARD_ACCESSORY_ASPECTS_KEY) {
					last_scalar = BOARD_ACCESSORY_ASPECTS_VALUE;
					in_seq = false;
					if (mapping.aspects->len == 0) {
						error = true;
						syslog_libbidib(LOG_ERR, "No aspect configured for board point/signal %s",
						                mapping.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (in_seq && last_scalar == BOARD_ACCESSORY_ASPECTS_KEY) {
					error = bidib_config_parse_aspect(parser, mapping.aspects);
					if (error) {
						syslog_libbidib(LOG_ERR, "Error while parsing an aspect of board point/signal %s",
						                mapping.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_END_EVENT:
				if (!in_seq && (last_scalar == BOARD_ACCESSORY_INITIAL_VALUE ||
				                last_scalar == BOARD_ACCESSORY_ASPECTS_VALUE)) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				switch (last_scalar) {
					case BOARD_ACCESSORY_START:
						if (!strcmp((char *) event.data.scalar.value, "id")) {
							last_scalar = BOARD_ACCESSORY_ID_KEY;
						} else {
							error = true;
						}
						break;
					case BOARD_ACCESSORY_ID_KEY:
						accessory_state.id = malloc(
								sizeof(char) * (strlen((char *) event.data.scalar.value) + 1));
						strcpy(accessory_state.id, (char *) event.data.scalar.value);
						mapping.id = g_string_new(accessory_state.id);
						mapping.aspects = g_array_sized_new(FALSE, FALSE, sizeof(t_bidib_aspect), 3);
						last_scalar = BOARD_ACCESSORY_ID_VALUE;
						break;
					case BOARD_ACCESSORY_ID_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "number")) {
							last_scalar = BOARD_ACCESSORY_NUMBER_KEY;
						} else {
							error = true;
						}
						break;
					case BOARD_ACCESSORY_NUMBER_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &mapping.number)) {
							error = true;
							syslog_libbidib(LOG_ERR, "Number of board point/signal %s is in wrong format",
							                mapping.id->str);
						} else {
							last_scalar = BOARD_ACCESSORY_NUMBER_VALUE;
						}
						break;
					case BOARD_ACCESSORY_NUMBER_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "aspects")) {
							last_scalar = BOARD_ACCESSORY_ASPECTS_KEY;
						} else {
							error = true;
						}
						break;
					case BOARD_ACCESSORY_ASPECTS_KEY:
						error = true;
						break;
					case BOARD_ACCESSORY_ASPECTS_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "initial")) {
							last_scalar = BOARD_ACCESSORY_INITIAL_KEY;
						} else {
							error = true;
						}
						break;
					case BOARD_ACCESSORY_INITIAL_KEY:
						initial_value.id = g_string_new(accessory_state.id);
						initial_value.value = g_string_new((const gchar *) event.data.scalar.value);
						if (type == BOARD_SETUP_POINTS_BOARD_KEY) {
							bidib_state_add_initial_point_value(initial_value);
						} else {
							bidib_state_add_initial_signal_value(initial_value);
						}
						last_scalar = BOARD_ACCESSORY_INITIAL_VALUE;
						if (!initial_value_valid(mapping.aspects, initial_value.value->str, false)) {
							error = true;
							syslog_libbidib(LOG_ERR, 
							                "Initial value %s of board point/signal %s "
							                "is not defined in aspects",
							                initial_value.value->str, mapping.id->str);
						}
						break;
					case BOARD_ACCESSORY_INITIAL_VALUE:
					//	error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	t_bidib_board_accessory_mapping tmp;
	if (type == BOARD_SETUP_POINTS_BOARD_KEY) {
		for (size_t i = 0; i < board->points_board->len; i++) {
			tmp = g_array_index(board->points_board, t_bidib_board_accessory_mapping, i);
			if (tmp.number == mapping.number) {
				syslog_libbidib(LOG_ERR, "Point %s configured with same number "
				                "as point %s", mapping.id->str, tmp.id->str);
				error = true;
				break;
			}
		}
		g_array_append_val(board->points_board, mapping);
	} else {
		for (size_t i = 0; i < board->signals_board->len; i++) {
			tmp = g_array_index(board->signals_board, t_bidib_board_accessory_mapping, i);
			if (tmp.number == mapping.number) {
				syslog_libbidib(LOG_ERR, "Signal %s configured with same number "
				                "as signal %s", mapping.id->str, tmp.id->str);
				error = true;
				break;
			}
		}
		g_array_append_val(board->signals_board, mapping);
	}

	if (error) {
		if (accessory_state.id != NULL) {
			free(accessory_state.id);
		}
	} else {
		switch (type) {
			case BOARD_SETUP_POINTS_BOARD_KEY:
				if (bidib_state_add_board_point_state(accessory_state)) {
					syslog_libbidib(LOG_ERR, "Point %s configured with same id "
					                "as another point", mapping.id->str);
					bidib_state_free_single_board_accessory_state(accessory_state);
					error = true;
				}
				break;
			case BOARD_SETUP_SIGNALS_BOARD_KEY:
				if (bidib_state_add_board_signal_state(accessory_state)) {
					syslog_libbidib(LOG_ERR, "Signal %s configured with same id "
					                "as another signal", mapping.id->str);
					bidib_state_free_single_board_accessory_state(accessory_state);
					error = true;
				}
				break;
			default:
				error = true;
				break;
		}
	}
	return error;
}

typedef enum {
	DCC_ASPECT_PORT_START,
	DCC_ASPECT_PORT_PORT_KEY, DCC_ASPECT_PORT_PORT_VALUE,
	DCC_ASPECT_PORT_VALUE_KEY, DCC_ASPECT_PORT_VALUE_VALUE
} t_bidib_parser_dcc_aspect_port_scalar;

static bool bidib_config_parse_dcc_aspect_port(yaml_parser_t *parser, GArray *port_values) {
	yaml_event_t event;
	t_bidib_dcc_aspect_port_value port_value = {0x00, 0x00};
	bool error = false;
	bool done = false;
	t_bidib_parser_dcc_aspect_port_scalar last_scalar = DCC_ASPECT_PORT_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_END_EVENT:
				error = true;
				break;
			case YAML_MAPPING_START_EVENT:
				error = true;
				break;
			case YAML_MAPPING_END_EVENT:
				if (last_scalar == DCC_ASPECT_PORT_VALUE_VALUE) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				switch (last_scalar) {
					case DCC_ASPECT_PORT_START:
						if (!strcmp((char *) event.data.scalar.value, "port")) {
							last_scalar = DCC_ASPECT_PORT_PORT_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ASPECT_PORT_PORT_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &port_value.port) || port_value.port > 31) {
							error = true;
							syslog_libbidib(LOG_ERR, "Port is in wrong format");
						} else {
							last_scalar = DCC_ASPECT_PORT_PORT_VALUE;
						}
						break;
					case DCC_ASPECT_PORT_PORT_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "value")) {
							last_scalar = DCC_ASPECT_PORT_VALUE_KEY;
						} else {
							error = true;
							syslog_libbidib(LOG_ERR, "Value is in wrong format, must be 0 or 1");
						}
						break;
					case DCC_ASPECT_PORT_VALUE_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value, &port_value.value) ||
						    port_value.value > 1) {
							error = true;
						} else {
							last_scalar = DCC_ASPECT_PORT_VALUE_VALUE;
						}
						break;
					case DCC_ASPECT_PORT_VALUE_VALUE:
						error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	t_bidib_dcc_aspect_port_value tmp;
	for (size_t i = 0; i < port_values->len; i++) {
		tmp = g_array_index(port_values, t_bidib_dcc_aspect_port_value, i);
		if (tmp.port == port_value.port) {
			syslog_libbidib(LOG_ERR, "Port 0x%02x configured twice for a dcc aspect ",
			                port_value.port);
			error = true;
			break;
		}
	}
	g_array_append_val(port_values, port_value);
	return error;
}

static bool dcc_aspects_equal(t_bidib_dcc_aspect *aspect1, t_bidib_dcc_aspect *aspect2) {
	if (!strcmp(aspect1->id->str, aspect2->id->str)) {
		return true;
	}
	t_bidib_dcc_aspect_port_value *port_value1;
	t_bidib_dcc_aspect_port_value *port_value2;
	for (size_t i = 0; i < aspect1->port_values->len; i++) {
		port_value1 = &g_array_index(aspect1->port_values, t_bidib_dcc_aspect_port_value, i);
		bool seen = false;
		for (size_t j = 0; j < aspect2->port_values->len; j++) {
			port_value2 = &g_array_index(aspect2->port_values, t_bidib_dcc_aspect_port_value, j);
			if (port_value1->port == port_value2->port) {
				if (port_value1->value != port_value2->value) {
					return false;
				} else {
					seen = true;
					break;
				}
			}
		}
		if (!seen) {
			return false;
		}
	}
	return true;
}

typedef enum {
	DCC_ASPECT_START,
	DCC_ASPECT_ID_KEY, DCC_ASPECT_ID_VALUE,
	DCC_ASPECT_PORTS_KEY, DCC_ASPECT_PORTS_VALUE,
} t_bidib_parser_dcc_aspect_scalar;

static bool bidib_config_parse_dcc_aspect(yaml_parser_t *parser, GArray *aspect_list) {
	yaml_event_t event;
	t_bidib_dcc_aspect aspect = {NULL, NULL};
	bool error = false;
	bool done = false;
	bool in_seq = false;
	t_bidib_parser_dcc_aspect_scalar last_scalar = DCC_ASPECT_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				if (in_seq) {
					error = true;
				} else {
					in_seq = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (in_seq && last_scalar == DCC_ASPECT_PORTS_KEY) {
					last_scalar = DCC_ASPECT_PORTS_VALUE;
					in_seq = false;
					if (aspect.port_values->len == 0) {
						error = true;
						syslog_libbidib(LOG_ERR, "No port values configured for aspect %s",
						                aspect.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (in_seq && last_scalar == DCC_ASPECT_PORTS_KEY) {
					error = bidib_config_parse_dcc_aspect_port(parser, aspect.port_values);
					if (error) {
						syslog_libbidib(LOG_ERR, "Error while parsing a port value for aspect %s",
						                aspect.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_END_EVENT:
				if (!in_seq && last_scalar == DCC_ASPECT_PORTS_VALUE) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				switch (last_scalar) {
					case DCC_ASPECT_START:
						if (!strcmp((char *) event.data.scalar.value, "id")) {
							last_scalar = DCC_ASPECT_ID_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ASPECT_ID_KEY:
						aspect.id = g_string_new((const gchar *) event.data.scalar.value);
						aspect.port_values = g_array_sized_new(FALSE, FALSE,
						                                       sizeof(t_bidib_dcc_aspect_port_value), 3);
						last_scalar = DCC_ASPECT_ID_VALUE;
						break;
					case DCC_ASPECT_ID_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "ports")) {
							last_scalar = DCC_ASPECT_PORTS_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ASPECT_PORTS_KEY:
					case DCC_ASPECT_PORTS_VALUE:
						error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	t_bidib_dcc_aspect tmp;
	for (size_t i = 0; i < aspect_list->len; i++) {
		tmp = g_array_index(aspect_list, t_bidib_dcc_aspect, i);
		if (dcc_aspects_equal(&aspect, &tmp)) {
			syslog_libbidib(LOG_ERR, "Dcc aspect %s configured with same id or port "
			                "combination as aspect %s", aspect.id->str, tmp.id->str);
			error = true;
			break;
		}
	}
	g_array_append_val(aspect_list, aspect);
	return error;
}

typedef enum {
	DCC_ACCESSORY_START,
	DCC_ACCESSORY_ID_KEY, DCC_ACCESSORY_ID_VALUE,
	DCC_ACCESSORY_ADDR_KEY, DCC_ACCESSORY_ADDR_VALUE,
	DCC_ACCESSORY_EXTENDED_KEY, DCC_ACCESSORY_EXTENDED_VALUE,
	DCC_ACCESSORY_ASPECTS_KEY, DCC_ACCESSORY_ASPECTS_VALUE,
	DCC_ACCESSORY_INITIAL_KEY, DCC_ACCESSORY_INITIAL_VALUE
} t_bidib_parser_dcc_accessory_scalar;

static bool bidib_config_parse_single_dcc_accessory(yaml_parser_t *parser,
                                                    t_bidib_board *board,
                                                    t_bidib_parser_board_setup_scalar type) {
	yaml_event_t event;
	t_bidib_dcc_accessory_state accessory_state = {NULL, {NULL, 0x00, true, true, BIDIB_DCC_ACK_PENDING,
	                                                      BIDIB_TIMEUNIT_MILLISECONDS, 0x00}};
	t_bidib_dcc_accessory_mapping mapping = {NULL, {0x00, 0x00, 0x00}, 0x00, NULL};
	t_bidib_state_initial_value initial_value = {NULL, NULL};
	bool error = false;
	bool done = false;
	bool in_seq = false;
	t_bidib_parser_dcc_accessory_scalar last_scalar = DCC_ACCESSORY_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				if (in_seq) {
					error = true;
				} else {
					in_seq = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (in_seq && last_scalar == DCC_ACCESSORY_ASPECTS_KEY) {
					last_scalar = DCC_ACCESSORY_ASPECTS_VALUE;
					in_seq = false;
					if (mapping.aspects->len == 0) {
						error = true;
						syslog_libbidib(LOG_ERR, 
						                "No aspect configured for dcc point/signal %s",
						                mapping.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (in_seq && last_scalar == DCC_ACCESSORY_ASPECTS_KEY) {
					error = bidib_config_parse_dcc_aspect(parser, mapping.aspects);
					if (error) {
						syslog_libbidib(LOG_ERR, 
						                "Error while parsing a dcc aspect of dcc "
						                "point/signal %s", mapping.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_END_EVENT:
				if (!in_seq && (last_scalar == DCC_ACCESSORY_INITIAL_VALUE ||
				                last_scalar == DCC_ACCESSORY_ASPECTS_VALUE)) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				switch (last_scalar) {
					case DCC_ACCESSORY_START:
						if (!strcmp((char *) event.data.scalar.value, "id")) {
							last_scalar = DCC_ACCESSORY_ID_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ACCESSORY_ID_KEY:
						accessory_state.id = malloc(
								sizeof(char) * (strlen((char *) event.data.scalar.value) + 1));
						strcpy(accessory_state.id, (char *) event.data.scalar.value);
						mapping.id = g_string_new(accessory_state.id);
						mapping.aspects = g_array_sized_new(FALSE, FALSE, sizeof(t_bidib_dcc_aspect), 3);
						last_scalar = DCC_ACCESSORY_ID_VALUE;
						break;
					case DCC_ACCESSORY_ID_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "dcc-address")) {
							last_scalar = DCC_ACCESSORY_ADDR_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ACCESSORY_ADDR_KEY:
						if (bidib_string_to_dccaddr((char *) event.data.scalar.value,
						                            &mapping.dcc_addr)) {
							error = true;
							syslog_libbidib(LOG_ERR, 
							                "Dcc address of dcc point/signal %s is in wrong format",
							                mapping.id->str);
						} else {
							last_scalar = DCC_ACCESSORY_ADDR_VALUE;
						}
						break;
					case DCC_ACCESSORY_ADDR_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "extended")) {
							last_scalar = DCC_ACCESSORY_EXTENDED_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ACCESSORY_EXTENDED_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &mapping.extended_accessory) ||
						    mapping.extended_accessory > 1) {
							error = true;
							syslog_libbidib(LOG_ERR, 
							                "Extended bit of dcc point/signal %s must be 0 or 1",
							                mapping.id->str);
						} else {
							last_scalar = DCC_ACCESSORY_EXTENDED_VALUE;
						}
						break;
					case DCC_ACCESSORY_EXTENDED_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "aspects")) {
							last_scalar = DCC_ACCESSORY_ASPECTS_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ACCESSORY_ASPECTS_KEY:
						error = true;
						break;
					case DCC_ACCESSORY_ASPECTS_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "initial")) {
							last_scalar = DCC_ACCESSORY_INITIAL_KEY;
						} else {
							error = true;
						}
						break;
					case DCC_ACCESSORY_INITIAL_KEY:
						initial_value.id = g_string_new(accessory_state.id);
						initial_value.value = g_string_new((const gchar *) event.data.scalar.value);
						if (type == BOARD_SETUP_POINTS_DCC_KEY) {
							bidib_state_add_initial_point_value(initial_value);
						} else {
							bidib_state_add_initial_signal_value(initial_value);
						}
						last_scalar = DCC_ACCESSORY_INITIAL_VALUE;
						if (!initial_value_valid(mapping.aspects, initial_value.value->str, true)) {
							error = true;
							syslog_libbidib(LOG_ERR, 
							                "Initial value %s of dcc point/signal %s "
							                "is not defined in aspects",
							                initial_value.value->str, mapping.id->str);
						}
						break;
					case DCC_ACCESSORY_INITIAL_VALUE:
						error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	if (error) {
		if (accessory_state.id != NULL) {
			free(accessory_state.id);
		}
	} else {
		switch (type) {
			case BOARD_SETUP_POINTS_DCC_KEY:
				if (bidib_state_add_dcc_point_state(accessory_state, mapping.dcc_addr)) {
					syslog_libbidib(LOG_ERR, "Point %s configured with same id or dcc address "
					                "as another point", mapping.id->str);
					bidib_state_free_single_dcc_accessory_state(accessory_state);
					error = true;
				}
				break;
			case BOARD_SETUP_SIGNALS_DCC_KEY:
				if (bidib_state_add_dcc_signal_state(accessory_state, mapping.dcc_addr)) {
					syslog_libbidib(LOG_ERR, "Signal %s configured with same id or dcc address "
					                "as another signal", mapping.id->str);
					bidib_state_free_single_dcc_accessory_state(accessory_state);
					error = true;
				}
				break;
			default:
				error = true;
				break;
		}
	}

	if (type == BOARD_SETUP_POINTS_DCC_KEY) {
		g_array_append_val(board->points_dcc, mapping);
	} else {
		g_array_append_val(board->signals_dcc, mapping);
	}

	return error;
}

typedef enum {
	PERIPHERAL_START,
	PERIPHERAL_ID_KEY, PERIPHERAL_ID_VALUE,
	PERIPHERAL_NUMBER_KEY, PERIPHERAL_NUMBER_VALUE,
	PERIPHERAL_PORT_KEY, PERIPHERAL_PORT_VALUE,
	PERIPHERAL_ASPECTS_KEY, PERIPHERAL_ASPECTS_VALUE,
	PERIPHERAL_INITIAL_KEY, PERIPHERAL_INITIAL_VALUE
} t_bidib_parser_peripheral_scalar;

static bool bidib_config_parse_single_board_peripheral(yaml_parser_t *parser,
                                                       t_bidib_board *board) {
	yaml_event_t event;
	t_bidib_peripheral_state peripheral_state = {NULL, {NULL, 0x00,
	                                                    BIDIB_TIMEUNIT_MILLISECONDS, 0x00}};
	t_bidib_peripheral_mapping mapping = {NULL, 0x00, {0x00, 0x00}, NULL};
	t_bidib_state_initial_value initial_value = {NULL, NULL};
	bool error = false;
	bool done = false;
	bool in_seq = false;
	t_bidib_parser_peripheral_scalar last_scalar = PERIPHERAL_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				if (in_seq) {
					error = true;
				} else {
					in_seq = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (in_seq && last_scalar == PERIPHERAL_ASPECTS_KEY) {
					last_scalar = PERIPHERAL_ASPECTS_VALUE;
					in_seq = false;
					if (mapping.aspects->len == 0) {
						error = true;
						syslog_libbidib(LOG_ERR, "No aspect configured for peripheral %s",
						                mapping.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (in_seq && last_scalar == PERIPHERAL_ASPECTS_KEY) {
					error = bidib_config_parse_aspect(parser, mapping.aspects);
					if (error) {
						syslog_libbidib(LOG_ERR, "Error while parsing an aspect of peripheral %s",
						                mapping.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_END_EVENT:
				if (!in_seq && (last_scalar == PERIPHERAL_INITIAL_VALUE ||
				                last_scalar == PERIPHERAL_ASPECTS_VALUE)) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				switch (last_scalar) {
					case PERIPHERAL_START:
						if (!strcmp((char *) event.data.scalar.value, "id")) {
							last_scalar = PERIPHERAL_ID_KEY;
						} else {
							error = true;
						}
						break;
					case PERIPHERAL_ID_KEY:
						peripheral_state.id = malloc(
								sizeof(char) * (strlen((char *) event.data.scalar.value) + 1));
						strcpy(peripheral_state.id, (char *) event.data.scalar.value);
						mapping.id = g_string_new(peripheral_state.id);
						mapping.aspects = g_array_sized_new(FALSE, FALSE, sizeof(t_bidib_aspect), 3);
						last_scalar = PERIPHERAL_ID_VALUE;
						break;
					case PERIPHERAL_ID_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "number")) {
							last_scalar = PERIPHERAL_NUMBER_KEY;
						} else {
							error = true;
						}
						break;
					case PERIPHERAL_NUMBER_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &mapping.number)) {
							error = true;
							syslog_libbidib(LOG_ERR, "Number of peripheral %s is in wrong format",
							                mapping.id->str);
						} else {
							last_scalar = PERIPHERAL_NUMBER_VALUE;
						}
						break;
					case PERIPHERAL_NUMBER_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "port")) {
							last_scalar = PERIPHERAL_PORT_KEY;
						} else {
							error = true;
						}
						break;
					case PERIPHERAL_PORT_KEY:
						if (bidib_string_to_port((char *) event.data.scalar.value,
						                         &mapping.port)) {
							error = true;
							syslog_libbidib(LOG_ERR, "Port of peripheral %s is in wrong format", 
							                mapping.id->str);
						} else {
							last_scalar = PERIPHERAL_PORT_VALUE;
						}
						break;
					case PERIPHERAL_PORT_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "aspects")) {
							last_scalar = PERIPHERAL_ASPECTS_KEY;
						} else {
							error = true;
						}
						break;
					case PERIPHERAL_ASPECTS_KEY:
						error = true;
						break;
					case PERIPHERAL_ASPECTS_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "initial")) {
							last_scalar = PERIPHERAL_INITIAL_KEY;
						} else {
							error = true;
						}
						break;
					case PERIPHERAL_INITIAL_KEY:
						initial_value.id = g_string_new(peripheral_state.id);
						initial_value.value = g_string_new((const gchar *) event.data.scalar.value);
						bidib_state_add_initial_peripheral_value(initial_value);
						last_scalar = PERIPHERAL_INITIAL_VALUE;
						if (!initial_value_valid(mapping.aspects, initial_value.value->str, false)) {
							error = true;
							syslog_libbidib(LOG_ERR, 
							                "Initial value %s of peripheral %s is not defined in aspects",
							                initial_value.value->str, mapping.id->str);
						}
						break;
					case PERIPHERAL_INITIAL_VALUE:
						error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	t_bidib_peripheral_mapping tmp;
	for (size_t i = 0; i < board->peripherals->len; i++) {
		tmp = g_array_index(board->peripherals, t_bidib_peripheral_mapping, i);
		if (tmp.number == mapping.number) {
			syslog_libbidib(LOG_ERR, "Peripheral %s configured with same number "
							"as peripheral %s", mapping.id->str, tmp.id->str);
			error = true;
			break;
		}
	}

	for (size_t i = 0; i < board->peripherals->len; i++) {
		tmp = g_array_index(board->peripherals, t_bidib_peripheral_mapping, i);
		if (tmp.port.port0 == mapping.port.port0 &&
		    tmp.port.port1 == mapping.port.port1) {
			syslog_libbidib(LOG_ERR, "Peripheral %s configured with same port "
			                "as peripheral %s", mapping.id->str, tmp.id->str);
			error = true;
			break;
		}
	}
	g_array_append_val(board->peripherals, mapping);

	if (error) {
		if (peripheral_state.id != NULL) {
			free(peripheral_state.id);
		}
	} else {
		if (bidib_state_add_peripheral_state(peripheral_state)) {
			syslog_libbidib(LOG_ERR, "Peripheral %s configured with same id "
			                "as another peripheral", mapping.id->str);
			bidib_state_free_single_peripheral_state(peripheral_state);
			error = true;
		}
	}
	return error;
}

typedef enum {
	SEGMENT_START,
	SEGMENT_ID_KEY, SEGMENT_ID_VALUE,
	SEGMENT_ADDR_KEY, SEGMENT_ADDR_VALUE
} t_bidib_parser_segment_scalar;

static bool bidib_config_parse_single_board_segment(yaml_parser_t *parser,
                                                    t_bidib_board *board) {
	yaml_event_t event;
	t_bidib_segment_state_intern segment_state;
	segment_state.id = NULL;
	segment_state.dcc_addresses = NULL;
	t_bidib_segment_mapping mapping = {NULL, 0x00};
	bool error = false;
	bool done = false;
	t_bidib_parser_segment_scalar last_scalar = SEGMENT_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_END_EVENT:
				error = true;
				break;
			case YAML_MAPPING_START_EVENT:
				error = true;
				break;
			case YAML_MAPPING_END_EVENT:
				if (last_scalar == SEGMENT_ADDR_VALUE) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				switch (last_scalar) {
					case SEGMENT_START:
						if (!strcmp((char *) event.data.scalar.value, "id")) {
							last_scalar = SEGMENT_ID_KEY;
						} else {
							error = true;
						}
						break;
					case SEGMENT_ID_KEY:
						segment_state.id = g_string_new((char *) event.data.scalar.value);
						mapping.id = g_string_new(segment_state.id->str);
						segment_state.occupied = false;
						segment_state.confidence.conf_void = false;
						segment_state.confidence.freeze = false;
						segment_state.confidence.nosignal = false;
						segment_state.power_consumption.known = false;
						segment_state.power_consumption.overcurrent = false;
						segment_state.power_consumption.current = 0;
						segment_state.dcc_addresses = g_array_sized_new(
								FALSE, FALSE, sizeof(t_bidib_dcc_address), 4);
						last_scalar = SEGMENT_ID_VALUE;
						break;
					case SEGMENT_ID_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "address")) {
							last_scalar = SEGMENT_ADDR_KEY;
						} else {
							error = true;
						}
						break;
					case SEGMENT_ADDR_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &mapping.addr)) {
							error = true;
						} else {
							last_scalar = SEGMENT_ADDR_VALUE;
						}
						break;
					case SEGMENT_ADDR_VALUE:
					//	error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	t_bidib_segment_mapping tmp;
	for (size_t i = 0; i < board->segments->len; i++) {
		tmp = g_array_index(board->segments, t_bidib_segment_mapping, i);
		if (tmp.addr == mapping.addr) {
			syslog_libbidib(LOG_ERR, "Segment %s configured with same address "
			                "as segment %s", mapping.id->str, tmp.id->str);
			error = true;
			break;
		}
	}
	g_array_append_val(board->segments, mapping);

	if (error) {
		bidib_state_free_single_segment_state_intern(segment_state);
	} else {
		if (bidib_state_add_segment_state(segment_state)) {
			syslog_libbidib(LOG_ERR, "Segment %s configured with same id "
			                "as another segment", mapping.id->str);
			bidib_state_free_single_segment_state_intern(segment_state);
			error = true;
		}
	}
	return error;
}

static bool bidib_config_parse_single_board_setup(yaml_parser_t *parser) {
	t_bidib_board *board = NULL;
	yaml_event_t event;
	bool error = false;
	bool done = false;
	bool in_seq = false;
	t_bidib_parser_board_setup_scalar last_scalar = BOARD_SETUP_START;

	while (!error && !done) {
		if (!yaml_parser_parse(parser, &event)) {
			error = true;
			break;
		}

		switch (event.type) {
			case YAML_NO_EVENT:
				error = true;
				break;
			case YAML_STREAM_START_EVENT:
				error = true;
				break;
			case YAML_STREAM_END_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_START_EVENT:
				error = true;
				break;
			case YAML_DOCUMENT_END_EVENT:
				error = true;
				break;
			case YAML_SEQUENCE_START_EVENT:
				if (in_seq) {
					error = true;
				} else {
					in_seq = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (!in_seq) {
					error = true;
				} else {
					switch (last_scalar) {
						case BOARD_SETUP_POINTS_BOARD_KEY:
							last_scalar = BOARD_SETUP_POINTS_BOARD_VALUE;
							in_seq = false;
							break;
						case BOARD_SETUP_POINTS_DCC_KEY:
							last_scalar = BOARD_SETUP_POINTS_DCC_VALUE;
							in_seq = false;
							break;
						case BOARD_SETUP_SIGNALS_BOARD_KEY:
							last_scalar = BOARD_SETUP_SIGNALS_BOARD_VALUE;
							in_seq = false;
							break;
						case BOARD_SETUP_SIGNALS_DCC_KEY:
							last_scalar = BOARD_SETUP_SIGNALS_DCC_VALUE;
							in_seq = false;
							break;
						case BOARD_SETUP_PERIPHERALS_KEY:
							last_scalar = BOARD_SETUP_PERIPHERALS_VALUE;
							in_seq = false;
							break;
						case BOARD_SETUP_SEGMENTS_KEY:
							last_scalar = BOARD_SETUP_SEGMENTS_VALUE;
							in_seq = false;
							break;
						default:
							error = true;
					}
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (last_scalar != BOARD_SETUP_START && !in_seq) {
					error = true;
				} else {
					switch (last_scalar) {
						case BOARD_SETUP_POINTS_BOARD_KEY:
							error = bidib_config_parse_single_board_accessory(
									parser, board, last_scalar);
							if (error) {
								syslog_libbidib(LOG_ERR, "Error while parsing a board point of board %s",
								                board->id->str);
							}
							break;
						case BOARD_SETUP_POINTS_DCC_KEY:
							error = bidib_config_parse_single_dcc_accessory(
									parser, board, last_scalar);
							if (error) {
								syslog_libbidib(LOG_ERR, "Error while parsing a dcc point of board %s",
								                board->id->str);
							}
							break;
						case BOARD_SETUP_SIGNALS_BOARD_KEY:
							error = bidib_config_parse_single_board_accessory(
									parser, board, last_scalar);
							if (error) {
								syslog_libbidib(LOG_ERR, "Error while parsing a board signal of board %s",
								                board->id->str);
							}
							break;
						case BOARD_SETUP_SIGNALS_DCC_KEY:
							error = bidib_config_parse_single_dcc_accessory(
									parser, board, last_scalar);
							if (error) {
								syslog_libbidib(LOG_ERR, "Error while parsing a dcc signal of board %s",
								                board->id->str);
							}
							break;
						case BOARD_SETUP_PERIPHERALS_KEY:
							error = bidib_config_parse_single_board_peripheral(
									parser, board);
							if (error) {
								syslog_libbidib(LOG_ERR, "Error while parsing a peripheral of board %s",
								                board->id->str);
							}
							break;
						case BOARD_SETUP_SEGMENTS_KEY:
							error = bidib_config_parse_single_board_segment(
									parser, board);
							if (error) {
								syslog_libbidib(LOG_ERR, "Error while parsing a segment of board %s",
								                board->id->str);
							}
							break;
						default:
							error = true;
					}
				}
				break;
			case YAML_MAPPING_END_EVENT:
				done = true;
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				if (in_seq) {
					error = true;
				} else {
					switch (last_scalar) {
						case BOARD_SETUP_START:
							if (!strcmp((char *) event.data.scalar.value, "id")) {
								last_scalar = BOARD_SETUP_ID_KEY;
							} else {
								error = true;
							}
							break;
						case BOARD_SETUP_ID_KEY:
							board = bidib_state_get_board_ref((char *) event.data.scalar.value);
							if (board == NULL) {
								syslog_libbidib(LOG_ERR, "Board %s in track config, "
								                "but not in board config", board->id->str);
								error = true;
							} else {
								last_scalar = BOARD_SETUP_ID_VALUE;
							}
							break;
						case BOARD_SETUP_ID_VALUE:
							if (!strcmp((char *) event.data.scalar.value,
							            "points-board")) {
								last_scalar = BOARD_SETUP_POINTS_BOARD_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "points-dcc")) {
								last_scalar = BOARD_SETUP_POINTS_DCC_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "signals-board")) {
								last_scalar = BOARD_SETUP_SIGNALS_BOARD_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "signals-dcc")) {
								last_scalar = BOARD_SETUP_SIGNALS_DCC_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "peripherals")) {
								last_scalar = BOARD_SETUP_PERIPHERALS_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "segments")) {
								last_scalar = BOARD_SETUP_SEGMENTS_KEY;
							} else {
								error = true;
								break;
							}
							break;
						case BOARD_SETUP_POINTS_BOARD_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "points-dcc")) {
								last_scalar = BOARD_SETUP_POINTS_DCC_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "signals-board")) {
								last_scalar = BOARD_SETUP_SIGNALS_BOARD_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "signals-dcc")) {
								last_scalar = BOARD_SETUP_SIGNALS_DCC_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "peripherals")) {
								last_scalar = BOARD_SETUP_PERIPHERALS_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "segments")) {
								last_scalar = BOARD_SETUP_SEGMENTS_KEY;
							} else {
								error = true;
								break;
							}
							break;
						case BOARD_SETUP_POINTS_DCC_VALUE:
							if (!strcmp((char *) event.data.scalar.value,
							            "signals-board")) {
								last_scalar = BOARD_SETUP_SIGNALS_BOARD_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "signals-dcc")) {
								last_scalar = BOARD_SETUP_SIGNALS_DCC_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "peripherals")) {
								last_scalar = BOARD_SETUP_PERIPHERALS_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "segments")) {
								last_scalar = BOARD_SETUP_SEGMENTS_KEY;
							} else {
								error = true;
								break;
							}
							break;
						case BOARD_SETUP_SIGNALS_BOARD_VALUE:
							if (!strcmp((char *) event.data.scalar.value,
							            "signals-dcc")) {
								last_scalar = BOARD_SETUP_SIGNALS_DCC_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "peripherals")) {
								last_scalar = BOARD_SETUP_PERIPHERALS_KEY;
							} else if (!strcmp((char *) event.data.scalar.value,
							                   "segments")) {
								last_scalar = BOARD_SETUP_SEGMENTS_KEY;
							} else {
								error = true;
								break;
							}
							break;
						case BOARD_SETUP_SIGNALS_DCC_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "peripherals")) {
								last_scalar = BOARD_SETUP_PERIPHERALS_KEY;
							} else if (!strcmp((char *) event.data.scalar.value, "segments")) {
								last_scalar = BOARD_SETUP_SEGMENTS_KEY;
							} else {
								error = true;
								break;
							}
							break;
						case BOARD_SETUP_PERIPHERALS_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "segments")) {
								last_scalar = BOARD_SETUP_SEGMENTS_KEY;
							} else {
								error = true;
								break;
							}
							break;
						case BOARD_SETUP_SEGMENTS_VALUE:
							error = true;
							break;
						default:
							error = true;
							break;
					}
				}
				break;
		}
		yaml_event_delete(&event);
	}
	return error;
}

int bidib_config_parse_track_config(const char *config_dir) {
	FILE *fh;
	yaml_parser_t parser;
	if (bidib_config_init_parser(config_dir, "/bidib_track_config.yml", &fh,
	                             &parser)) {
		return true;
	}

	bool error = bidib_config_parse_scalar_then_section(
			&parser, "boards", bidib_config_parse_single_board_setup);

	if (error) {
		syslog_libbidib(LOG_ERR, "%s", "Error while parsing track config");
	}

	yaml_parser_delete(&parser);
	fclose(fh);
	return error;
}
