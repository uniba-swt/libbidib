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

#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdint.h>

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../parser/bidib_config_parser_intern.h"
#include "bidib_state_intern.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../../include/bidib.h"
#include "bidib_state_getter_intern.h"
#include "../../include/definitions/bidib_definitions_custom.h"
#include "../highlevel/bidib_highlevel_intern.h"


pthread_mutex_t bidib_state_trains_mutex;
pthread_mutex_t bidib_state_track_mutex;
pthread_mutex_t bidib_state_boards_mutex;

t_bidib_state_initial_values bidib_initial_values;
t_bidib_track_state_intern bidib_track_state;
GArray *bidib_boards = NULL;
GArray *bidib_trains = NULL;


int bidib_state_init(const char *config_dir) {
	bidib_initial_values.points = g_array_sized_new(FALSE, FALSE,
	                                                sizeof(t_bidib_state_initial_value), 32);
	bidib_initial_values.signals = g_array_sized_new(FALSE, FALSE,
	                                                 sizeof(t_bidib_state_initial_value), 32);
	bidib_initial_values.peripherals = g_array_sized_new(FALSE, FALSE,
	                                                     sizeof(t_bidib_state_initial_value), 32);
	bidib_initial_values.trains = g_array_sized_new(FALSE, FALSE,
	                                                sizeof(t_bidib_state_train_initial_value), 16);

	bidib_track_state.points_board = g_array_sized_new(FALSE, FALSE,
	                                                   sizeof(t_bidib_board_accessory_state), 16);
	bidib_track_state.points_dcc = g_array_sized_new(FALSE, FALSE,
	                                                 sizeof(t_bidib_dcc_accessory_state), 16);
	bidib_track_state.signals_board = g_array_sized_new(FALSE, FALSE,
	                                                    sizeof(t_bidib_board_accessory_state), 16);
	bidib_track_state.signals_dcc = g_array_sized_new(FALSE, FALSE,
	                                                  sizeof(t_bidib_dcc_accessory_state), 16);
	bidib_track_state.peripherals = g_array_sized_new(FALSE, FALSE,
	                                                  sizeof(t_bidib_peripheral_state), 32);
	bidib_track_state.segments = g_array_sized_new(FALSE, FALSE,
	                                               sizeof(t_bidib_segment_state_intern), 256);
	bidib_track_state.trains = g_array_sized_new(FALSE, FALSE,
	                                             sizeof(t_bidib_train_state_intern), 16);
	bidib_track_state.boosters = g_array_sized_new(FALSE, FALSE,
	                                               sizeof(t_bidib_booster_state), 8);
	bidib_track_state.track_outputs = g_array_sized_new(FALSE, FALSE,
	                                                    sizeof(t_bidib_track_output_state), 8);

	bidib_boards = g_array_sized_new(FALSE, FALSE, sizeof(t_bidib_board), 32);
	bidib_trains = g_array_sized_new(FALSE, FALSE, sizeof(t_bidib_train), 16);

	if (bidib_config_parse(config_dir)) {
		return 1;
	}
	return 0;
}

