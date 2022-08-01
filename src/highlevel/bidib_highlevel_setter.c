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

#include <string.h>
#include <stdint.h>

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../state/bidib_state_intern.h"
#include "../state/bidib_state_getter_intern.h"
#include "../transmission/bidib_transmission_intern.h"
#include "bidib_highlevel_intern.h"
#include "../../include/definitions/bidib_definitions_custom.h"
#include "../lowlevel/bidib_lowlevel_intern.h"


static t_bidib_aspect *bidib_get_aspect_by_id(GArray *aspects, const char *aspect_id) {
	t_bidib_aspect *aspect_mapping;
	for (size_t i = 0; i < aspects->len; i++) {
		aspect_mapping = &g_array_index(aspects, t_bidib_aspect, i);
		if (!strcmp(aspect_mapping->id->str, aspect_id)) {
			return aspect_mapping;
		}
	}
	return NULL;
}

static t_bidib_dcc_aspect *bidib_get_dcc_aspect_by_id(GArray *aspects, const char *aspect_id) {
	t_bidib_dcc_aspect *aspect_mapping;
	for (size_t i = 0; i < aspects->len; i++) {
		aspect_mapping = &g_array_index(aspects, t_bidib_dcc_aspect, i);
		if (!strcmp(aspect_mapping->id->str, aspect_id)) {
			return aspect_mapping;
		}
	}
	return NULL;
}

