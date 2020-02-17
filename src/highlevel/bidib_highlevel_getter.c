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

#include <memory.h>
#include <stdlib.h>
#include <glib.h>

#include "../state/bidib_state_intern.h"
#include "../../include/bidib.h"
#include "../state/bidib_state_getter_intern.h"
#include "../../include/definitions/bidib_definitions_custom.h"


static t_bidib_board_accessory_state *bidib_get_state_accessories_board(GArray *accessories) {
	t_bidib_board_accessory_state *state = malloc(
			sizeof(t_bidib_board_accessory_state) * accessories->len);
	t_bidib_board_accessory_state *tmp;
	for (size_t i = 0; i < accessories->len; i++) {
		tmp = &g_array_index(accessories, t_bidib_board_accessory_state, i);
		state[i] = *tmp;
		state[i].id = malloc(sizeof(char) * (strlen(tmp->id) + 1));
		strcpy(state[i].id, tmp->id);
		char *state_id;
		if (tmp->data.state_id != NULL) {
			state_id = tmp->data.state_id;
		} else {
			state_id = "unknown";
		}
		state[i].data.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
		strcpy(state[i].data.state_id, state_id);
	}
	return state;
}

static t_bidib_dcc_accessory_state *bidib_get_state_accessories_dcc(GArray *accessories) {
	t_bidib_dcc_accessory_state *state = malloc(
			sizeof(t_bidib_dcc_accessory_state) * accessories->len);
	t_bidib_dcc_accessory_state *tmp;
	for (size_t i = 0; i < accessories->len; i++) {
		tmp = &g_array_index(accessories, t_bidib_dcc_accessory_state, i);
		state[i] = *tmp;
		state[i].id = malloc(sizeof(char) * (strlen(tmp->id) + 1));
		strcpy(state[i].id, tmp->id);
		char *state_id;
		if (tmp->data.state_id != NULL) {
			state_id = tmp->data.state_id;
		} else {
			state_id = "unknown";
		}
		state[i].data.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
		strcpy(state[i].data.state_id, state_id);
	}
	return state;
}

static t_bidib_peripheral_state *bidib_get_state_peripherals(void) {
	t_bidib_peripheral_state *state = malloc(
			sizeof(t_bidib_peripheral_state) * bidib_track_state.peripherals->len);
	t_bidib_peripheral_state *tmp;
	for (size_t i = 0; i < bidib_track_state.peripherals->len; i++) {
		tmp = &g_array_index(bidib_track_state.peripherals, t_bidib_peripheral_state, i);
		state[i] = *tmp;
		state[i].id = malloc(sizeof(char) * (strlen(tmp->id) + 1));
		strcpy(state[i].id, tmp->id);
		char *state_id;
		if (tmp->data.state_id != NULL) {
			state_id = tmp->data.state_id;
		} else {
			state_id = "unknown";
		}
		state[i].data.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
		strcpy(state[i].data.state_id, state_id);
	}
	return state;
}

static t_bidib_segment_state *bidib_get_state_segments(void) {
	t_bidib_segment_state *state = malloc(
			sizeof(t_bidib_segment_state) * bidib_track_state.segments->len);
	t_bidib_segment_state_intern *tmp;
	for (size_t i = 0; i < bidib_track_state.segments->len; i++) {
		tmp = &g_array_index(bidib_track_state.segments, t_bidib_segment_state_intern, i);
		state[i].id = malloc(sizeof(char) * (tmp->id->len + 1));
		strcpy(state[i].id, tmp->id->str);
		state[i].data.occupied = tmp->occupied;
		state[i].data.confidence = tmp->confidence;
		state[i].data.power_consumption = tmp->power_consumption;
		state[i].data.dcc_address_cnt = tmp->dcc_addresses->len;
		state[i].data.dcc_addresses = malloc(sizeof(t_bidib_dcc_address) * tmp->dcc_addresses->len);
		memcpy(state[i].data.dcc_addresses, tmp->dcc_addresses->data,
		       sizeof(t_bidib_dcc_address) * tmp->dcc_addresses->len);
	}
	return state;
}

static t_bidib_train_state *bidib_get_state_trains(void) {
	t_bidib_train_state *state = malloc(
			sizeof(t_bidib_train_state) * bidib_track_state.trains->len);
	t_bidib_train_state_intern *tmp;
	for (size_t i = 0; i < bidib_track_state.trains->len; i++) {
		tmp = &g_array_index(bidib_track_state.trains, t_bidib_train_state_intern, i);
		state[i].id = malloc(sizeof(char) * (tmp->id->len + 1));
		strcpy(state[i].id, tmp->id->str);
		state[i].data.on_track = tmp->on_track;
		state[i].data.orientation = tmp->orientation;
		state[i].data.set_speed_step = tmp->set_speed_step;
		state[i].data.ack = tmp->ack;
		state[i].data.detected_kmh_speed = tmp->detected_kmh_speed;
		state[i].data.peripheral_cnt = tmp->peripherals->len;
		state[i].data.peripherals = malloc(
				sizeof(t_bidib_train_peripheral_state) * tmp->peripherals->len);
		t_bidib_train_peripheral_state peripheral_state_i;
		for (size_t j = 0; j < tmp->peripherals->len; j++) {
			peripheral_state_i = g_array_index(tmp->peripherals,
			                                   t_bidib_train_peripheral_state, j);
			state[i].data.peripherals[j].id = malloc(
					sizeof(char) * (strlen(peripheral_state_i.id) + 1));
			strcpy(state[i].data.peripherals[j].id, peripheral_state_i.id);
			state[i].data.peripherals[j].state = peripheral_state_i.state;
		}
		state[i].data.decoder_state = tmp->decoder_state;
	}
	return state;
}