static bool bidib_state_query_nodetab(t_bidib_node_address node_address,
                                      GQueue *sub_iface_queue) {
	uint8_t *message;
	uint8_t node_count = 0;

	bidib_send_nodetab_getall(node_address, 0);
	bidib_flush();
	while (true) {
		message = bidib_read_intern_message();
		if (message == NULL) {
			syslog_libbidib(LOG_WARNING, "%s", "Awaiting NODETAB_GET_ALL answer");
			usleep(50000);
		} else if (bidib_extract_msg_type(message) == MSG_NODETAB_COUNT) {
			node_count = message[bidib_first_data_byte_index(message)];
			free(message);
			break;
		} else {
			free(message);
		}
	}

	for (size_t i = 0; i < node_count; i++) {
		bidib_send_nodetab_getnext(node_address, 0);
	}
	bidib_flush();

	t_bidib_board *board_i;
	t_bidib_unique_id_mod unique_id_i;
	t_bidib_node_address node_address_i;
	int first_data_byte;
	uint8_t local_node_addr;
	size_t i = 0;
	while (i < node_count) {
		message = bidib_read_intern_message();
		if (message == NULL) {
			syslog_libbidib(LOG_WARNING, "%s", "Awaiting NODETAB_GET_NEXT answer");
			usleep(50000);
		} else if (bidib_extract_msg_type(message) == MSG_NODETAB_COUNT) {
			free(message);
			return true;
		} else if (bidib_extract_msg_type(message) != MSG_NODETAB) {
			free(message);
		} else {
			first_data_byte = bidib_first_data_byte_index(message);
			local_node_addr = message[first_data_byte + 1];
			unique_id_i.class_id = message[first_data_byte + 2];
			unique_id_i.class_id_ext = message[first_data_byte + 3];
			unique_id_i.vendor_id = message[first_data_byte + 4];
			unique_id_i.product_id1 = message[first_data_byte + 5];
			unique_id_i.product_id2 = message[first_data_byte + 6];
			unique_id_i.product_id3 = message[first_data_byte + 7];
			unique_id_i.product_id4 = message[first_data_byte + 8];

			node_address_i = node_address;
			if (node_address_i.top == 0x00) {
				node_address_i.top = local_node_addr;
			} else if (node_address_i.sub == 0x00) {
				node_address_i.sub = local_node_addr;
			} else {
				node_address_i.subsub = local_node_addr;
			}

			board_i = bidib_state_get_board_ref_by_uniqueid(unique_id_i);
			if (board_i == NULL) {
				syslog_libbidib(LOG_ERR, "No board configured for unique id 0x%02x%02x%02x%02x%02x%02x%02x",
				                unique_id_i.class_id, unique_id_i.class_id_ext, unique_id_i.vendor_id,
				                unique_id_i.product_id1, unique_id_i.product_id2, unique_id_i.product_id3,
				                unique_id_i.product_id4);
			} else {
				board_i->connected = true;
				board_i->node_addr = node_address_i;
				syslog_libbidib(LOG_INFO, "Board %s connected with address 0x%02x 0x%02x 0x%02x 0x00",
				                board_i->id->str, board_i->node_addr.top, board_i->node_addr.sub,
				                board_i->node_addr.subsub);
			}

			if (i > 0 && unique_id_i.class_id & (1 << 7)) {
				// add node to queue if it is an interface
				t_bidib_node_address *sub_iface_addr = malloc(sizeof(t_bidib_node_address));
				*sub_iface_addr = node_address_i;
				g_queue_push_tail(sub_iface_queue, sub_iface_addr);
			}

			free(message);
			i++;
		}
	}
	return false;
}

void bidib_state_init_allocation_table(void) {
	pthread_mutex_lock(&bidib_state_boards_mutex);

	t_bidib_node_address interface = {0x00, 0x00, 0x00};
	GQueue *sub_iface_queue = g_queue_new();
	t_bidib_node_address *sub_interface;
	bool reset;
	while (true) {
		reset = bidib_state_query_nodetab(interface, sub_iface_queue);
		if (!reset) {
			while (!reset && !g_queue_is_empty(sub_iface_queue)) {
				sub_interface = g_queue_pop_head(sub_iface_queue);
				reset = bidib_state_query_nodetab(*sub_interface, sub_iface_queue);
				free(sub_interface);
			}
			break;
		}
		while (!g_queue_is_empty(sub_iface_queue)) {
			sub_interface = g_queue_pop_head(sub_iface_queue);
			free(sub_interface);
		}
	}
	g_queue_free(sub_iface_queue);

	pthread_mutex_unlock(&bidib_state_boards_mutex);
}

void bidib_state_query_occupancy(void) {
	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (board_i->connected && (board_i->unique_id.class_id & (1 << 4))) {
			uint8_t max_seg_addr = 0x00;
			t_bidib_segment_mapping *seg_mapping;
			for (size_t j = 0; j < board_i->segments->len; j++) {
				seg_mapping = &g_array_index(board_i->segments, t_bidib_segment_mapping, j);
				if (seg_mapping->addr > max_seg_addr) {
					max_seg_addr = seg_mapping->addr;
				}
			}
			bidib_send_bm_get_range(board_i->node_addr, 0, (uint8_t) (((max_seg_addr / 8) + 1) * 8), 0);
			bidib_send_bm_addr_get_range(board_i->node_addr, 0, (uint8_t) (max_seg_addr + 1), 0);
		}
	}
}

