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
#include <stdint.h>

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../state/bidib_state_intern.h"
#include "bidib_config_parser_intern.h"


static bool bidib_config_parse_single_train_calibration(yaml_parser_t *parser,
                                                        t_bidib_train *train) {
	yaml_event_t event;
	bool error = false;
	bool done = false;
	train->calibration = g_array_sized_new(FALSE, FALSE, sizeof(int), 9);
	size_t counter = 0;
	uint8_t value;

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
				error = true;
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				if (bidib_string_to_byte((char *) event.data.scalar.value, &value) ||
				    value > 126) {
					error = true;
					syslog_libbidib(LOG_ERR, "Calibration values must be smaller than 127");
				} else {
					int tmp = value;
					g_array_append_val(train->calibration, tmp);
					counter++;
					if (counter == 9) {
						done = true;
					}
				}
				break;
		}
		yaml_event_delete(&event);
	}

	if (error) {
		g_array_free(train->calibration, TRUE);
		train->calibration = NULL;
	}
	return error;
}

typedef enum {
	TRAIN_PERIPHERAL_START,
	TRAIN_PERIPHERAL_ID_KEY, TRAIN_PERIPHERAL_ID_VALUE,
	TRAIN_PERIPHERAL_BIT_KEY, TRAIN_PERIPHERAL_BIT_VALUE,
	TRAIN_PERIPHERAL_INITIAL_KEY, TRAIN_PERIPHERAL_INITIAL_VALUE
} t_bidib_parser_train_peripheral_scalar;

static bool bidib_config_parse_single_train_peripheral(yaml_parser_t *parser,
                                                       t_bidib_train *train,
                                                       t_bidib_train_state_intern *train_state) {
	yaml_event_t event;
	t_bidib_train_peripheral_state peripheral_state;
	peripheral_state.id = NULL;
	peripheral_state.state = 0x00;
	t_bidib_train_peripheral_mapping mapping;
	mapping.id = NULL;
	t_bidib_state_train_initial_value initial_value = {NULL, NULL, 0x00};
	bool error = false;
	bool done = false;
	t_bidib_parser_train_peripheral_scalar last_scalar = TRAIN_PERIPHERAL_START;

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
				if (last_scalar == TRAIN_PERIPHERAL_BIT_VALUE ||
				    last_scalar == TRAIN_PERIPHERAL_INITIAL_VALUE) {
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
					case TRAIN_PERIPHERAL_START:
						if (!strcmp((char *) event.data.scalar.value, "id")) {
							last_scalar = TRAIN_PERIPHERAL_ID_KEY;
						} else {
							error = true;
						}
						break;
					case TRAIN_PERIPHERAL_ID_KEY:
						peripheral_state.id = strdup((char *) event.data.scalar.value);
						last_scalar = TRAIN_PERIPHERAL_ID_VALUE;
						break;
					case TRAIN_PERIPHERAL_ID_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "bit")) {
							last_scalar = TRAIN_PERIPHERAL_BIT_KEY;
						} else {
							error = true;
						}
						break;
					case TRAIN_PERIPHERAL_BIT_KEY:
						mapping.id = g_string_new(peripheral_state.id);
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &mapping.bit) || mapping.bit > 31) {
							error = true;
							syslog_libbidib(LOG_ERR, "Bit of peripheral %s must be smaller than 31",
							                mapping.id->str);
						} else {
							t_bidib_train_peripheral_mapping tmp;
							for (size_t i = 0; i < train->peripherals->len; i++) {
								tmp = g_array_index(train->peripherals,
								                    t_bidib_train_peripheral_mapping, i);
								if (tmp.bit == mapping.bit || !strcmp(tmp.id->str, mapping.id->str)) {
									syslog_libbidib(LOG_ERR, 
									                "Two train peripherals with same bit or same "
									                "id configured for train %s", train->id->str);
									error = true;
								}
							}
							last_scalar = TRAIN_PERIPHERAL_BIT_VALUE;
						}
						break;
					case TRAIN_PERIPHERAL_BIT_VALUE:
						if (!strcmp((char *) event.data.scalar.value, "initial")) {
							last_scalar = TRAIN_PERIPHERAL_INITIAL_KEY;
						} else {
							error = true;
						}
						break;
					case TRAIN_PERIPHERAL_INITIAL_KEY:
						if (bidib_string_to_byte((char *) event.data.scalar.value,
						                         &initial_value.value) ||
						    initial_value.value > 1) {
							error = true;
							syslog_libbidib(LOG_ERR, 
							                "Initial value of peripheral %s must be 0 or 1",
							                mapping.id->str);
						} else {
							initial_value.train = g_string_new(train_state->id->str);
							initial_value.id = g_string_new(peripheral_state.id);
							bidib_state_add_initial_train_value(initial_value);
							last_scalar = TRAIN_PERIPHERAL_INITIAL_VALUE;
						}
						break;
					case TRAIN_PERIPHERAL_INITIAL_VALUE:
						error = true;
						break;
				}
				break;
		}
		yaml_event_delete(&event);
	}

	g_array_append_val(train->peripherals, mapping);

	if (error) {
		if (peripheral_state.id != NULL) {
			free(peripheral_state.id);
		}
	} else {
		t_bidib_train_peripheral_state state_i;
		for (size_t i = 0; i < train_state->peripherals->len; i++) {
			state_i = g_array_index(train_state->peripherals,
			                        t_bidib_train_peripheral_state, i);
			if (!strcmp(state_i.id, peripheral_state.id)) {
				free(peripheral_state.id);
				error = true;
			}
		}
		if (!error) {
			g_array_append_val(train_state->peripherals, peripheral_state);
		}
	}
	return error;
}