static t_bidib_booster_state *bidib_get_state_boosters(void) {
	t_bidib_booster_state *state = malloc(
			sizeof(t_bidib_booster_state) * bidib_track_state.boosters->len);
	t_bidib_booster_state *tmp;
	for (size_t i = 0; i < bidib_track_state.boosters->len; i++) {
		tmp = &g_array_index(bidib_track_state.boosters, t_bidib_booster_state, i);
		state[i].id = malloc(sizeof(char) * (strlen(tmp->id) + 1));
		strcpy(state[i].id, tmp->id);
		state[i].data.power_state = tmp->data.power_state;
		state[i].data.power_state_simple = tmp->data.power_state_simple;
		state[i].data.power_consumption = tmp->data.power_consumption;
		state[i].data.voltage_known = tmp->data.voltage_known;
		state[i].data.voltage = tmp->data.voltage;
		state[i].data.temp_celsius = tmp->data.temp_celsius;
	}
	return state;
}

static t_bidib_track_output_state *bidib_get_state_track_outputs(void) {
	t_bidib_track_output_state *state = malloc(
			sizeof(t_bidib_track_output_state) * bidib_track_state.track_outputs->len);
	t_bidib_track_output_state *tmp;
	for (size_t i = 0; i < bidib_track_state.track_outputs->len; i++) {
		tmp = &g_array_index(bidib_track_state.track_outputs, t_bidib_track_output_state, i);
		state[i].id = malloc(sizeof(char) * (strlen(tmp->id) + 1));
		strcpy(state[i].id, tmp->id);
		state[i].cs_state = tmp->cs_state;
	}
	return state;
}