void bidib_state_set_board_features(void) {
	pthread_mutex_lock(&bidib_state_boards_mutex);

	t_bidib_board *board_i;
	for (size_t i = 0; i < bidib_boards->len; i++) {
		board_i = &g_array_index(bidib_boards, t_bidib_board, i);
		if (board_i->connected) {
			t_bidib_board_feature *feature_j;
			for (size_t j = 0; j < board_i->features->len; j++) {
				feature_j = &g_array_index(board_i->features, t_bidib_board_feature, j);
				bidib_send_feature_set(board_i->node_addr,
				                       feature_j->number, feature_j->value, 0);
			}
			bidib_flush();
			uint8_t *message;
			for (size_t j = 0; j < board_i->features->len; j++) {
				while (true) {
					message = bidib_read_intern_message();
					if (message == NULL) {
						usleep(50000);
					} else if (bidib_extract_msg_type(message) == MSG_FEATURE) {
						int first_data_byte = bidib_first_data_byte_index(message);
						for (size_t k = 0; k < board_i->features->len; k++) {
							feature_j = &g_array_index(board_i->features,
							                           t_bidib_board_feature, k);
							if (feature_j->number == message[first_data_byte]) {
								if (feature_j->value != message[first_data_byte + 1]) {
									syslog_libbidib(LOG_ERR, 
									                "Feature 0x%02x for board 0x%02x 0x%02x 0x%02x "
									                "0x00 could not be set", feature_j->number,
									                board_i->node_addr.top, board_i->node_addr.sub,
									                board_i->node_addr.subsub);
								} else {
									syslog_libbidib(LOG_INFO, 
									                "Feature 0x%02x for board 0x%02x 0x%02x 0x%02x "
									                "0x00 was set to 0x%02x", feature_j->number,
									                board_i->node_addr.top, board_i->node_addr.sub,
									                board_i->node_addr.subsub, feature_j->value);
								}
								break;
							}
						}
						free(message);
						break;
					} else {
						free(message);
					}
				}
			}
		}
	}

	pthread_mutex_unlock(&bidib_state_boards_mutex);
}

void bidib_state_set_initial_values(void) {
	t_bidib_state_initial_value *initial_value;
	t_bidib_state_train_initial_value *train_initial_value;
	t_bidib_track_output_state *track_output_state;

	for (size_t i = 0; i < bidib_initial_values.points->len; i++) {
		initial_value = &g_array_index(
				bidib_initial_values.points, t_bidib_state_initial_value, i);
		bidib_switch_point(initial_value->id->str, initial_value->value->str);
	}

	for (size_t i = 0; i < bidib_initial_values.signals->len; i++) {
		initial_value = &g_array_index(
				bidib_initial_values.signals, t_bidib_state_initial_value, i);
		bidib_set_signal(initial_value->id->str, initial_value->value->str);
	}

	for (size_t i = 0; i < bidib_initial_values.peripherals->len; i++) {
		initial_value = &g_array_index(
				bidib_initial_values.peripherals, t_bidib_state_initial_value, i);
		bidib_set_peripheral(initial_value->id->str, initial_value->value->str);
	}

	for (size_t i = 0; i < bidib_initial_values.trains->len; i++) {
		train_initial_value = &g_array_index(
				bidib_initial_values.trains, t_bidib_state_train_initial_value, i);
		for (size_t j = 0; j < bidib_track_state.track_outputs->len; j++) {
			track_output_state = &g_array_index(
					bidib_track_state.track_outputs, t_bidib_track_output_state, j);
			bidib_set_train_peripheral(train_initial_value->train->str,
			                           train_initial_value->id->str, train_initial_value->value,
			                           track_output_state->id);
			bidib_set_train_speed(train_initial_value->train->str, 0, track_output_state->id);
		}
	}
	bidib_flush();
}

bool bidib_state_uids_equal(t_bidib_unique_id_mod *uid1,
                            t_bidib_unique_id_mod *uid2) {
	if (uid1->class_id == uid2->class_id &&
	    uid1->class_id_ext == uid2->class_id_ext &&
	    uid1->vendor_id == uid2->vendor_id &&
	    uid1->product_id1 == uid2->product_id1 &&
	    uid1->product_id2 == uid2->product_id2 &&
	    uid1->product_id3 == uid2->product_id3 &&
	    uid1->product_id4 == uid2->product_id4) {
		return true;
	} else {
		return false;
	}
}