int bidib_switch_point(const char *point, const char *aspect) {
	if (point == NULL || aspect == NULL) {
		syslog_libbidib(LOG_ERR, "Switch point: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board_i;
	t_bidib_board_accessory_mapping *board_mapping;
	t_bidib_dcc_accessory_mapping *dcc_mapping;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);

		for (size_t j = 0; j < board_i->points_board->len; j++) {

			board_mapping = &g_array_index(board_i->points_board, t_bidib_board_accessory_mapping, j);
			if (!strcmp(point, board_mapping->id->str)) {

				if (!board_i->connected) {
					pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Switch point %s: board %s is not connected",
					                point, board_i->id->str);
					return 1;
				}

				t_bidib_node_address tmp_addr = board_i->node_addr;
				pthread_mutex_unlock(&bidib_state_boards_mutex);
				t_bidib_aspect *aspect_mapping = bidib_get_aspect_by_id(board_mapping->aspects, aspect);
				if (aspect_mapping != NULL) {
					unsigned int action_id = bidib_get_and_incr_action_id();
					syslog_libbidib(LOG_NOTICE, "Switch point: %s on board: %s (0x%02x 0x%02x "
					                "0x%02x 0x00) to aspect: %s (0x%02x) with action id: %d",
					                point, board_i->id->str, tmp_addr.top, tmp_addr.sub,
					                tmp_addr.subsub, aspect, aspect_mapping->value, action_id);
					bidib_send_accessory_set(tmp_addr, board_mapping->number, aspect_mapping->value, action_id);
					return 0;
				} else {
					//INCORRECT DOUBLE UNLOCK
					//pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Switch point %s: aspect %s doesn't exist", point, aspect);
					return 1;
				}
			}
		}
		for (size_t j = 0; j < board_i->points_dcc->len; j++) {
			
			dcc_mapping = &g_array_index(board_i->points_dcc, t_bidib_dcc_accessory_mapping, j);
			
			if (!strcmp(point, dcc_mapping->id->str)) {
				
				if (!board_i->connected) {
					pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Switch point %s: board %s is not connected", point, board_i->id->str);
					return 1;
				}
				t_bidib_node_address tmp_addr = board_i->node_addr;
				pthread_mutex_unlock(&bidib_state_boards_mutex);
				t_bidib_dcc_aspect *aspect_mapping = bidib_get_dcc_aspect_by_id(dcc_mapping->aspects, aspect);

				if (aspect_mapping != NULL) {
					t_bidib_cs_accessory_mod params;
					params.dcc_address = dcc_mapping->dcc_addr;
					params.time = 0x00;
					unsigned int action_id = bidib_get_and_incr_action_id();
					t_bidib_dcc_aspect_port_value *aspect_port_value;
					
					for (size_t k = 0; k < aspect_mapping->port_values->len; k++) {
						aspect_port_value = &g_array_index(aspect_mapping->port_values, t_bidib_dcc_aspect_port_value, k);
						params.data = (uint8_t) (aspect_port_value->port & 0x1F);
						params.data = params.data | (aspect_port_value->value << 5);
						params.data = params.data | (dcc_mapping->extended_accessory << 7);
						bidib_send_cs_accessory(tmp_addr, params, action_id);
					}
					
					pthread_mutex_lock(&bidib_state_track_mutex); //BL: Moved lock one line up
					t_bidib_dcc_accessory_state *accessory_state = bidib_state_get_dcc_accessory_state_ref(point, true);
					if (accessory_state != NULL) { //BL addition
						accessory_state->data.state_id = aspect_mapping->id->str;
						//Todo add log message for else.
					}
					pthread_mutex_unlock(&bidib_state_track_mutex);
					syslog_libbidib(LOG_NOTICE, "Switch point: %s on board: %s (0x%02x 0x%02x "
					                "0x%02x 0x00) to aspect: %s with action id: %d",
					                point, board_i->id->str, tmp_addr.top, tmp_addr.sub,
					                tmp_addr.subsub, aspect, action_id);
					return 0;
				} else {
					//INCORRECT DOUBLE UNLOCK
					//pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Switch point %s: aspect %s doesn't exist",
					                point, aspect);
					return 1;
				}
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	syslog_libbidib(LOG_ERR, "Switch point %s: not found", point);
	return 1;
}

int bidib_set_signal(const char *signal, const char *aspect) {
	if (signal == NULL || aspect == NULL) {
		syslog_libbidib(LOG_ERR, "Set signal: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board_i;
	t_bidib_board_accessory_mapping *board_mapping;
	t_bidib_dcc_accessory_mapping *dcc_mapping;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		for (size_t j = 0; j < board_i->signals_board->len; j++) {
			board_mapping = &g_array_index(
					board_i->signals_board, t_bidib_board_accessory_mapping, j);
			if (!strcmp(signal, board_mapping->id->str)) {
				if (!board_i->connected) {
					pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Set signal %s: board %s is not connected",
					                signal, board_i->id->str);
					return 1;
				}
				t_bidib_node_address tmp_addr = board_i->node_addr;
				pthread_mutex_unlock(&bidib_state_boards_mutex);
				t_bidib_aspect *aspect_mapping = bidib_get_aspect_by_id(board_mapping->aspects, aspect);
				if (aspect_mapping != NULL) {
					unsigned int action_id = bidib_get_and_incr_action_id();
					syslog_libbidib(LOG_NOTICE, "Set signal: %s on board: %s (0x%02x 0x%02x "
					                "0x%02x 0x00) to aspect: %s (0x%02x) with action id: %d",
					                signal, board_i->id->str, tmp_addr.top, tmp_addr.sub, tmp_addr.subsub,
					                aspect_mapping->id->str, aspect_mapping->value, action_id);
					bidib_send_accessory_set(tmp_addr, board_mapping->number,
					                         aspect_mapping->value, action_id);
					return 0;
				} else {
					//INCORRECT DOUBLE UNLOCK
					//pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Set signal %s: aspect %s doesn't exist",
					                signal, aspect);
					return 1;
				}
			}
		}
		for (size_t j = 0; j < board_i->signals_dcc->len; j++) {
			dcc_mapping = &g_array_index(
					board_i->signals_dcc, t_bidib_dcc_accessory_mapping, j);
			if (!strcmp(signal, dcc_mapping->id->str)) {
				if (!board_i->connected) {
					pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Set signal %s: board %s is not connected",
					                signal, board_i->id->str);
					return 1;
				}
				t_bidib_node_address tmp_addr = board_i->node_addr;
				pthread_mutex_unlock(&bidib_state_boards_mutex);
				t_bidib_dcc_aspect *aspect_mapping = bidib_get_dcc_aspect_by_id(dcc_mapping->aspects, aspect);
				if (aspect_mapping != NULL) {
					t_bidib_cs_accessory_mod params;
					params.dcc_address = dcc_mapping->dcc_addr;
					params.time = 0x00;
					unsigned int action_id = bidib_get_and_incr_action_id();
					t_bidib_dcc_aspect_port_value *aspect_port_value;
					for (size_t k = 0; k < aspect_mapping->port_values->len; k++) {
						aspect_port_value = &g_array_index(aspect_mapping->port_values, t_bidib_dcc_aspect_port_value, k);
						params.data = (uint8_t) (aspect_port_value->port & 0x1F);
						params.data = (uint8_t) (aspect_port_value->value | (1 << 5));
						params.data = params.data | (dcc_mapping->extended_accessory << 7);
						bidib_send_cs_accessory(tmp_addr, params, action_id);
					}
					pthread_mutex_lock(&bidib_state_track_mutex);//BL: Moved lock one line up
					t_bidib_dcc_accessory_state *accessory_state = bidib_state_get_dcc_accessory_state_ref(signal, false);
					if (accessory_state != NULL) { //BL addition
						accessory_state->data.state_id = aspect_mapping->id->str;
						//Todo add log message for else.
					}
					pthread_mutex_unlock(&bidib_state_track_mutex);
					syslog_libbidib(LOG_NOTICE, "Set signal: %s on board: %s (0x%02x 0x%02x "
					                "0x%02x 0x00) to aspect: %s with action id: %d",
					                signal, board_i->id->str, tmp_addr.top, tmp_addr.sub,
					                tmp_addr.subsub, aspect, action_id);
					return 0;
				} else {
					//INCORRECT DOUBLE UNLOCK
					//pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Set signal %s: aspect %s doesn't exist", signal, aspect);
					return 1;
				}
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	syslog_libbidib(LOG_ERR, "Set signal %s: not found", signal);
	return 1;
}

int bidib_set_peripheral(const char *peripheral, const char *aspect) {
	if (peripheral == NULL || aspect == NULL) {
		syslog_libbidib(LOG_ERR, "Set peripheral: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board_i;
	t_bidib_peripheral_mapping *peripheral_mapping;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		for (size_t j = 0; j < board_i->peripherals->len; j++) {
			peripheral_mapping = &g_array_index(
					board_i->peripherals, t_bidib_peripheral_mapping, j);
			if (!strcmp(peripheral, peripheral_mapping->id->str)) {
				if (!board_i->connected) {
					pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Set peripheral %s: board %s is not connected",
					                peripheral, board_i->id->str);
					return 1;
				}
				t_bidib_aspect *aspect_mapping = bidib_get_aspect_by_id(peripheral_mapping->aspects, aspect);
				if (aspect_mapping != NULL) {
					unsigned int action_id = bidib_get_and_incr_action_id();
					syslog_libbidib(LOG_NOTICE, "Set peripheral: %s on board: %s (0x%02x 0x%02x "
					                "0x%02x 0x00) to aspect: %s (0x%02x) with action id: %d",
					                peripheral, board_i->id->str, board_i->node_addr.top,
					                board_i->node_addr.sub, board_i->node_addr.subsub,
					                aspect_mapping->id->str, aspect_mapping->value, action_id);
					bidib_send_lc_output(board_i->node_addr, peripheral_mapping->port.port0,
					                     peripheral_mapping->port.port1, aspect_mapping->value, action_id);
					pthread_mutex_unlock(&bidib_state_boards_mutex);
					return 0;
				} else {
					pthread_mutex_unlock(&bidib_state_boards_mutex);
					syslog_libbidib(LOG_ERR, "Set peripheral %s: aspect %s doesn't exist",
					                peripheral, aspect);
					return 1;
				}
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	syslog_libbidib(LOG_ERR, "Set peripheral %s: not found", peripheral);
	return 1;
}

int bidib_set_train_speed(const char *train, int speed, const char *track_output) {
	if (train == NULL || track_output == NULL) {
		syslog_libbidib(LOG_ERR, "Set train speed: parameters must not be NULL");
		return 1;
	}
	if (speed < -126 || speed > 126) {
		syslog_libbidib(LOG_ERR, "Set train speed: illegal speed value %d, must be in range -126...126",
		                speed);
		return 1;
	}
	t_bidib_train *tmp_train = bidib_state_get_train_ref(train);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board = bidib_state_get_board_ref(track_output);
	if (tmp_train == NULL) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set train speed: train %s doesn't exist", train);
		return 1;
	} else if (board == NULL || !board->connected) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set train speed: board %s doesn't exist or is not connected", track_output);
		return 1;
	} else if (!(board->unique_id.class_id & (1 << 4))) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set train speed: board %s has no track output", track_output);
		return 1;
	} else {
		t_bidib_cs_drive_mod params;
		params.dcc_address = tmp_train->dcc_addr;
		switch (tmp_train->dcc_speed_steps) {
			case 28:
				params.dcc_format = 0x02;
				break;
			case 126:
				params.dcc_format = 0x03;
				break;
			case 14:
			default:
				params.dcc_format = 0x00;
				break;
		}
		params.active = 0x01;
		params.speed = bidib_lib_speed_to_dcc_format(speed);
		params.function1 = 0x00;
		params.function2 = 0x00;
		params.function3 = 0x00;
		params.function4 = 0x00;
		unsigned int action_id = bidib_get_and_incr_action_id();
		syslog_libbidib(LOG_NOTICE, "Set train: %s to speed: %d via board: %s (0x%02x "
		                "0x%02x 0x%02x 0x00) with action id: %d",
		                train, speed, board->id->str, board->node_addr.top,
		                board->node_addr.sub, board->node_addr.subsub, action_id);
		t_bidib_node_address tmp_addr = board->node_addr;
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		bidib_send_cs_drive(tmp_addr, params, action_id);
		return 0;
	}
}

int bidib_set_calibrated_train_speed(const char *train, int speed, const char *track_output) {
	if (train == NULL || track_output == NULL) {
		syslog_libbidib(LOG_ERR, "Set calibrated train speed: parameters must not be NULL");
		return 1;
	}
	if (speed < -9 || speed > 9) {
		syslog_libbidib(LOG_ERR, "Set calibrated train speed: speed must be in range -9...9");
		return 1;
	}
	
	t_bidib_train *tmp_train = bidib_state_get_train_ref(train);
	
	if (tmp_train == NULL) {
		syslog_libbidib(LOG_ERR, "Set calibrated train speed: %s train does not exist", train);
		return 1;
	}
	
	int error = 0;
	if (tmp_train->calibration == NULL) {
		syslog_libbidib(LOG_ERR, "Set calibrated train speed: no calibration for train %s", 
		                tmp_train->id->str);
		error = 1;
	} else {
		if (speed < 0) {
			error = bidib_set_train_speed(
				train, g_array_index(tmp_train->calibration, int, (speed * -1) - 1) * -1,
				track_output);
		} else if (speed == 0) {
			error = bidib_set_train_speed(train, 0, track_output);
		} else {
			error = bidib_set_train_speed(
				train, g_array_index(tmp_train->calibration, int, speed - 1),
				track_output);
		}
	}
	return error;
}

int bidib_emergency_stop_train(const char *train, const char *track_output) {
	if (train == NULL || track_output == NULL) {
		syslog_libbidib(LOG_ERR, "Emergency stop train: parameters must not be NULL");
		return 1;
	}
	t_bidib_train *tmp_train = bidib_state_get_train_ref(train);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board = bidib_state_get_board_ref(track_output);
	if (tmp_train == NULL) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Emergency stop train: train %s doesn't exist", train);
		return 1;
	} else if (board == NULL || !board->connected) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Emergency stop train: board %s doesn't exist or is not connected", 
		                track_output);
		return 1;
	} else if (!(board->unique_id.class_id & (1 << 4))) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Emergency stop train: board %s is no track output", 
		                track_output);
		return 1;
	} else {
		t_bidib_cs_drive_mod params;
		params.dcc_address = tmp_train->dcc_addr;
		switch (tmp_train->dcc_speed_steps) {
			case 28:
				params.dcc_format = 0x02;
				break;
			case 126:
				params.dcc_format = 0x03;
				break;
			case 14:
			default:
				params.dcc_format = 0x00;
				break;
		}
		params.active = 0x01;
		params.speed = 0x81;
		params.function1 = 0x00;
		params.function2 = 0x00;
		params.function3 = 0x00;
		params.function4 = 0x00;
		unsigned int action_id = bidib_get_and_incr_action_id();
		syslog_libbidib(LOG_NOTICE, "Emergency stop train: %s via board: %s (0x%02x "
		                "0x%02x 0x%02x 0x00) with action id: %d",
		                train, board->id->str, board->node_addr.top,
		                board->node_addr.sub, board->node_addr.subsub, action_id);
		t_bidib_node_address tmp_addr = board->node_addr;
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		bidib_send_cs_drive(tmp_addr, params, action_id);
		return 0;
	}
}