typedef enum {
	TRAIN_START,
	TRAIN_ID_KEY, TRAIN_ID_VALUE,
	TRAIN_DCC_ADDR_KEY, TRAIN_DCC_ADDR_VALUE,
	TRAIN_DCC_STEPS_KEY, TRAIN_DCC_STEPS_VALUE,
	TRAIN_CALIBRATION_KEY, TRAIN_CALIBRATION_VALUE,
	TRAIN_PERIPHERALS_KEY, TRAIN_PERIPHERALS_VALUE
} t_bidib_parser_train_config_scalar;

static bool bidib_config_parse_single_train(yaml_parser_t *parser) {
	t_bidib_train train;
	train.id = NULL;
	train.calibration = NULL;
	train.peripherals = NULL;
	t_bidib_train_state_intern train_state;
	train_state.id = NULL;
	train_state.peripherals = NULL;
	yaml_event_t event;
	bool error = false;
	bool done = false;
	bool in_seq = false;
	t_bidib_parser_train_config_scalar last_scalar = TRAIN_START;

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
				if (!in_seq && (last_scalar == TRAIN_CALIBRATION_KEY ||
				                last_scalar == TRAIN_PERIPHERALS_KEY)) {
					if (last_scalar == TRAIN_CALIBRATION_KEY) {
						error = bidib_config_parse_single_train_calibration(parser, &train);
						if (error) {
							syslog_libbidib(LOG_ERR, "Error while parsing the calibration values "
							                "for train %s", train.id->str);
						}
					}
					in_seq = true;
				} else {
					error = true;
				}
				break;
			case YAML_SEQUENCE_END_EVENT:
				if (in_seq && last_scalar == TRAIN_CALIBRATION_KEY) {
					last_scalar = TRAIN_CALIBRATION_VALUE;
					in_seq = false;
				} else if (in_seq && last_scalar == TRAIN_PERIPHERALS_KEY) {
					last_scalar = TRAIN_PERIPHERALS_VALUE;
					in_seq = false;
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_START_EVENT:
				if (in_seq && last_scalar == TRAIN_PERIPHERALS_KEY) {
					error = bidib_config_parse_single_train_peripheral(parser, &train,
					                                                   &train_state);
					if (error) {
						syslog_libbidib(LOG_ERR, "Error while parsing a peripheral of train %s",
						                train.id->str);
					}
				} else {
					error = true;
				}
				break;
			case YAML_MAPPING_END_EVENT:
				if (last_scalar == TRAIN_DCC_STEPS_VALUE ||
				    last_scalar == TRAIN_PERIPHERALS_VALUE) {
					done = true;
				} else {
					error = true;
				}
				break;
			case YAML_ALIAS_EVENT:
				error = true;
				break;
			case YAML_SCALAR_EVENT:
				if (in_seq) {
					error = true;
				} else {
					switch (last_scalar) {
						case TRAIN_START:
							if (!strcmp((char *) event.data.scalar.value, "id")) {
								last_scalar = TRAIN_ID_KEY;
							} else {
								error = true;
							}
							break;
						case TRAIN_ID_KEY:
							train.id = g_string_new((char *) event.data.scalar.value);
							train.peripherals = g_array_sized_new(FALSE, FALSE, sizeof(
									t_bidib_train_peripheral_mapping), 4);
							train_state.id = g_string_new((char *) event.data.scalar.value);
							train_state.on_track = false;
							train_state.orientation = BIDIB_TRAIN_ORIENTATION_LEFT;
							train_state.set_speed_step = 0;
							train_state.set_is_forwards = true;
							train_state.ack = BIDIB_DCC_ACK_PENDING;
							train_state.detected_kmh_speed = 0;
							train_state.peripherals = g_array_sized_new(
									FALSE, FALSE, sizeof(t_bidib_train_peripheral_state), 5);
							train_state.decoder_state.signal_quality_known = false;
							train_state.decoder_state.temp_known = false;
							train_state.decoder_state.energy_storage_known = false;
							train_state.decoder_state.container2_storage_known = false;
							train_state.decoder_state.container3_storage_known = false;
							last_scalar = TRAIN_ID_VALUE;
							break;
						case TRAIN_ID_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "dcc-address")) {
								last_scalar = TRAIN_DCC_ADDR_KEY;
							} else {
								error = true;
							}
							break;
						case TRAIN_DCC_ADDR_KEY:
							if (bidib_string_to_dccaddr((char *) event.data.scalar.value, &train.dcc_addr)) {
								error = true;
								syslog_libbidib(LOG_ERR, "Dcc address of train %s is in wrong format",
								                train.id->str);
							} else {
								last_scalar = TRAIN_DCC_ADDR_VALUE;
							}
							break;
						case TRAIN_DCC_ADDR_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "dcc-speed-steps")) {
								last_scalar = TRAIN_DCC_STEPS_KEY;
							} else {
								error = true;
							}
							break;
						case TRAIN_DCC_STEPS_KEY:
							if (bidib_string_to_byte((char *) event.data.scalar.value, &train.dcc_speed_steps)) {
								error = true;
								syslog_libbidib(LOG_ERR, "Speed steps value of train %s is in wrong format",
								                train.id->str);
							} else if (train.dcc_speed_steps != 14 && train.dcc_speed_steps != 28 &&
							           train.dcc_speed_steps != 126) {
								error = true;
								syslog_libbidib(LOG_ERR, "Train %s has invalid speed steps value", 
								                train.id->str);
							} else {
								last_scalar = TRAIN_DCC_STEPS_VALUE;
							}
							break;
						case TRAIN_DCC_STEPS_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "calibration")) {
								last_scalar = TRAIN_CALIBRATION_KEY;
							} else if (!strcmp((char *) event.data.scalar.value, "peripherals")) {
								last_scalar = TRAIN_PERIPHERALS_KEY;
							} else {
								error = true;
							}
							break;
						case TRAIN_CALIBRATION_VALUE:
							if (!strcmp((char *) event.data.scalar.value, "peripherals")) {
								last_scalar = TRAIN_PERIPHERALS_KEY;
							} else {
								error = true;
							}
							break;
						case TRAIN_PERIPHERALS_VALUE:
						//	error = true;
							break;
						default:
							break;
					}
				}
				break;
		}
		yaml_event_delete(&event);
	}

	if (error) {
		bidib_state_free_single_train(train);
		bidib_state_free_single_train_state_intern(train_state);
	} else {
		if ((error = bidib_state_add_train(train))) {
			syslog_libbidib(LOG_ERR, "Train %s configured with same id or dcc address as another train",
			                train.id->str);
			bidib_state_free_single_train(train);
			bidib_state_free_single_train_state_intern(train_state);
		} else {
			bidib_state_add_train_state(train_state);
		}
	}

	return error;
}

int bidib_config_parse_train_config(const char *config_dir) {
	FILE *fh;
	yaml_parser_t parser;
	if (bidib_config_init_parser(config_dir, "/bidib_train_config.yml", &fh,
	                             &parser)) {
		return true;
	}

	bool error = bidib_config_parse_scalar_then_section(&parser, "trains",
	                                                    bidib_config_parse_single_train);

	if (error) {
		syslog_libbidib(LOG_ERR, "%s", "Error while parsing train config");
	}

	yaml_parser_delete(&parser);
	fclose(fh);
	return error;
}
