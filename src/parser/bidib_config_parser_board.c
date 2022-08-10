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

#include "bidib_config_parser_intern.h"
#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../state/bidib_state_intern.h"


typedef enum {
	BOARD_CONFIG_START,
	BOARD_CONFIG_ID_KEY, BOARD_CONFIG_ID_VALUE,
	BOARD_CONFIG_UID_KEY, BOARD_CONFIG_UID_VALUE,
	BOARD_CONFIG_FEATURES_KEY,
	BOARD_CONFIG_NUMBER_KEY, BOARD_CONFIG_NUMBER_VALUE,
	BOARD_CONFIG_VALUE_KEY, BOARD_CONFIG_VALUE_VALUE
} t_bidib_parser_board_config_scalar;

static bool bidib_config_parse_single_board_features(yaml_parser_t *parser) {
	t_bidib_board board = {NULL, {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}, false,
	                       {0x00, 0x00, 0x00}, false, NULL, NULL, NULL, NULL, NULL,
	                       NULL, NULL};
	t_bidib_board_feature feature;
	yaml_event_t event;
	bool error = false;
	bool done = false;
	bool seq_read = false;
	bool feature_ready = true;
	t_bidib_parser_board_config_scalar last_scalar = BOARD_CONFIG_START;

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
				if (last_scalar != BOARD_CONFIG_FEATURES_KEY || seq_read) {
					error = true;
				} else {
					seq_read = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (!seq_read) {
					error = true;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (!seq_read) {
					error = true;
				}
				break;
			case YAML_MAPPING_END_EVENT:
				if (seq_read) {
					if (feature_ready) {
						done = true;
					} else if (last_scalar == BOARD_CONFIG_VALUE_VALUE) {
						board.features = g_array_append_val(board.features, feature);
						if (feature.number == 0x03 && feature.value > 0) {
							board.secack_on = true;
							syslog_libbidib(LOG_INFO, "SecAck on for board %s", board.id->str);
						}
						feature_ready = true;
					} else {
						error = true;
					}
				} else {
					if (last_scalar < BOARD_CONFIG_UID_VALUE) {
						syslog_libbidib(LOG_ERR, "Each board configuration needs at least an "
						                "id and an unique id");
						error = true;
					} else {
						done = true;
					}
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				if (seq_read) {
					switch (last_scalar) {
						case BOARD_CONFIG_NUMBER_KEY:
							if (bidib_string_to_byte((char *) event.data.scalar.value,
							                         &feature.number)) {
								error = true;
							} else {
								t_bidib_board_feature tmp;
								for (size_t i = 0; i < board.features->len; i++) {
									tmp = g_array_index(board.features, t_bidib_board_feature, i);
									if (tmp.number == feature.number) {
										syslog_libbidib(LOG_ERR, "Two board features with same number "
										                "configured for board %s", board.id->str);
										error = true;
									}
								}
								last_scalar = BOARD_CONFIG_NUMBER_VALUE;
							}
							break;
						case BOARD_CONFIG_NUMBER_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "value")) {
								last_scalar = BOARD_CONFIG_VALUE_KEY;
							} else {
								error = true;
							}
							break;
						case BOARD_CONFIG_VALUE_KEY:
							if (bidib_string_to_byte((char *) event.data.scalar.value,
							                         &feature.value)) {
								error = true;
							} else {
								last_scalar = BOARD_CONFIG_VALUE_VALUE;
							}
							break;
						case BOARD_CONFIG_VALUE_VALUE:
						case BOARD_CONFIG_FEATURES_KEY:
							if (!strcmp((char *) event.data.scalar.value, "number")
							    && feature_ready) {
								feature_ready = false;
								last_scalar = BOARD_CONFIG_NUMBER_KEY;
							} else {
								error = true;
							}
							break;
						default:
							error = true;
							break;
					}
				} else {
					switch (last_scalar) {
						case BOARD_CONFIG_START:
							if (!strcmp((char *) event.data.scalar.value, "id")) {
								last_scalar = BOARD_CONFIG_ID_KEY;
							} else {
								error = true;
							}
							break;
						case BOARD_CONFIG_ID_KEY:
							board.id = g_string_new((char *) event.data.scalar.value);
							board.connected = false;
							board.node_addr.top = 0x00;
							board.node_addr.sub = 0x00;
							board.node_addr.subsub = 0x00;
							board.secack_on = false;
							board.features = g_array_sized_new(
									FALSE, FALSE, sizeof(t_bidib_board_feature), 8);
							board.points_board = g_array_sized_new(
									FALSE, FALSE, sizeof(t_bidib_board_accessory_mapping), 8);
							board.points_dcc = g_array_sized_new(
									FALSE, FALSE, sizeof(t_bidib_dcc_accessory_mapping), 8);
							board.signals_board = g_array_sized_new(
									FALSE, FALSE, sizeof(t_bidib_board_accessory_mapping), 8);
							board.signals_dcc = g_array_sized_new(
									FALSE, FALSE, sizeof(t_bidib_dcc_accessory_mapping), 8);
							board.peripherals = g_array_sized_new(FALSE, FALSE, sizeof(
									t_bidib_peripheral_mapping), 8);
							board.segments = g_array_sized_new(
									FALSE, FALSE, sizeof(t_bidib_segment_mapping), 16);
							last_scalar = BOARD_CONFIG_ID_VALUE;
							break;
						case BOARD_CONFIG_ID_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "unique-id")) {
								last_scalar = BOARD_CONFIG_UID_KEY;
							} else {
								error = true;
							}
							break;
						case BOARD_CONFIG_UID_KEY:
							if (bidib_string_to_uid((char *) event.data.scalar.value, &board.unique_id)) {
								error = true;
								syslog_libbidib(LOG_ERR, "Unique id of board %s is in wrong format", 
								                board.id->str);
							} else {
								if (board.unique_id.class_id & (1 << 1)) {
									// board has booster functionality
									t_bidib_booster_state booster_state;
									booster_state.id = malloc(sizeof(char) * (board.id->len + 1));
									strcpy(booster_state.id, board.id->str);
									booster_state.data.power_state = BIDIB_BSTR_OFF;
									booster_state.data.power_state_simple = bidib_booster_normal_to_simple(
											booster_state.data.power_state);
									booster_state.data.power_consumption.known = false;
									booster_state.data.voltage_known = false;
									booster_state.data.temp_known = false;
									bidib_state_add_booster(booster_state);
								}
								if (board.unique_id.class_id & (1 << 4)) {
									// board has dcc functionality
									t_bidib_track_output_state track_output_state;
									track_output_state.id = malloc(sizeof(char) * (board.id->len + 1));
									strcpy(track_output_state.id, board.id->str);
									track_output_state.cs_state = BIDIB_CS_OFF;
									bidib_state_add_track_output(track_output_state);
								}
								last_scalar = BOARD_CONFIG_UID_VALUE;
							}
							break;
						case BOARD_CONFIG_UID_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "features")) {
								last_scalar = BOARD_CONFIG_FEATURES_KEY;
							} else {
								error = true;
							}
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

	if (error) {
		bidib_state_free_single_board(board);
	} else {
		if ((error = bidib_state_add_board(board))) {
			syslog_libbidib(LOG_ERR, "Board %s configured with same id or unique id as another board",
			                board.id->str);
			bidib_state_free_single_board(board);
		}
	}
	return error;
}

int bidib_config_parse_board_config(const char *config_dir) {
	FILE *fh;
	yaml_parser_t parser;
	if (bidib_config_init_parser(config_dir, "/bidib_board_config.yml", &fh,
	                             &parser)) {
		return true;
	}

	bool error = bidib_config_parse_scalar_then_section(&parser, "boards",
	                                                    bidib_config_parse_single_board_features);

	if (error) {
		syslog_libbidib(LOG_ERR, "%s", "Error while parsing board config");
	}

	yaml_parser_delete(&parser);
	fclose(fh);
	return error;
}