static void bidib_get_current_train_peripheral_bits(t_bidib_train *train, size_t start,
                                                    size_t end, uint8_t *bits) {
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref(train->id->str);
	t_bidib_train_peripheral_mapping *mapping_i;
	t_bidib_train_peripheral_state *train_per_state_i;
	for (size_t i = 0; i < train->peripherals->len; i++) {
		mapping_i = &g_array_index(train->peripherals, t_bidib_train_peripheral_mapping, i);
		if (mapping_i->bit >= start && mapping_i->bit <= end) {
			for (size_t j = 0; j < train->peripherals->len; j++) {
				train_per_state_i = bidib_state_get_train_peripheral_state_by_bit(
						train_state, mapping_i->bit);
				*bits |= (train_per_state_i->state << (mapping_i->bit % 8));
			}
		}
	}
}

int bidib_set_train_peripheral(const char *train, const char *peripheral, uint8_t state,
                               const char *track_output) {
	if (train == NULL || peripheral == NULL || track_output == NULL) {
		syslog_libbidib(LOG_ERR, "Set train peripheral: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	
	t_bidib_train *tmp_train = bidib_state_get_train_ref(train);
	t_bidib_board *board = bidib_state_get_board_ref(track_output);
	if (tmp_train == NULL) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set train peripheral: train %s doesn't exist", train);
		return 1;
	} else if (board == NULL || !board->connected) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set train peripheral: board %s doesn't exist or is not connected", 
		                track_output);
		return 1;
	} else if (!(board->unique_id.class_id & (1 << 4))) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set train peripheral: board %s is no track output", track_output);
		return 1;
	}

	t_bidib_train_peripheral_mapping *mapping_i;
	for (size_t i = 0; i < tmp_train->peripherals->len; i++) {
		mapping_i = &g_array_index(
				tmp_train->peripherals, t_bidib_train_peripheral_mapping, i);
		if (!strcmp(peripheral, mapping_i->id->str)) {
			t_bidib_cs_drive_mod params;
			params.dcc_address = tmp_train->dcc_addr;
			switch (tmp_train->dcc_speed_steps) {
				case 28:
					params.dcc_format = 0x02;
					break;
				case 126:
					params.dcc_format = 0x03;
					break;
				case 14:
				default:
					params.dcc_format = 0x00;
					break;
			}
			params.speed = 0x00;
			uint8_t function_bits[] = {0x00, 0x00, 0x00, 0x00};
			pthread_mutex_lock(&bidib_state_trains_mutex);
			if (mapping_i->bit < 5) {
				params.active = (1 << 1);
				bidib_get_current_train_peripheral_bits(tmp_train, 0, 4,
				                                        &function_bits[0]);
			} else if (mapping_i->bit < 12) {
				params.active = (1 << 2);
				bidib_get_current_train_peripheral_bits(tmp_train, 8, 11,
				                                        &function_bits[1]);
			} else if (mapping_i->bit < 16) {
				params.active = (1 << 3);
				bidib_get_current_train_peripheral_bits(tmp_train, 12, 15,
				                                        &function_bits[1]);
			} else if (mapping_i->bit < 24) {
				params.active = (1 << 4);
				bidib_get_current_train_peripheral_bits(tmp_train, 16, 23,
				                                        &function_bits[2]);
			} else {
				params.active = (1 << 5);
				bidib_get_current_train_peripheral_bits(tmp_train, 24, 31,
				                                        &function_bits[3]);
			}
			function_bits[mapping_i->bit / 8] &= ~(1 << (mapping_i->bit % 8));
			function_bits[mapping_i->bit / 8] |= (state << (mapping_i->bit % 8));
			params.function1 = function_bits[0];
			params.function2 = function_bits[1];
			params.function3 = function_bits[2];
			params.function4 = function_bits[3];
			unsigned int action_id = bidib_get_and_incr_action_id();
			syslog_libbidib(LOG_NOTICE, "Set train peripheral: %s of train: %s to "
			                "state: 0x%02x via board: %s (0x%02x 0x%02x "
			                "0x%02x 0x00) with action id: %d",
			                peripheral, train, state, board->id->str, board->node_addr.top,
			                board->node_addr.sub, board->node_addr.subsub, action_id);
			t_bidib_node_address tmp_addr = board->node_addr;
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			bidib_send_cs_drive_intern(tmp_addr, params, action_id, false);
			pthread_mutex_unlock(&bidib_state_trains_mutex);
			return 0;
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	syslog_libbidib(LOG_ERR, "Set train peripheral: peripheral %s doesn't exist", peripheral);
	return 1;
}

int bidib_set_booster_power_state(const char *booster, bool on) {
	if (booster == NULL) {
		syslog_libbidib(LOG_ERR, "Set booster power state: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board = bidib_state_get_board_ref(booster);
	if (board == NULL || !board->connected) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set booster: board %s doesn't exist or is not connected", 
		                booster);
		return 1;
	} else if (!(board->unique_id.class_id & (1 << 1))) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set booster: board %s is no booster", booster);
		return 1;
	} else {
		unsigned int action_id = bidib_get_and_incr_action_id();
		if (on) {
			syslog_libbidib(LOG_NOTICE, "Set booster: %s (0x%02x 0x%02x "
			                "0x%02x 0x00) to state: %s with action id: %d",
			                board->id->str, board->node_addr.top, board->node_addr.sub,
			                board->node_addr.subsub, "on", action_id);
			t_bidib_node_address tmp_addr = board->node_addr;
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			bidib_send_boost_on(tmp_addr, 0x01, action_id);
		} else {
			syslog_libbidib(LOG_NOTICE, "Set booster: %s (0x%02x 0x%02x "
			                "0x%02x 0x00) to state: %s with action id: %d",
			                board->id->str, board->node_addr.top, board->node_addr.sub,
			                board->node_addr.subsub, "off", action_id);
			t_bidib_node_address tmp_addr = board->node_addr;
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			bidib_send_boost_off(tmp_addr, 0x01, action_id);
		}
		return 0;
	}
}