t_bidib_booster_power_state_simple bidib_booster_normal_to_simple(
		t_bidib_booster_power_state state) {
	switch (state) {
		case BIDIB_BSTR_ON:
		case BIDIB_BSTR_ON_LIMIT:
		case BIDIB_BSTR_ON_HOT:
		case BIDIB_BSTR_ON_HERE:
			return BIDIB_BSTR_SIMPLE_ON;
		case BIDIB_BSTR_OFF:
		case BIDIB_BSTR_OFF_GO_REQ:
		case BIDIB_BSTR_OFF_HERE:
		case BIDIB_BSTR_OFF_NO_DCC:
		case BIDIB_BSTR_OFF_NOPOWER:
			return BIDIB_BSTR_SIMPLE_OFF;
		case BIDIB_BSTR_ON_STOP_REQ:
		case BIDIB_BSTR_OFF_SHORT:
		case BIDIB_BSTR_OFF_HOT:
		default:
			return BIDIB_BSTR_SIMPLE_ERROR;
	}
}

int bidib_dcc_speed_to_lib_format(uint8_t speed) {
	int speed_step = (uint8_t) (speed & 0x7F);          // exclude direction bit
	if (speed_step == 0 || speed_step == 1) {           // no difference between stop
		return 0;                                       // and emergency stop
	} else {
		speed_step--;                                   // let it start at 1 not 2
		if ((speed & (1 << 7)) == 0) {                  // check direction bit
			speed_step *= -1;
		}
		return speed_step;
	}
}

uint8_t bidib_lib_speed_to_dcc_format(int speed) {
	uint8_t bidib_speed = 0x00;
	if (speed >= 0) {
		bidib_speed = (uint8_t) (bidib_speed | (1 << 7));
	} else {
		speed = speed * -1;
	}
	bidib_speed = (uint8_t) (bidib_speed | speed);
	if (speed != 0) {
		bidib_speed++;
	}
	return bidib_speed;
}

t_bidib_bm_confidence_level bidib_bm_confidence_to_level(t_bidib_segment_state_confidence confidence) {
	if (!confidence.conf_void && !confidence.freeze && !confidence.nosignal) {
		return BIDIB_BM_CONFIDENCE_ACCURATE;
	} else if (!confidence.conf_void && !confidence.freeze && confidence.nosignal) {
		return BIDIB_BM_CONFIDENCE_SUBSTITUTED;
	} else if (!confidence.conf_void && confidence.freeze && confidence.nosignal) {
		return BIDIB_BM_CONFIDENCE_STALE;
	}
	return BIDIB_BM_CONFIDENCE_INVALID;
}