t_bidib_track_state bidib_get_state(void) {
	t_bidib_track_state query = {0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL,
	                             0, NULL, 0, NULL, 0, NULL, 0, NULL};
	pthread_mutex_lock(&bidib_state_track_mutex);
	query.points_board_count = bidib_track_state.points_board->len;
	query.points_board = bidib_get_state_accessories_board(bidib_track_state.points_board);
	query.points_dcc_count = bidib_track_state.points_dcc->len;
	query.points_dcc = bidib_get_state_accessories_dcc(bidib_track_state.points_dcc);
	query.signals_board_count = bidib_track_state.signals_board->len;
	query.signals_board = bidib_get_state_accessories_board(bidib_track_state.signals_board);
	query.signals_dcc_count = bidib_track_state.signals_dcc->len;
	query.signals_dcc = bidib_get_state_accessories_dcc(bidib_track_state.signals_dcc);
	query.peripherals_count = bidib_track_state.peripherals->len;
	query.peripherals = bidib_get_state_peripherals();
	query.segments_count = bidib_track_state.segments->len;
	query.segments = bidib_get_state_segments();
	query.trains_count = bidib_track_state.trains->len;
	query.trains = bidib_get_state_trains();
	query.booster_count = bidib_track_state.boosters->len;
	query.booster = bidib_get_state_boosters();
	query.track_outputs_count = bidib_track_state.track_outputs->len;
	query.track_outputs = bidib_get_state_track_outputs();
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_boards(void) {
	t_bidib_id_list_query query = {0, NULL};
	if (bidib_boards->len > 0) {
		query.length = bidib_boards->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_board board_i;
		for (size_t i = 0; i < bidib_boards->len; i++) {
			board_i = g_array_index(bidib_boards, t_bidib_board, i);
			query.ids[i] = malloc(sizeof(char) * (board_i.id->len + 1));
			strcpy(query.ids[i], board_i.id->str);
		}
	}
	return query;
}

t_bidib_id_list_query bidib_get_boards_connected(void) {
	t_bidib_id_list_query query = {0, NULL};
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		if (g_array_index(bidib_boards, t_bidib_board, i).connected) {
			count++;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		t_bidib_board tmp;
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_boards->len && current_index < count; i++) {
			tmp = g_array_index(bidib_boards, t_bidib_board, i);
			if (tmp.connected) {
				query.ids[current_index] = malloc(sizeof(char) * (tmp.id->len + 1));
				strcpy(query.ids[current_index], tmp.id->str);
				current_index++;
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

bool bidib_get_board_connected(const char *board) {
	if (board == NULL) {
		return false;
	}
	bool res = false;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *tmp = bidib_state_get_board_ref(board);
	if (tmp != NULL) {
		res = tmp->connected;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return res;
}

t_bidib_board_features_query bidib_get_board_features(const char *board) {
	t_bidib_board_features_query query = {0, NULL};
	if (board == NULL) {
		return query;
	}
	t_bidib_board *tmp = bidib_state_get_board_ref(board);
	if (tmp != NULL && tmp->features->len != 0) {
		query.length = tmp->features->len;
		query.features = malloc(sizeof(char) * query.length * 2);
		memcpy(query.features, tmp->features->data, sizeof(t_bidib_board_feature) * query.length);
	}
	return query;
}

t_bidib_id_query bidib_get_board_id(t_bidib_unique_id_mod unique_id) {
	t_bidib_id_query query = {false, NULL};
	t_bidib_board board_i;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = g_array_index(bidib_boards, t_bidib_board, i);
		if (bidib_state_uids_equal(&unique_id, &board_i.unique_id)) {
			query.known = true;
			query.id = malloc(sizeof(char) * (board_i.id->len + 1));
			strcpy(query.id, board_i.id->str);
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_unique_id_query bidib_get_uniqueid(const char *board) {
	t_bidib_unique_id_query query;
	query.known = false;
	if (board == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board_ref = bidib_state_get_board_ref(board);
	if (board_ref != NULL && board_ref->unique_id.class_id != 0xFF) {
		query.known = true;
		query.unique_id = board_ref->unique_id;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_unique_id_query bidib_get_uniqueid_by_nodeaddr(t_bidib_node_address node_address) {
	t_bidib_unique_id_query query;
	query.known = false;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board_ref = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (board_ref != NULL && board_ref->connected) {
		query.known = true;
		query.unique_id = board_ref->unique_id;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_node_address_query bidib_get_nodeaddr(const char *board) {
	t_bidib_node_address_query query;
	query.known_and_connected = false;
	if (board == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board_ref = bidib_state_get_board_ref(board);
	if (board_ref != NULL && board_ref->connected) {
		query.known_and_connected = true;
		query.address = board_ref->node_addr;
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_node_address_query bidib_get_nodeaddr_by_uniqueid(t_bidib_unique_id_mod uid) {
	t_bidib_node_address_query query;
	query.known_and_connected = false;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (bidib_state_uids_equal(&uid, &board_i->unique_id)) {
			query.known_and_connected = true;
			query.address = board_i->node_addr;
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_board_points(const char *board) {
	t_bidib_id_list_query query = {0, NULL};
	if (board == NULL) {
		return query;
	}
	t_bidib_board *board_ref = bidib_state_get_board_ref(board);
	if (board_ref != NULL && (board_ref->points_board->len > 0 ||
	                          board_ref->points_dcc->len > 0)) {
		query.length = board_ref->points_board->len + board_ref->points_dcc->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_board_accessory_mapping board_accessory_mapping;
		size_t current_index = 0;
		for (size_t i = 0; i < board_ref->points_board->len; i++) {
			board_accessory_mapping = g_array_index(board_ref->points_board,
			                                        t_bidib_board_accessory_mapping, i);
			query.ids[current_index] = malloc(
					sizeof(char) * (board_accessory_mapping.id->len + 1));
			strcpy(query.ids[current_index], board_accessory_mapping.id->str);
			current_index++;
		}
		t_bidib_dcc_accessory_mapping dcc_accessory_mapping;
		for (size_t i = 0; i < board_ref->points_dcc->len; i++) {
			dcc_accessory_mapping = g_array_index(board_ref->points_dcc,
			                                      t_bidib_dcc_accessory_mapping, i);
			query.ids[current_index] = malloc(
					sizeof(char) * (dcc_accessory_mapping.id->len + 1));
			strcpy(query.ids[current_index], dcc_accessory_mapping.id->str);
			current_index++;
		}
	}
	return query;
}

t_bidib_id_list_query bidib_get_board_signals(const char *board) {
	t_bidib_id_list_query query = {0, NULL};
	if (board == NULL) {
		return query;
	}
	t_bidib_board *board_ref = bidib_state_get_board_ref(board);
	if (board_ref != NULL && (board_ref->signals_board->len > 0 ||
	                          board_ref->signals_dcc->len > 0)) {
		query.length = board_ref->signals_board->len + board_ref->signals_dcc->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_board_accessory_mapping board_accessory_mapping;
		size_t current_index = 0;
		for (size_t i = 0; i < board_ref->signals_board->len; i++) {
			board_accessory_mapping = g_array_index(board_ref->signals_board,
			                                        t_bidib_board_accessory_mapping, i);
			query.ids[current_index] = malloc(
					sizeof(char) * (board_accessory_mapping.id->len + 1));
			strcpy(query.ids[current_index], board_accessory_mapping.id->str);
			current_index++;
		}
		t_bidib_dcc_accessory_mapping dcc_accessory_mapping;
		for (size_t i = 0; i < board_ref->signals_dcc->len; i++) {
			dcc_accessory_mapping = g_array_index(board_ref->signals_dcc,
			                                      t_bidib_dcc_accessory_mapping, i);
			query.ids[current_index] = malloc(
					sizeof(char) * (dcc_accessory_mapping.id->len + 1));
			strcpy(query.ids[current_index], dcc_accessory_mapping.id->str);
			current_index++;
		}
	}
	return query;
}

t_bidib_id_list_query bidib_get_board_peripherals(const char *board) {
	t_bidib_id_list_query query = {0, NULL};
	if (board == NULL) {
		return query;
	}
	t_bidib_board *board_ref = bidib_state_get_board_ref(board);
	if (board_ref != NULL && (board_ref->peripherals->len > 0)) {
		query.length = board_ref->peripherals->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_peripheral_mapping peripheral_mapping;
		for (size_t i = 0; i < board_ref->peripherals->len; i++) {
			peripheral_mapping = g_array_index(board_ref->peripherals,
			                                   t_bidib_peripheral_mapping, i);
			query.ids[i] = malloc(sizeof(char) * (peripheral_mapping.id->len + 1));
			strcpy(query.ids[i], peripheral_mapping.id->str);
		}
	}
	return query;
}

t_bidib_id_list_query bidib_get_board_segments(const char *board) {
	t_bidib_id_list_query query = {0, NULL};
	if (board == NULL) {
		return query;
	}
	t_bidib_board *board_ref = bidib_state_get_board_ref(board);
	if (board_ref != NULL && (board_ref->segments->len > 0)) {
		query.length = board_ref->segments->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_segment_mapping segment_mapping;
		for (size_t i = 0; i < board_ref->segments->len; i++) {
			segment_mapping = g_array_index(board_ref->segments,
			                                t_bidib_segment_mapping, i);
			query.ids[i] = malloc(sizeof(char) * (segment_mapping.id->len + 1));
			strcpy(query.ids[i], segment_mapping.id->str);
		}
	}
	return query;
}

t_bidib_id_list_query bidib_get_connected_points(void) {
	t_bidib_id_list_query query = {0, NULL};
	t_bidib_board board_ref;
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_ref = g_array_index(bidib_boards, t_bidib_board, i);
		if (board_ref.connected) {
			count += board_ref.points_board->len;
			count += board_ref.points_dcc->len;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_boards->len; i++) {
			board_ref = g_array_index(bidib_boards, t_bidib_board, i);
			if (board_ref.connected) {
				for (size_t j = 0; j < board_ref.points_board->len; j++) {
					t_bidib_board_accessory_mapping mapping = g_array_index(
							board_ref.points_board, t_bidib_board_accessory_mapping, j);
					query.ids[current_index] = malloc(sizeof(char) * (mapping.id->len) + 1);
					strcpy(query.ids[current_index], mapping.id->str);
					current_index++;
				}
				for (size_t j = 0; j < board_ref.points_dcc->len; j++) {
					t_bidib_dcc_accessory_mapping mapping = g_array_index(
							board_ref.points_dcc, t_bidib_dcc_accessory_mapping, j);
					query.ids[current_index] = malloc(sizeof(char) * (mapping.id->len) + 1);
					strcpy(query.ids[current_index], mapping.id->str);
					current_index++;
				}
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_connected_signals(void) {
	t_bidib_id_list_query query = {0, NULL};
	t_bidib_board board_ref;
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_ref = g_array_index(bidib_boards, t_bidib_board, i);
		if (board_ref.connected) {
			count += board_ref.signals_board->len;
			count += board_ref.signals_dcc->len;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_boards->len; i++) {
			board_ref = g_array_index(bidib_boards, t_bidib_board, i);
			if (board_ref.connected) {
				for (size_t j = 0; j < board_ref.signals_board->len; j++) {
					t_bidib_board_accessory_mapping mapping = g_array_index(
							board_ref.signals_board, t_bidib_board_accessory_mapping, j);
					query.ids[current_index] = malloc(sizeof(char) * (mapping.id->len) + 1);
					strcpy(query.ids[current_index], mapping.id->str);
					current_index++;
				}
				for (size_t j = 0; j < board_ref.signals_dcc->len; j++) {
					t_bidib_dcc_accessory_mapping mapping = g_array_index(
							board_ref.signals_dcc, t_bidib_dcc_accessory_mapping, j);
					query.ids[current_index] = malloc(sizeof(char) * (mapping.id->len) + 1);
					strcpy(query.ids[current_index], mapping.id->str);
					current_index++;
				}
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_connected_peripherals(void) {
	t_bidib_id_list_query query = {0, NULL};
	t_bidib_board board_ref;
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_ref = g_array_index(bidib_boards, t_bidib_board, i);
		if (board_ref.connected) {
			count += board_ref.peripherals->len;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_boards->len; i++) {
			board_ref = g_array_index(bidib_boards, t_bidib_board, i);
			if (board_ref.connected) {
				for (size_t j = 0; j < board_ref.peripherals->len; j++) {
					t_bidib_peripheral_mapping mapping = g_array_index(
							board_ref.peripherals, t_bidib_peripheral_mapping, j);
					query.ids[current_index] = malloc(sizeof(char) * (mapping.id->len) + 1);
					strcpy(query.ids[current_index], mapping.id->str);
					current_index++;
				}
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_connected_segments(void) {
	t_bidib_id_list_query query = {0, NULL};
	t_bidib_board board_ref;
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_ref = g_array_index(bidib_boards, t_bidib_board, i);
		if (board_ref.connected) {
			count += board_ref.segments->len;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_boards->len; i++) {
			board_ref = g_array_index(bidib_boards, t_bidib_board, i);
			if (board_ref.connected) {
				for (size_t j = 0; j < board_ref.segments->len; j++) {
					t_bidib_segment_mapping mapping = g_array_index(
							board_ref.segments, t_bidib_segment_mapping, j);
					query.ids[current_index] = malloc(sizeof(char) * (mapping.id->len) + 1);
					strcpy(query.ids[current_index], mapping.id->str);
					current_index++;
				}
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_connected_boosters(void) {
	t_bidib_id_list_query query = {0, NULL};
	t_bidib_board board_ref;
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_ref = g_array_index(bidib_boards, t_bidib_board, i);
		if (board_ref.connected && board_ref.unique_id.class_id & (1 << 1)) {
			count++;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_boards->len; i++) {
			board_ref = g_array_index(bidib_boards, t_bidib_board, i);
			if (board_ref.connected && (board_ref.unique_id.class_id & (1 << 1))) {
				query.ids[current_index] = malloc(sizeof(char) * (board_ref.id->len) + 1);
				strcpy(query.ids[current_index], board_ref.id->str);
				current_index++;
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_boosters(void) {
	t_bidib_id_list_query query = {0, NULL};
	if (bidib_track_state.boosters->len > 0) {
		query.length = bidib_track_state.boosters->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_booster_state state_i;
		for (size_t i = 0; i < bidib_track_state.boosters->len; i++) {
			state_i = g_array_index(bidib_track_state.boosters, t_bidib_booster_state, i);
			query.ids[i] = malloc(sizeof(char) * (strlen(state_i.id) + 1));
			strcpy(query.ids[i], state_i.id);
		}
	}
	return query;
}

t_bidib_id_list_query bidib_get_connected_track_outputs(void) {
	t_bidib_id_list_query query = {0, NULL};
	t_bidib_board board_ref;
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_ref = g_array_index(bidib_boards, t_bidib_board, i);
		if (board_ref.connected && board_ref.unique_id.class_id & (1 << 4)) {
			count++;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_boards->len; i++) {
			board_ref = g_array_index(bidib_boards, t_bidib_board, i);
			if (board_ref.connected && (board_ref.unique_id.class_id & (1 << 4))) {
				query.ids[current_index] = malloc(sizeof(char) * (board_ref.id->len) + 1);
				strcpy(query.ids[current_index], board_ref.id->str);
				current_index++;
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_track_outputs(void) {
	t_bidib_id_list_query query = {0, NULL};
	if (bidib_track_state.track_outputs->len > 0) {
		query.length = bidib_track_state.track_outputs->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_track_output_state state_i;
		for (size_t i = 0; i < bidib_track_state.track_outputs->len; i++) {
			state_i = g_array_index(bidib_track_state.track_outputs,
			                        t_bidib_track_output_state, i);
			query.ids[i] = malloc(sizeof(char) * (strlen(state_i.id) + 1));
			strcpy(query.ids[i], state_i.id);
		}
	}
	return query;
}

size_t bidib_get_point_state_index(const char *point) {
	t_bidib_board_accessory_state *accessory_state = NULL;
	for (size_t i = 0; i < bidib_track_state.points_board->len; i++) {
		accessory_state = &g_array_index(bidib_track_state.points_board,
		                                 t_bidib_board_accessory_state, i);
		if (!strcmp(accessory_state->id, point)) {
			return i;
		}
	}
	return -1;
}

size_t bidib_get_signal_state_index(const char *signal) {
	t_bidib_board_accessory_state *accessory_state = NULL;
	for (size_t i = 0; i < bidib_track_state.signals_board->len; i++) {
		accessory_state = &g_array_index(bidib_track_state.signals_board,
		                                 t_bidib_board_accessory_state, i);
		if (!strcmp(accessory_state->id, signal)) {
			return i;
		}
	}
	return -1;
}

size_t bidib_get_segment_state_index(const char *segment) {
	t_bidib_segment_state_intern *segment_state_i;
	for (size_t i = 0; i < bidib_track_state.segments->len; i++) {
		segment_state_i = &g_array_index(bidib_track_state.segments,
		                                 t_bidib_segment_state_intern, i);
		if (!strcmp(segment_state_i->id->str, segment)) {
			return i;
		}
	}
	return -1;
}

t_bidib_unified_accessory_state_query bidib_get_point_state(const char *point) {
	t_bidib_unified_accessory_state_query query = { .known = false };
	if (point == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_board_accessory_state *board_tmp = bidib_state_get_board_accessory_state_ref(point, true);
	if (board_tmp != NULL) {
		query.known = true;
		query.type = BIDIB_ACCESSORY_BOARD;
		char *state_id;
		if (board_tmp->data.state_id != NULL) {
			state_id = board_tmp->data.state_id;
		} else {
			state_id = "unknown";
		}
		query.dcc_accessory_state.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
		strcpy(query.dcc_accessory_state.state_id, state_id);
		query.board_accessory_state.state_value = board_tmp->data.state_value;
		query.board_accessory_state.execution_state = board_tmp->data.execution_state;
		query.board_accessory_state.wait_details = board_tmp->data.wait_details;
	}

	if (query.known == false) {
		t_bidib_dcc_accessory_state *dcc_tmp = bidib_state_get_dcc_accessory_state_ref(point, true);
		if (dcc_tmp != NULL) {
			query.known = true;
			query.type = BIDIB_ACCESSORY_DCC;
			char *state_id;
			if (dcc_tmp->data.state_id != NULL) {
				state_id = dcc_tmp->data.state_id;
			} else {
				state_id = "unknown";
			}
			query.dcc_accessory_state.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
			strcpy(query.dcc_accessory_state.state_id, state_id);
			query.dcc_accessory_state.state_value = dcc_tmp->data.state_value;
			query.dcc_accessory_state.coil_on = dcc_tmp->data.coil_on;
			query.dcc_accessory_state.time_unit = dcc_tmp->data.time_unit;
			query.dcc_accessory_state.switch_time = dcc_tmp->data.switch_time;
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_unified_accessory_state_query bidib_get_signal_state(const char *signal) {
	t_bidib_unified_accessory_state_query query = { .known = false };
	if (signal == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_board_accessory_state *board_tmp = bidib_state_get_board_accessory_state_ref(signal, false);
	if (board_tmp != NULL) {
		query.known = true;
		query.type = BIDIB_ACCESSORY_BOARD;
		char *state_id;
		if (board_tmp->data.state_id != NULL) {
			state_id = board_tmp->data.state_id;
		} else {
			state_id = "unknown";
		}
		query.dcc_accessory_state.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
		strcpy(query.dcc_accessory_state.state_id, state_id);
		query.board_accessory_state.state_value = board_tmp->data.state_value;
		query.board_accessory_state.execution_state = board_tmp->data.execution_state;
		query.board_accessory_state.wait_details = board_tmp->data.wait_details;
	}
	if (query.known == false) {
		t_bidib_dcc_accessory_state *dcc_tmp = bidib_state_get_dcc_accessory_state_ref(signal, false);
		if (dcc_tmp != NULL) {
			query.known = true;
			query.type = BIDIB_ACCESSORY_DCC;
			char *state_id;
			if (dcc_tmp->data.state_id != NULL) {
				state_id = dcc_tmp->data.state_id;
			} else {
				state_id = "unknown";
			}
			query.dcc_accessory_state.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
			strcpy(query.dcc_accessory_state.state_id, state_id);
			query.dcc_accessory_state.state_value = dcc_tmp->data.state_value;
			query.dcc_accessory_state.time_unit = dcc_tmp->data.time_unit;
			query.dcc_accessory_state.switch_time = dcc_tmp->data.switch_time;
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_peripheral_state_query bidib_get_peripheral_state(const char *peripheral) {
	t_bidib_peripheral_state_query query;
	query.available = false;
	if (peripheral == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_peripheral_state *tmp = bidib_state_get_peripheral_state_ref(peripheral);
	if (tmp != NULL) {
		query.available = true;
		char *state_id;
		if (tmp->data.state_id != NULL) {
			state_id = tmp->data.state_id;
		} else {
			state_id = "unknown";
		}
		query.data.state_id = malloc(sizeof(char) * (strlen(state_id) + 1));
		strcpy(query.data.state_id, state_id);
		query.data.state_value = tmp->data.state_value;
		query.data.time_unit = tmp->data.time_unit;
		query.data.wait = tmp->data.wait;
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_segment_state_query bidib_get_segment_state(const char *segment) {
	t_bidib_segment_state_query query;
	query.known = false;
	query.data.dcc_addresses = NULL;
	if (segment == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_segment_state_intern *tmp = bidib_state_get_segment_state_ref(segment);
	if (tmp != NULL) {
		query.known = true;
		query.data.occupied = tmp->occupied;
		query.data.confidence = tmp->confidence;
		query.data.power_consumption = tmp->power_consumption;
		query.data.dcc_address_cnt = tmp->dcc_addresses->len;
		query.data.dcc_addresses = malloc(
				sizeof(t_bidib_dcc_address) * tmp->dcc_addresses->len);
		memcpy(query.data.dcc_addresses, tmp->dcc_addresses->data,
		       sizeof(t_bidib_dcc_address) * tmp->dcc_addresses->len);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_booster_state_query bidib_get_booster_state(const char *booster) {
	t_bidib_booster_state_query query;
	query.known = false;
	if (booster == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_booster_state *tmp = bidib_state_get_booster_state_ref(booster);
	if (tmp != NULL) {
		query.known = true;
		query.data.power_state = tmp->data.power_state;
		query.data.power_state_simple = tmp->data.power_state_simple;
		query.data.power_consumption = tmp->data.power_consumption;
		query.data.voltage_known = tmp->data.voltage_known;
		query.data.voltage = tmp->data.voltage;
		query.data.temp_known = tmp->data.temp_known;
		query.data.temp_celsius = tmp->data.temp_celsius;
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_track_output_state_query bidib_get_track_output_state(const char *track_output) {
	t_bidib_track_output_state_query query;
	query.known = false;
	if (track_output == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_track_output_state *tmp = bidib_state_get_track_output_state_ref(track_output);
	if (tmp != NULL) {
		query.known = true;
		query.cs_state = tmp->cs_state;
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_trains(void) {
	t_bidib_id_list_query query = {0, NULL};
	if (bidib_trains->len > 0) {
		query.length = bidib_trains->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_train train_i;
		for (size_t i = 0; i < bidib_trains->len; i++) {
			train_i = g_array_index(bidib_trains, t_bidib_train, i);
			query.ids[i] = malloc(sizeof(char) * (train_i.id->len + 1));
			strcpy(query.ids[i], train_i.id->str);
		}
	}
	return query;
}

t_bidib_id_list_query bidib_get_trains_on_track(void) {
	t_bidib_id_list_query query = {0, NULL};
	size_t count = 0;
	pthread_mutex_lock(&bidib_state_track_mutex);
	for (size_t i = 0; i < bidib_track_state.trains->len; i++) {
		if (g_array_index(bidib_track_state.trains,
		                  t_bidib_train_state_intern, i).on_track) {
			count++;
		}
	}
	if (count > 0) {
		query.length = count;
		query.ids = malloc(sizeof(char *) * count);
		t_bidib_train_state_intern tmp;
		size_t current_index = 0;
		for (size_t i = 0; i < bidib_track_state.trains->len && current_index < count; i++) {
			tmp = g_array_index(bidib_track_state.trains, t_bidib_train_state_intern, i);
			if (tmp.on_track) {
				query.ids[current_index] = malloc(sizeof(char) * (tmp.id->len + 1));
				strcpy(query.ids[current_index], tmp.id->str);
				current_index++;
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_id_query bidib_get_train_id(t_bidib_dcc_address dcc_address) {
	t_bidib_id_query query = {false, NULL};
	t_bidib_train *train_i;
	for (size_t i = 0; i < bidib_trains->len; i++) {
		train_i = &g_array_index(bidib_trains, t_bidib_train, i);
		if (train_i->dcc_addr.addrl == dcc_address.addrl &&
		    train_i->dcc_addr.addrh == dcc_address.addrh) {
			query.known = true;
			query.id = malloc(sizeof(char) * (train_i->id->len + 1));
			strcpy(query.id, train_i->id->str);
			break;
		}
	}
	return query;
}

t_bidib_dcc_address_query bidib_get_train_dcc_addr(const char *train) {
	t_bidib_dcc_address_query query;
	query.known = false;
	if (train == NULL) {
		return query;
	}
	t_bidib_train *tmp = bidib_state_get_train_ref(train);
	if (tmp != NULL) {
		query.known = true;
		query.dcc_address.addrh = tmp->dcc_addr.addrh;
		query.dcc_address.addrl = tmp->dcc_addr.addrl;
		query.dcc_address.type = tmp->dcc_addr.type;
	}
	return query;
}

t_bidib_id_list_query bidib_get_train_peripherals(const char *train) {
	t_bidib_id_list_query query = {0, NULL};
	if (train == NULL) {
		return query;
	}
	t_bidib_train *tmp = bidib_state_get_train_ref(train);
	if (tmp != NULL) {
		query.length = tmp->peripherals->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_train_peripheral_mapping mapping_i;
		for (size_t i = 0; i < query.length; i++) {
			mapping_i = g_array_index(tmp->peripherals,
			                          t_bidib_train_peripheral_mapping, i);
			query.ids[i] = malloc(sizeof(char) * (mapping_i.id->len + 1));
			strcpy(query.ids[i], mapping_i.id->str);
		}
	}
	return query;
}

t_bidib_train_state_query bidib_get_train_state(const char *train) {
	t_bidib_train_state_query query;
	query.known = false;
	query.data.peripherals = NULL;
	if (train == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_trains_mutex);
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref(train);
	if (train_state != NULL) {
		query.known = true;
		query.data.on_track = train_state->on_track;
		query.data.orientation = train_state->orientation;
		query.data.set_speed_step = train_state->set_speed_step;
		query.data.detected_kmh_speed = train_state->detected_kmh_speed;
		query.data.ack = train_state->ack;
		query.data.peripheral_cnt = train_state->peripherals->len;
		query.data.peripherals = malloc(
				sizeof(t_bidib_train_peripheral_state) * query.data.peripheral_cnt);
		t_bidib_train_peripheral_state peripheral_state_i;
		for (size_t i = 0; i < query.data.peripheral_cnt; i++) {
			peripheral_state_i = g_array_index(
					train_state->peripherals, t_bidib_train_peripheral_state, i);
			query.data.peripherals[i].id = malloc(sizeof(char) * (strlen(
					peripheral_state_i.id) + 1));
			strcpy(query.data.peripherals[i].id, peripheral_state_i.id);
			query.data.peripherals[i].state = peripheral_state_i.state;
		}
		query.data.decoder_state = train_state->decoder_state;
	}
	pthread_mutex_unlock(&bidib_state_trains_mutex);
	return query;
}

t_bidib_train_peripheral_state_query bidib_get_train_peripheral_state(const char *train,
                                                                      const char *peripheral) {
	t_bidib_train_peripheral_state_query query = {false, 0x00};
	if (train == NULL || peripheral == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref(train);
	t_bidib_train_peripheral_state peripheral_state;
	if (train_state != NULL) {
		for (size_t i = 0; i < train_state->peripherals->len; i++) {
			peripheral_state = g_array_index(
					train_state->peripherals, t_bidib_train_peripheral_state, i);
			if (!strcmp(peripheral, peripheral_state.id)) {
				query.available = true;
				query.state = peripheral_state.state;
				break;
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

bool bidib_get_train_on_track(const char *train) {
	bool res = false;
	if (train == NULL) {
		return res;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref(train);
	if (train_state != NULL && train_state->on_track) {
		res = true;
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return res;
}

t_bidib_train_position_query bidib_get_train_position_intern(const char *train) {
	t_bidib_train_position_query query = {0, NULL, true};
	if (train == NULL) {
		return query;
	}
	t_bidib_train_state_intern *train_state_ref = bidib_state_get_train_state_ref(train);
	t_bidib_train *train_ref = bidib_state_get_train_ref(train);
	if (train_state_ref != NULL && train_ref != NULL) {
		size_t count = 0;
		t_bidib_segment_state_intern segment_state;
		for (size_t i = 0; i < bidib_track_state.segments->len; i++) {
			segment_state = g_array_index(bidib_track_state.segments,
			                              t_bidib_segment_state_intern, i);
			t_bidib_dcc_address dcc_address;
			for (size_t j = 0; j < segment_state.dcc_addresses->len; j++) {
				dcc_address = g_array_index(segment_state.dcc_addresses, t_bidib_dcc_address, j);
				if (train_ref->dcc_addr.addrh == dcc_address.addrh &&
				    train_ref->dcc_addr.addrl == dcc_address.addrl) {
					count++;
				}
			}
		}
		if (count > 0) {
			query.length = count;
			query.segments = malloc(sizeof(char *) * count);
			size_t current_index = 0;
			for (size_t i = 0; i < bidib_track_state.segments->len && current_index < count; i++) {
				segment_state = g_array_index(bidib_track_state.segments,
				                              t_bidib_segment_state_intern, i);
				t_bidib_dcc_address dcc_address;
				for (size_t j = 0; j < segment_state.dcc_addresses->len && current_index < count; j++) {
					dcc_address = g_array_index(segment_state.dcc_addresses, t_bidib_dcc_address, j);
					if (train_ref->dcc_addr.addrh == dcc_address.addrh &&
					    train_ref->dcc_addr.addrl == dcc_address.addrl) {
						query.segments[current_index] = malloc(
								sizeof(char) * (segment_state.id->len + 1));
						strcpy(query.segments[current_index], segment_state.id->str);
						query.orientation_is_left = (dcc_address.type == 0);
						current_index++;
					}
				}
			}
		}
	}
	return query;
}

t_bidib_train_position_query bidib_get_train_position(const char *train) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_position_query query = bidib_get_train_position_intern(train);
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_train_speed_step_query bidib_get_train_speed_step(const char *train) {
	t_bidib_train_speed_step_query query = {false, 0};
	if (train == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref(train);
	if (train_state != NULL && train_state->on_track) {
		query.known_and_avail = true;
		query.speed_step = train_state->set_speed_step;
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

t_bidib_train_speed_kmh_query bidib_get_train_speed_kmh(const char *train) {
	t_bidib_train_speed_kmh_query query = {false, 0};
	if (train == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_state_intern *train_state = bidib_state_get_train_state_ref(train);
	if (train_state != NULL && train_state->on_track) {
		query.known_and_avail = true;
		query.speed_kmh = train_state->detected_kmh_speed;
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return query;
}

static t_bidib_id_list_query bidib_get_accessory_aspects(const char *accessory, bool point) {
	t_bidib_id_list_query query = {0, NULL};
	if (accessory == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board_accessory_mapping *board_mapping =
			bidib_state_get_board_accessory_mapping_ref(accessory, point);
	if (board_mapping == NULL) {
		t_bidib_dcc_accessory_mapping *dcc_mapping =
				bidib_state_get_dcc_accessory_mapping_ref(accessory, point);
		if (dcc_mapping == NULL) {
			pthread_mutex_unlock(&bidib_state_boards_mutex);
			return query;
		} else {
			query.length = dcc_mapping->aspects->len;
			query.ids = malloc(sizeof(char *) * query.length);
			t_bidib_dcc_aspect *aspect_mapping;
			for (size_t i = 0; i < dcc_mapping->aspects->len; i++) {
				aspect_mapping = &g_array_index(dcc_mapping->aspects, t_bidib_dcc_aspect, i);
				query.ids[i] = malloc(sizeof(char) * (aspect_mapping->id->len + 1));
				strcpy(query.ids[i], aspect_mapping->id->str);
			}
		}
	} else {
		query.length = board_mapping->aspects->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_aspect *aspect_mapping;
		for (size_t i = 0; i < board_mapping->aspects->len; i++) {
			aspect_mapping = &g_array_index(board_mapping->aspects, t_bidib_aspect, i);
			query.ids[i] = malloc(sizeof(char) * (aspect_mapping->id->len + 1));
			strcpy(query.ids[i], aspect_mapping->id->str);
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

t_bidib_id_list_query bidib_get_point_aspects(const char *point) {
	return bidib_get_accessory_aspects(point, true);
}

t_bidib_id_list_query bidib_get_signal_aspects(const char *signal) {
	return bidib_get_accessory_aspects(signal, false);
}

t_bidib_id_list_query bidib_get_peripheral_aspects(const char *peripheral) {
	t_bidib_id_list_query query = {0, NULL};
	if (peripheral == NULL) {
		return query;
	}
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_peripheral_mapping *peripheral_mapping =
			bidib_state_get_peripheral_mapping_ref(peripheral);
	if (peripheral_mapping != NULL) {
		query.length = peripheral_mapping->aspects->len;
		query.ids = malloc(sizeof(char *) * query.length);
		t_bidib_aspect *aspect_mapping;
		for (size_t i = 0; i < peripheral_mapping->aspects->len; i++) {
			aspect_mapping = &g_array_index(peripheral_mapping->aspects, t_bidib_aspect, i);
			query.ids[i] = malloc(sizeof(char) * (aspect_mapping->id->len + 1));
			strcpy(query.ids[i], aspect_mapping->id->str);
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return query;
}

void bidib_free_track_state(t_bidib_track_state track_state) {
	for (size_t i = 0; i < track_state.points_board_count; i++) {
		bidib_state_free_single_board_accessory_state(track_state.points_board[i]);
		free(track_state.points_board[i].data.state_id);
	}
	free(track_state.points_board);

	for (size_t i = 0; i < track_state.points_dcc_count; i++) {
		bidib_state_free_single_dcc_accessory_state(track_state.points_dcc[i]);
		free(track_state.points_dcc[i].data.state_id);
	}
	free(track_state.points_dcc);

	for (size_t i = 0; i < track_state.signals_board_count; i++) {
		bidib_state_free_single_board_accessory_state(track_state.signals_board[i]);
		free(track_state.signals_board[i].data.state_id);
	}
	free(track_state.signals_board);

	for (size_t i = 0; i < track_state.signals_dcc_count; i++) {
		bidib_state_free_single_dcc_accessory_state(track_state.signals_dcc[i]);
		free(track_state.signals_dcc[i].data.state_id);
	}
	free(track_state.signals_dcc);

	for (size_t i = 0; i < track_state.peripherals_count; i++) {
		bidib_state_free_single_peripheral_state(track_state.peripherals[i]);
		free(track_state.peripherals[i].data.state_id);
	}
	free(track_state.peripherals);

	for (size_t i = 0; i < track_state.segments_count; i++) {
		bidib_state_free_single_segment_state(track_state.segments[i]);
	}
	free(track_state.segments);

	for (size_t i = 0; i < track_state.trains_count; i++) {
		bidib_state_free_single_train_state(track_state.trains[i]);
	}
	free(track_state.trains);

	for (size_t i = 0; i < track_state.booster_count; i++) {
		bidib_state_free_single_booster_state(track_state.booster[i]);
	}
	free(track_state.booster);

	for (size_t i = 0; i < track_state.track_outputs_count; i++) {
		bidib_state_free_single_track_output_state(track_state.track_outputs[i]);
	}
	free(track_state.track_outputs);
}

void bidib_free_unified_accessory_state_query(t_bidib_unified_accessory_state_query query) {
	if (query.type == BIDIB_ACCESSORY_BOARD) {
		if (query.board_accessory_state.state_id != NULL) {
			free(query.board_accessory_state.state_id);
		}
	} else {
		if (query.dcc_accessory_state.state_id != NULL) {
			free(query.dcc_accessory_state.state_id);
		}
	}
}

void bidib_free_peripheral_state_query(t_bidib_peripheral_state_query query) {
	if (query.data.state_id != NULL) {
		free(query.data.state_id);
	}
}

void bidib_free_segment_state_query(t_bidib_segment_state_query query) {
	if (query.data.dcc_addresses != NULL) {
		free(query.data.dcc_addresses);
	}
}

void bidib_free_id_list_query(t_bidib_id_list_query query) {
	if (query.ids != NULL) {
		for (size_t i = 0; i < query.length; i++) {
			free(query.ids[i]);
		}
		free(query.ids);
	}
}

void bidib_free_board_features_query(t_bidib_board_features_query query) {
	if (query.features != NULL) {
		free(query.features);
	}
}

void bidib_free_id_query(t_bidib_id_query query) {
	if (query.id != NULL) {
		free(query.id);
	}
}

void bidib_free_unique_id_list_query(t_bidib_unique_id_list_query query) {
	if (query.unique_ids != NULL) {
		free(query.unique_ids);
	}
}

void bidib_free_train_state_query(t_bidib_train_state_query query) {
	if (query.data.peripherals != NULL) {
		for (size_t i = 0; i < query.data.peripheral_cnt; i++) {
			free(query.data.peripherals[i].id);
		}
		free(query.data.peripherals);
	}
}

void bidib_free_train_position_query(t_bidib_train_position_query query) {
	if (query.segments != NULL) {
		for (size_t i = 0; i < query.length; i++) {
			free(query.segments[i]);
		}
		free(query.segments);
	}
}