int bidib_set_track_output_state(const char *track_output, t_bidib_cs_state state) {
	if (track_output == NULL) {
		syslog_libbidib(LOG_ERR, "Set track output state: parameters must not be NULL");
		return 1;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board = bidib_state_get_board_ref(track_output);
	if (board == NULL || !board->connected) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set track output state: board %s doesn't exist or is not connected",
		                track_output);
		return 1;
	} else if (!(board->unique_id.class_id & (1 << 4))) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog_libbidib(LOG_ERR, "Set track output state: board %s is no track output", 
		                track_output);
		return 1;
	} else {
		unsigned int action_id = bidib_get_and_incr_action_id();
		syslog_libbidib(LOG_NOTICE, "Set track output: %s (0x%02x 0x%02x "
		                "0x%02x 0x00) to state: 0x%02x with action id: %d",
		                board->id->str, board->node_addr.top, board->node_addr.sub,
		                board->node_addr.subsub, state, action_id);
		t_bidib_node_address tmp_addr = board->node_addr;
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		bidib_send_cs_set_state(tmp_addr, state, action_id);
		return 0;
	}
}

void bidib_set_track_output_state_all(t_bidib_cs_state state) {
	pthread_mutex_lock(&bidib_state_boards_mutex);
	unsigned int action_id = bidib_get_and_incr_action_id();
	syslog_libbidib(LOG_NOTICE, "Set all track outputs to state: 0x%02x with action id: %d",
	                state, action_id);
	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if ((board_i->unique_id.class_id & (1 << 4)) && board_i->connected) {
			bidib_send_cs_set_state(board_i->node_addr, state, action_id);
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
}
