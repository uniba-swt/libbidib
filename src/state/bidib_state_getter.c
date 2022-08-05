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

#include "bidib_state_intern.h"
#include "../../include/bidib.h"
#include "../../include/definitions/bidib_definitions_custom.h"

t_bidib_board *bidib_state_get_board_ref(const char *board) {
	t_bidib_board *searched = NULL;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		if (!strcmp(board, g_array_index(bidib_boards, t_bidib_board, i).id->str)) {
			searched = &g_array_index(bidib_boards, t_bidib_board, i);
			break;
		}
	}
	return searched;
}

t_bidib_board *bidib_state_get_board_ref_by_nodeaddr(t_bidib_node_address node_address) {
	t_bidib_board *board_i = NULL;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (board_i->connected && board_i->node_addr.top == node_address.top &&
		    board_i->node_addr.sub == node_address.sub &&
		    board_i->node_addr.subsub == node_address.subsub) {
			return board_i;
		}
	}
	return NULL;
}

t_bidib_board *bidib_state_get_board_ref_by_uniqueid(t_bidib_unique_id_mod unique_id) {
	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (bidib_state_uids_equal(&unique_id, &board_i->unique_id)) {
			return board_i;
		}
	}
	return NULL;
}

t_bidib_booster_state *bidib_state_get_booster_state_ref(const char *booster) {
	t_bidib_booster_state *booster_state_i;
	for (size_t i = 0; i < bidib_track_state.boosters->len; i++) {
		booster_state_i = &g_array_index(bidib_track_state.boosters,
		                                 t_bidib_booster_state, i);
		if (!strcmp(booster, booster_state_i->id)) {
			return booster_state_i;
		}
	}
	return NULL;
}

t_bidib_track_output_state *bidib_state_get_track_output_state_ref(const char *track_output) {
	t_bidib_track_output_state *track_output_i;
	for (size_t i = 0; i < bidib_track_state.track_outputs->len; i++) {
		track_output_i = &g_array_index(bidib_track_state.track_outputs,
		                                t_bidib_track_output_state, i);
		if (!strcmp(track_output_i->id, track_output)) {
			return track_output_i;
		}
	}
	return NULL;
}

t_bidib_board_accessory_mapping *bidib_state_get_board_accessory_mapping_ref(
		const char *accessory, bool point) {
	t_bidib_board *tmp_board;
	t_bidib_board_accessory_mapping *mapping;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		tmp_board = &g_array_index(bidib_boards, t_bidib_board, i);
		if (point) {
			for (size_t j = 0; j < tmp_board->points_board->len; j++) {
				mapping = &g_array_index(tmp_board->points_board,
				                         t_bidib_board_accessory_mapping, j);
				if (!strcmp(mapping->id->str, accessory)) {
					return mapping;
				}
			}
		} else {
			for (size_t j = 0; j < tmp_board->signals_board->len; j++) {
				mapping = &g_array_index(tmp_board->signals_board,
				                         t_bidib_board_accessory_mapping, j);
				if (!strcmp(mapping->id->str, accessory)) {
					return mapping;
				}
			}
		}
	}
	return NULL;
}

t_bidib_board_accessory_mapping *bidib_state_get_board_accessory_mapping_ref_by_number(
		t_bidib_node_address node_address, uint8_t number, bool *point) {
	t_bidib_board *sender = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (sender == NULL) {
		return NULL;
	}
	t_bidib_board_accessory_mapping *mapping;
	for (size_t i = 0; i < sender->points_board->len; i++) {
		mapping = &g_array_index(sender->points_board,
		                         t_bidib_board_accessory_mapping, i);
		if (mapping->number == number) {
			*point = true;
			return mapping;
		}
	}
	for (size_t i = 0; i < sender->signals_board->len; i++) {
		mapping = &g_array_index(sender->signals_board,
		                         t_bidib_board_accessory_mapping, i);
		if (mapping->number == number) {
			*point = false;
			return mapping;
		}
	}
	return NULL;
}

t_bidib_board_accessory_state *bidib_state_get_board_accessory_state_ref(const char *accessory,
                                                                         bool point) {
	t_bidib_board_accessory_state *accessory_state = NULL;
	if (point) {
		for (size_t i = 0; i < bidib_track_state.points_board->len; i++) {
			accessory_state = &g_array_index(bidib_track_state.points_board,
			                                 t_bidib_board_accessory_state, i);
			if (!strcmp(accessory_state->id, accessory)) {
				return accessory_state;
			}
		}
	} else {
		for (size_t i = 0; i < bidib_track_state.signals_board->len; i++) {
			accessory_state = &g_array_index(bidib_track_state.signals_board,
			                                 t_bidib_board_accessory_state, i);
			if (!strcmp(accessory_state->id, accessory)) {
				return accessory_state;
			}
		}
	}
	return NULL;
}