bool bidib_state_add_board(t_bidib_board board) {
	bool error = false;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	if (bidib_state_get_board_ref(board.id->str) != NULL ||
	    bidib_state_get_board_ref_by_uniqueid(board.unique_id) != NULL) {
		error = true;
	} else {
		g_array_append_val(bidib_boards, board);
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	return error;
}

bool bidib_state_dcc_addr_in_use(t_bidib_dcc_address dcc_address) {
	t_bidib_board *tmp_board;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	for (size_t i = 0; i < bidib_boards->len; i++) {
		tmp_board = &g_array_index(bidib_boards, t_bidib_board, i);

		t_bidib_dcc_accessory_mapping *tmp_dcc_mapping;
		for (size_t j = 0; j < tmp_board->points_dcc->len; j++) {
			tmp_dcc_mapping = &g_array_index(tmp_board->points_dcc,
			                                 t_bidib_dcc_accessory_mapping, j);
			if (tmp_dcc_mapping->dcc_addr.addrl == dcc_address.addrl &&
			    tmp_dcc_mapping->dcc_addr.addrh == dcc_address.addrh) {
				pthread_mutex_unlock(&bidib_state_boards_mutex);
				return true;
			}
		}

		for (size_t j = 0; j < tmp_board->signals_dcc->len; j++) {
			tmp_dcc_mapping = &g_array_index(tmp_board->signals_dcc,
			                                 t_bidib_dcc_accessory_mapping, j);
			if (tmp_dcc_mapping->dcc_addr.addrl == dcc_address.addrl &&
			    tmp_dcc_mapping->dcc_addr.addrh == dcc_address.addrh) {
				pthread_mutex_unlock(&bidib_state_boards_mutex);
				return true;
			}
		}
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);

	t_bidib_train *tmp_train;
	for (size_t i = 0; i < bidib_trains->len; i++) {
		tmp_train = &g_array_index(bidib_trains, t_bidib_train, i);
		if (tmp_train->dcc_addr.addrl == dcc_address.addrl &&
		    tmp_train->dcc_addr.addrh == dcc_address.addrh) {
			return true;
		}
	}
	return false;
}

bool bidib_state_add_train(t_bidib_train train) {
	bool error = bidib_state_dcc_addr_in_use(train.dcc_addr);
	if (!error) {
		if (bidib_state_get_train_ref(train.id->str) != NULL) {
			error = true;
		} else {
			g_array_append_val(bidib_trains, train);
		}
	}
	return error;
}

static bool bidib_state_point_exists(char *id) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	for (size_t i = 0; i < bidib_track_state.points_board->len; i++) {
		if (!strcmp(id, g_array_index(bidib_track_state.points_board,
		                              t_bidib_board_accessory_state, i).id)) {
			pthread_mutex_unlock(&bidib_state_track_mutex);
			return true;
		}
	}
	for (size_t i = 0; i < bidib_track_state.points_dcc->len; i++) {
		if (!strcmp(id, g_array_index(bidib_track_state.points_dcc,
		                              t_bidib_dcc_accessory_state, i).id)) {
			pthread_mutex_unlock(&bidib_state_track_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

static bool bidib_state_signal_exists(char *id) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	for (size_t i = 0; i < bidib_track_state.signals_board->len; i++) {
		if (!strcmp(id, g_array_index(bidib_track_state.signals_board,
		                              t_bidib_board_accessory_state, i).id)) {
			pthread_mutex_unlock(&bidib_state_track_mutex);
			return true;
		}
	}
	for (size_t i = 0; i < bidib_track_state.signals_dcc->len; i++) {
		if (!strcmp(id, g_array_index(bidib_track_state.signals_dcc,
		                              t_bidib_dcc_accessory_state, i).id)) {
			pthread_mutex_unlock(&bidib_state_track_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

static bool bidib_state_peripheral_exists(char *id) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	for (size_t i = 0; i < bidib_track_state.peripherals->len; i++) {
		if (!strcmp(id, g_array_index(bidib_track_state.peripherals,
		                              t_bidib_peripheral_state, i).id)) {
			pthread_mutex_unlock(&bidib_state_track_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

static bool bidib_state_segment_exists(char *id) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	for (size_t i = 0; i < bidib_track_state.segments->len; i++) {
		if (!strcmp(id, g_array_index(bidib_track_state.segments,
		                              t_bidib_segment_state_intern, i).id->str)) {
			pthread_mutex_unlock(&bidib_state_track_mutex);
			return true;
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

void bidib_state_add_booster(t_bidib_booster_state booster_state) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.boosters, booster_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_add_track_output(t_bidib_track_output_state track_output_state) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.track_outputs, track_output_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_add_train_state(t_bidib_train_state_intern train_state) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	if (bidib_state_get_train_state_ref(train_state.id->str) == NULL) {
		g_array_append_val(bidib_track_state.trains, train_state);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

bool bidib_state_add_board_point_state(t_bidib_board_accessory_state point_state) {
	if (bidib_state_point_exists(point_state.id)) {
		return true;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.points_board, point_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

bool bidib_state_add_board_signal_state(t_bidib_board_accessory_state signal_state) {
	if (bidib_state_signal_exists(signal_state.id)) {
		return true;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.signals_board, signal_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

bool bidib_state_add_dcc_point_state(t_bidib_dcc_accessory_state point_state,
                                     t_bidib_dcc_address dcc_address) {
	if (bidib_state_point_exists(point_state.id) ||
	    bidib_state_dcc_addr_in_use(dcc_address)) {
		return true;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.points_dcc, point_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

bool bidib_state_add_dcc_signal_state(t_bidib_dcc_accessory_state signal_state,
                                      t_bidib_dcc_address dcc_address) {
	if (bidib_state_signal_exists(signal_state.id) ||
	    bidib_state_dcc_addr_in_use(dcc_address)) {
		return true;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.signals_dcc, signal_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

bool bidib_state_add_peripheral_state(t_bidib_peripheral_state peripheral_state) {
	if (bidib_state_peripheral_exists(peripheral_state.id)) {
		return true;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.peripherals, peripheral_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

bool bidib_state_add_segment_state(t_bidib_segment_state_intern segment_state) {
	if (bidib_state_segment_exists(segment_state.id->str)) {
		return true;
	}
	pthread_mutex_lock(&bidib_state_track_mutex);
	g_array_append_val(bidib_track_state.segments, segment_state);
	pthread_mutex_unlock(&bidib_state_track_mutex);
	return false;
}

void bidib_state_add_initial_point_value(t_bidib_state_initial_value value) {
	g_array_append_val(bidib_initial_values.points, value);
}

void bidib_state_add_initial_signal_value(t_bidib_state_initial_value value) {
	g_array_append_val(bidib_initial_values.signals, value);
}

void bidib_state_add_initial_peripheral_value(t_bidib_state_initial_value value) {
	g_array_append_val(bidib_initial_values.peripherals, value);
}

void bidib_state_add_initial_train_value(t_bidib_state_train_initial_value value) {
	g_array_append_val(bidib_initial_values.trains, value);
}

void bidib_state_update_train_available(void) {
	t_bidib_train_state_intern *train_state;
	t_bidib_train_position_query query;
	for (size_t i = 0; i < bidib_track_state.trains->len; i++) {
		train_state = &g_array_index(
				bidib_track_state.trains, t_bidib_train_state_intern, i);
		query = bidib_get_train_position_intern(train_state->id->str);
		if (query.length > 0) {
			train_state->orientation = 
				(query.orientation_is_left ? BIDIB_TRAIN_ORIENTATION_LEFT 
				                           : BIDIB_TRAIN_ORIENTATION_RIGHT);
			if (train_state->on_track == false) {
				syslog_libbidib(LOG_NOTICE, "Train %s detected, orientated %s",
				                train_state->id->str,
				                query.orientation_is_left ? "left" : "right");
			}
			train_state->on_track = true;
		} else {
			if (train_state->on_track == true) {
				syslog_libbidib(LOG_WARNING, "Train %s lost",
				                train_state->id->str);
			}
			train_state->on_track = false;
		}
		bidib_free_train_position_query(query);
	}
}

void bidib_state_reset(void) {
	pthread_mutex_lock(&bidib_state_track_mutex);

	t_bidib_board_accessory_state *board_accessory_state;
	for (size_t i = 0; i < bidib_track_state.points_board->len; i++) {
		board_accessory_state = &g_array_index(bidib_track_state.points_board,
		                                       t_bidib_board_accessory_state, i);
		board_accessory_state->data.state_id = NULL;
		board_accessory_state->data.state_value = 0x00;
		board_accessory_state->data.execution_state = BIDIB_EXEC_STATE_REACHED;
		board_accessory_state->data.wait_details = 0x00;
	}

	t_bidib_dcc_accessory_state *dcc_accessory_state;
	for (size_t i = 0; i < bidib_track_state.points_dcc->len; i++) {
		dcc_accessory_state = &g_array_index(bidib_track_state.points_dcc,
		                                     t_bidib_dcc_accessory_state, i);
		dcc_accessory_state->data.state_id = NULL;
		dcc_accessory_state->data.state_value = 0x00;
		dcc_accessory_state->data.time_unit = BIDIB_TIMEUNIT_MILLISECONDS;
		dcc_accessory_state->data.switch_time = 0x00;
	}

	for (size_t i = 0; i < bidib_track_state.signals_board->len; i++) {
		board_accessory_state = &g_array_index(bidib_track_state.signals_board,
		                                       t_bidib_board_accessory_state, i);
		board_accessory_state->data.state_id = NULL;
		board_accessory_state->data.state_value = 0x00;
		board_accessory_state->data.execution_state = BIDIB_EXEC_STATE_REACHED;
		board_accessory_state->data.wait_details = 0x00;
	}

	for (size_t i = 0; i < bidib_track_state.signals_dcc->len; i++) {
		dcc_accessory_state = &g_array_index(bidib_track_state.signals_dcc,
		                                     t_bidib_dcc_accessory_state, i);
		dcc_accessory_state->data.state_id = NULL;
		dcc_accessory_state->data.state_value = 0x00;
		dcc_accessory_state->data.coil_on = 0x00;
		dcc_accessory_state->data.time_unit = BIDIB_TIMEUNIT_MILLISECONDS;
		dcc_accessory_state->data.switch_time = 0x00;
	}

	t_bidib_peripheral_state *peripheral_state;
	for (size_t i = 0; i < bidib_track_state.peripherals->len; i++) {
		peripheral_state = &g_array_index(bidib_track_state.peripherals,
		                                  t_bidib_peripheral_state, i);
		peripheral_state->data.state_id = NULL;
		peripheral_state->data.state_value = 0x00;
		peripheral_state->data.time_unit = BIDIB_TIMEUNIT_MILLISECONDS;
		peripheral_state->data.wait = 0x00;
	}

	t_bidib_segment_state_intern *segment_state;
	for (size_t i = 0; i < bidib_track_state.segments->len; i++) {
		segment_state = &g_array_index(bidib_track_state.segments,
		                               t_bidib_segment_state_intern, i);
		segment_state->occupied = false;
		segment_state->confidence.conf_void = false;
		segment_state->confidence.freeze = false;
		segment_state->confidence.nosignal = false;
		segment_state->power_consumption.known = false;
		segment_state->power_consumption.overcurrent = false;
		segment_state->power_consumption.current = 0;
		if (segment_state->dcc_addresses->len > 0) {
			g_array_remove_range(segment_state->dcc_addresses, 0,
			                     segment_state->dcc_addresses->len);
		}
	}

	t_bidib_train_state_intern *train_state;
	for (size_t i = 0; i < bidib_track_state.trains->len; i++) {
		train_state = &g_array_index(bidib_track_state.trains,
		                             t_bidib_train_state_intern, i);
		train_state->on_track = false;
		train_state->orientation = BIDIB_TRAIN_ORIENTATION_LEFT;
		train_state->set_speed_step = 0;
		train_state->ack = BIDIB_DCC_ACK_PENDING;
		for (size_t j = 0; j < train_state->peripherals->len; j++) {
			g_array_index(train_state->peripherals,
			              t_bidib_train_peripheral_state, j).state = 0x00;
		}
		train_state->decoder_state.signal_quality_known = false;
		train_state->decoder_state.temp_known = false;
		train_state->decoder_state.energy_storage_known = false;
		train_state->decoder_state.container2_storage_known = false;
		train_state->decoder_state.container3_storage_known = false;
	}

	t_bidib_booster_state *booster_state;
	for (size_t i = 0; i < bidib_track_state.boosters->len; i++) {
		booster_state = &g_array_index(bidib_track_state.boosters,
		                               t_bidib_booster_state, i);
		booster_state->data.power_state = BIDIB_BSTR_OFF;
		booster_state->data.power_state_simple = bidib_booster_normal_to_simple(
				booster_state->data.power_state);
		booster_state->data.power_consumption.known = false;
		booster_state->data.voltage_known = false;
		booster_state->data.temp_known = false;
	}

	t_bidib_track_output_state *track_output_state;
	for (size_t i = 0; i < bidib_track_state.track_outputs->len; i++) {
		track_output_state = &g_array_index(bidib_track_state.track_outputs,
		                                    t_bidib_track_output_state, i);
		track_output_state->cs_state = BIDIB_CS_OFF;
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_reset_train_params(void) {
	t_bidib_cs_drive_mod params;
	params.active = 0x00;
	params.speed = 0x00;
	params.function1 = 0x00;
	params.function2 = 0x00;
	params.function3 = 0x00;
	params.function4 = 0x00;
	pthread_mutex_lock(&bidib_state_trains_mutex);
	t_bidib_train *train_i;
	for (size_t i = 0; i < bidib_trains->len; i++) {
		train_i = &g_array_index(bidib_trains, t_bidib_train, i);
		params.dcc_address = train_i->dcc_addr;
		switch (train_i->dcc_speed_steps) {
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
		t_bidib_board *board_i;
		pthread_mutex_lock(&bidib_state_boards_mutex);
		for (size_t j = 0; j < bidib_boards->len; j++) {
			board_i = &g_array_index(bidib_boards, t_bidib_board, j);
			if (board_i->connected && board_i->unique_id.class_id & (1 << 4)) {
				//unavoidable if lock order is to be kept valid
				pthread_mutex_unlock(&bidib_state_boards_mutex);
				pthread_mutex_unlock(&bidib_state_trains_mutex);
				bidib_send_cs_drive(board_i->node_addr, params, 0);
				pthread_mutex_lock(&bidib_state_boards_mutex);
				pthread_mutex_lock(&bidib_state_trains_mutex);
			}
		}
		pthread_mutex_unlock(&bidib_state_boards_mutex);
	}
	pthread_mutex_unlock(&bidib_state_trains_mutex);
}