t_bidib_dcc_accessory_mapping *bidib_state_get_dcc_accessory_mapping_ref(
		const char *accessory, bool point) {
	t_bidib_board *tmp_board;
	t_bidib_dcc_accessory_mapping *mapping;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		tmp_board = &g_array_index(bidib_boards, t_bidib_board, i);
		if (point) {
			for (size_t j = 0; j < tmp_board->points_dcc->len; j++) {
				mapping = &g_array_index(tmp_board->points_dcc,
				                         t_bidib_dcc_accessory_mapping, j);
				if (!strcmp(mapping->id->str, accessory)) {
					return mapping;
				}
			}
		} else {
			for (size_t j = 0; j < tmp_board->signals_dcc->len; j++) {
				mapping = &g_array_index(tmp_board->signals_dcc,
				                         t_bidib_dcc_accessory_mapping, j);
				if (!strcmp(mapping->id->str, accessory)) {
					return mapping;
				}
			}
		}
	}
	return NULL;
}

t_bidib_dcc_accessory_mapping *bidib_state_get_dcc_accessory_mapping_ref_by_dccaddr(
		t_bidib_node_address node_address, t_bidib_dcc_address dcc_address, bool *point) {
	t_bidib_board *board = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (board != NULL) {
		t_bidib_dcc_accessory_mapping *mapping;
		for (size_t i = 0; i < board->points_dcc->len; i++) {
			mapping = &g_array_index(board->points_dcc, t_bidib_dcc_accessory_mapping, i);
			if (mapping->dcc_addr.addrh == dcc_address.addrh &&
			    mapping->dcc_addr.addrl == dcc_address.addrl) {
				*point = true;
				return mapping;
			}
		}
		for (size_t i = 0; i < board->signals_dcc->len; i++) {
			mapping = &g_array_index(board->signals_dcc, t_bidib_dcc_accessory_mapping, i);
			if (mapping->dcc_addr.addrh == dcc_address.addrh &&
			    mapping->dcc_addr.addrl == dcc_address.addrl) {
				*point = false;
				return mapping;
			}
		}
	}
	return NULL;
}

t_bidib_dcc_accessory_state *bidib_state_get_dcc_accessory_state_ref(const char *accessory,
                                                                     bool point) {
	t_bidib_dcc_accessory_state *accessory_state;
	if (point) {
		for (size_t i = 0; i < bidib_track_state.points_dcc->len; i++) {
			accessory_state = &g_array_index(bidib_track_state.points_dcc,
			                                 t_bidib_dcc_accessory_state, i);
			if (!strcmp(accessory_state->id, accessory)) {
				return accessory_state;
			}
		}
	} else {
		for (size_t i = 0; i < bidib_track_state.signals_dcc->len; i++) {
			accessory_state = &g_array_index(bidib_track_state.signals_dcc,
			                                 t_bidib_dcc_accessory_state, i);
			if (!strcmp(accessory_state->id, accessory)) {
				return accessory_state;
			}
		}
	}
	return NULL;
}

t_bidib_peripheral_mapping *bidib_state_get_peripheral_mapping_ref(const char *peripheral) {
	t_bidib_board *tmp_board;
	t_bidib_peripheral_mapping *mapping;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		tmp_board = &g_array_index(bidib_boards, t_bidib_board, i);
		for (size_t j = 0; j < tmp_board->peripherals->len; j++) {
			mapping = &g_array_index(tmp_board->peripherals,
			                         t_bidib_peripheral_mapping, j);
			if (!strcmp(mapping->id->str, peripheral)) {
				return mapping;
			}
		}
	}
	return NULL;
}

t_bidib_peripheral_mapping *bidib_state_get_peripheral_mapping_ref_by_port(
		t_bidib_node_address node_address, t_bidib_peripheral_port port) {
	t_bidib_board *board = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (board != NULL) {
		t_bidib_peripheral_mapping *mapping;
		for (size_t i = 0; i < board->peripherals->len; i++) {
			mapping = &g_array_index(
					board->peripherals, t_bidib_peripheral_mapping, i);
			if (mapping->port.port0 == port.port0 &&
			    mapping->port.port1 == port.port1) {
				return mapping;
			}
		}
	}
	return NULL;
}

t_bidib_peripheral_state *bidib_state_get_peripheral_state_ref(const char *peripheral) {
	t_bidib_peripheral_state *peripheral_state_i;
	for (size_t i = 0; i < bidib_track_state.peripherals->len; i++) {
		peripheral_state_i = &g_array_index(bidib_track_state.peripherals,
		                                    t_bidib_peripheral_state, i);
		if (!strcmp(peripheral_state_i->id, peripheral)) {
			return peripheral_state_i;
		}
	}
	return NULL;
}

t_bidib_segment_state_intern *bidib_state_get_segment_state_ref(const char *segment) {
	if (segment == NULL) {
		return NULL;
	}
	t_bidib_segment_state_intern *segment_state_i;
	for (size_t i = 0; i < bidib_track_state.segments->len; i++) {
		segment_state_i = &g_array_index(bidib_track_state.segments,
		                                 t_bidib_segment_state_intern, i);
		if (!strcmp(segment_state_i->id->str, segment)) {
			return segment_state_i;
		}
	}
	return NULL;
}

t_bidib_segment_state_intern bidib_state_get_segment_state(
		const t_bidib_segment_state_intern *segment_state) {
	t_bidib_segment_state_intern query;
	query.id = g_string_new(segment_state->id->str);
	query.length = g_string_new(segment_state->length->str);
	query.occupied = segment_state->occupied;
	query.confidence = segment_state->confidence;
	query.power_consumption = segment_state->power_consumption;
	query.dcc_addresses = g_array_new(FALSE, FALSE, 
			sizeof(t_bidib_dcc_address));
	for (size_t i = 0; i < segment_state->dcc_addresses->len; i++) {
		t_bidib_dcc_address dcc_address = g_array_index(segment_state->dcc_addresses, t_bidib_dcc_address, i);
		g_array_append_val(query.dcc_addresses, dcc_address);
	}

	return query;
}

t_bidib_segment_state_intern *bidib_state_get_segment_state_ref_by_nodeaddr(
		t_bidib_node_address node_address, uint8_t number) {
	pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
	t_bidib_board *board = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (board != NULL) {
		t_bidib_segment_mapping mapping_i;
		for (size_t i = 0; i < board->segments->len; i++) {
			mapping_i = g_array_index(board->segments, t_bidib_segment_mapping, i);
			if (mapping_i.addr == number) {
				pthread_rwlock_unlock(&bidib_state_boards_rwlock);
				return bidib_state_get_segment_state_ref(mapping_i.id->str);
			}
		}
	}
	pthread_rwlock_unlock(&bidib_state_boards_rwlock);
	return NULL;
}

t_bidib_train *bidib_state_get_train_ref(const char *train) {
	for (size_t i = 0; i < bidib_trains->len; i++) {
		t_bidib_train *train_i = &g_array_index(bidib_trains, t_bidib_train, i);
		if (!strcmp(train, train_i->id->str)) {
			return train_i;
		}
	}
	return NULL;
}

t_bidib_train_state_intern *bidib_state_get_train_state_ref(const char *train) {
	t_bidib_train_state_intern *train_state_i;
	for (size_t i = 0; i < bidib_track_state.trains->len; i++) {
		train_state_i = &g_array_index(bidib_track_state.trains,
		                               t_bidib_train_state_intern, i);
		if (!strcmp(train, train_state_i->id->str)) {
			return train_state_i;
		}
	}
	return NULL;
}

t_bidib_train_state_intern *bidib_state_get_train_state_ref_by_dccaddr(
		t_bidib_dcc_address dcc_address) {
	bool found = false;
	for (size_t i = 0; i < bidib_trains->len; i++) {
		const t_bidib_train *train_i = &g_array_index(bidib_trains, t_bidib_train, i);
		// ignore orientation 
		if (dcc_address.addrl == train_i->dcc_addr.addrl &&
		    (dcc_address.addrh & 0x3F) == train_i->dcc_addr.addrh) {
			return bidib_state_get_train_state_ref(train_i->id->str);
		}
	}
	return NULL;
}

t_bidib_train_peripheral_state *bidib_state_get_train_peripheral_state_by_bit(
		t_bidib_train_state_intern *train_state, uint8_t bit) {
	t_bidib_train *train = bidib_state_get_train_ref(train_state->id->str);
	bool found = false;
	if (train != NULL) {
		t_bidib_train_peripheral_mapping *mapping_i = NULL;
		for (size_t i = 0; i < train->peripherals->len; i++) {
			mapping_i = &g_array_index(train->peripherals,
			                           t_bidib_train_peripheral_mapping, i);
			if (mapping_i->bit == bit) {
				found = true;
				break;
			}
		}
		if (found) {
			t_bidib_train_peripheral_state *peripheral_state;
			for (size_t i = 0; i < train_state->peripherals->len; i++) {
				peripheral_state = &g_array_index(train_state->peripherals,
				                                  t_bidib_train_peripheral_state, i);
				if (!strcmp(mapping_i->id->str, peripheral_state->id)) {
					return peripheral_state;
				}
			}
		}
	}
	return NULL;
}

t_bidib_booster_state *bidib_state_get_booster_state_ref_by_nodeaddr(
		t_bidib_node_address node_address) {
	t_bidib_booster_state *booster_state = NULL;
	pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
	t_bidib_board *sender = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (sender != NULL) {
		booster_state = bidib_state_get_booster_state_ref(sender->id->str);
	}
	pthread_rwlock_unlock(&bidib_state_boards_rwlock);
	return booster_state;
}

t_bidib_track_output_state *bidib_state_get_track_output_state_ref_by_nodeaddr(
		t_bidib_node_address node_address) {
	t_bidib_track_output_state *track_output_state = NULL;
	pthread_rwlock_rdlock(&bidib_state_boards_rwlock);
	t_bidib_board *sender = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (sender != NULL) {
		track_output_state = bidib_state_get_track_output_state_ref(sender->id->str);
	}
	pthread_rwlock_unlock(&bidib_state_boards_rwlock);
	return track_output_state;
}
