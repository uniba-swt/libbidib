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

#include <syslog.h>

#include "../../include/bidib.h"
#include "bidib_state_intern.h"
#include "bidib_state_getter_intern.h"
#include "../../include/definitions/bidib_definitions_custom.h"


void bidib_state_boost_stat(t_bidib_node_address node_address, uint8_t power_state) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_booster_state *booster_state =
			bidib_state_get_booster_state_ref_by_nodeaddr(node_address);
	if (booster_state != NULL) {
		booster_state->data.power_state = (t_bidib_booster_power_state) power_state;
		booster_state->data.power_state_simple =
				bidib_booster_normal_to_simple(booster_state->data.power_state);
	} else {
		syslog(LOG_ERR, "No booster configured with node address 0x%02x 0x%02x 0x%02x 0x0",
		       node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_accessory_state(t_bidib_node_address node_address, uint8_t number,
                                 uint8_t aspect, uint8_t total,
                                 uint8_t execution, uint8_t wait) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	bool point;
	t_bidib_board_accessory_mapping *accessory_mapping =
			bidib_state_get_board_accessory_mapping_ref_by_number(node_address, number, &point);
	t_bidib_board_accessory_state *accessory_state;
	if (accessory_mapping != NULL &&
	    (accessory_state = bidib_state_get_board_accessory_state_ref(accessory_mapping->id->str, point)) != NULL) {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		accessory_state->data.state_id = NULL;
		t_bidib_aspect *aspect_mapping;
		for (size_t i = 0; i < accessory_mapping->aspects->len; i++) {
			aspect_mapping = &g_array_index(accessory_mapping->aspects, t_bidib_aspect, i);
			if (aspect_mapping->value == aspect) {
				accessory_state->data.state_id = aspect_mapping->id->str;
			}
		}
		if (accessory_state->data.state_id == NULL) {
			syslog(LOG_WARNING, "Aspect 0x%02x of accessory %s is not mapped in config files",
			       aspect, accessory_mapping->id->str);
		}
		accessory_state->data.state_value = aspect;
		accessory_state->data.execution_state = (t_bidib_accessory_execution_state) execution;
		accessory_state->data.wait_details = wait;
		if (total < accessory_mapping->aspects->len) {
			syslog(LOG_ERR, "More aspects configured in track config than on bidib board for accessory %s",
			       accessory_mapping->id->str);
		}
	} else {
		pthread_mutex_unlock(&bidib_state_boards_mutex);
		syslog(LOG_ERR, "No board accessory 0x%02x configured for node address 0x%02x 0x%02x 0x%02x 0x0",
		       number, node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_node_new(t_bidib_node_address node_address, uint8_t local_addr,
                          t_bidib_unique_id_mod unique_id) {
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board = bidib_state_get_board_ref_by_uniqueid(unique_id);
	if (board != NULL) {
		board->connected = true;
		if (node_address.top == 0x00) {
			node_address.top = local_addr;
		} else if (node_address.sub == 0x00) {
			node_address.sub = local_addr;
		} else {
			node_address.subsub = local_addr;
		}
		board->node_addr = node_address;
	} else {
		syslog(LOG_ERR, "No board configured for unique id 0x%02x%02x%02x%02x%02x%02x%02x",
		       unique_id.class_id, unique_id.class_id_ext, unique_id.vendor_id,
		       unique_id.product_id1, unique_id.product_id2, unique_id.product_id3,
		       unique_id.product_id4);
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
}

static bool bidib_state_is_subnode(t_bidib_node_address node_address,
                                   t_bidib_node_address subnode_address) {
	uint8_t node[] = {node_address.top, node_address.sub, node_address.subsub};
	uint8_t subnode[] = {subnode_address.top, subnode_address.sub, subnode_address.subsub};
	size_t i = 0;
	while (i < 3) {
		if (node[i] != subnode[i]) {
			break;
		}
		i++;
	}
	if (i < 3 && node[i] == 0) {
		return true;
	}
	return false;
}

void bidib_state_node_lost(t_bidib_unique_id_mod unique_id) {
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board = bidib_state_get_board_ref_by_uniqueid(unique_id);
	if (board != NULL) {
		board->connected = false;
		if (board->unique_id.class_id & (1 << 7)) {
			// if interface all subnodes are lost too
			t_bidib_board *board_i;
			for (size_t i = 0; i < bidib_boards->len; i++) {
				board_i = &g_array_index(bidib_boards, t_bidib_board, i);
				if (bidib_state_is_subnode(board->node_addr, board_i->node_addr)) {
					board_i->connected = false;
				}
			}
		}
	} else {
		syslog(LOG_ERR, "No board configured for unique id 0x%02x%02x%02x%02x%02x%02x%02x",
		       unique_id.class_id, unique_id.class_id_ext, unique_id.vendor_id,
		       unique_id.product_id1, unique_id.product_id2, unique_id.product_id3,
		       unique_id.product_id4);
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
}

void bidib_state_cs_state(t_bidib_node_address node_address, uint8_t state) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_track_output_state *track_output_state =
			bidib_state_get_track_output_state_ref_by_nodeaddr(node_address);
	if (track_output_state != NULL) {
		track_output_state->cs_state = (t_bidib_cs_state) state;
	} else {
		syslog(LOG_ERR, "No track output configured for node address 0x%02x 0x%02x 0x%02x 0x0",
		       node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_cs_drive_ack(t_bidib_dcc_address dcc_address, uint8_t ack) {
	pthread_mutex_lock(&bidib_state_trains_mutex);
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_state_intern *train_state =
			bidib_state_get_train_state_ref_by_dccaddr(dcc_address);
	if (train_state != NULL) {
		train_state->ack = (t_bidib_cs_ack) ack;
	} else {
		syslog(LOG_ERR, "No train configured for dcc address 0x%02x%02x",
		       dcc_address.addrh, dcc_address.addrl);
		if (bidib_state_dcc_addr_in_use(dcc_address)) {
			syslog(LOG_ERR, "Dcc address 0x%02x%02x is already in use, remove the "
					       "unconfigured train to avoid conflicts",
			       dcc_address.addrh, dcc_address.addrl);
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
	pthread_mutex_unlock(&bidib_state_trains_mutex);
}

void bidib_state_cs_accessory_ack(t_bidib_node_address node_address,
                                  t_bidib_dcc_address dcc_address, uint8_t ack) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	bool point;
	t_bidib_dcc_accessory_mapping *accessory_mapping =
			bidib_state_get_dcc_accessory_mapping_ref_by_dccaddr(node_address, dcc_address, &point);
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	t_bidib_dcc_accessory_state *accessory_state;
	if (accessory_mapping != NULL &&
	    (accessory_state = bidib_state_get_dcc_accessory_state_ref(accessory_mapping->id->str, point)) != NULL) {
		accessory_state->data.ack = (t_bidib_cs_ack) ack;
	} else {
		syslog(LOG_ERR, "No dcc accessory configured for dcc address 0x%02x%02x",
		       dcc_address.addrh, dcc_address.addrl);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_cs_drive(t_bidib_cs_drive_mod params) {
	pthread_mutex_lock(&bidib_state_track_mutex);

	t_bidib_train_state_intern *train_state =
			bidib_state_get_train_state_ref_by_dccaddr(params.dcc_address);
	t_bidib_train_peripheral_state *peripheral_state;
	if (train_state != NULL) {
		uint8_t function_bits[] = {params.function1, params.function2,
		                                 params.function3, params.function4};
		if (params.active == 0x00) {
			train_state->direction = BIDIB_TRAIN_DIRECTION_FORWARD;
			train_state->set_speed_step = 0;
			for (size_t i = 0; i < train_state->peripherals->len; i++) {
				peripheral_state = &g_array_index(train_state->peripherals,
				                                  t_bidib_train_peripheral_state, i);
				peripheral_state->state = 0x00;
			}
		} else {
			if (params.active & (1 << 0)) {
				// speed active
				if (params.speed & (1 << 7)) {
					train_state->direction = BIDIB_TRAIN_DIRECTION_FORWARD;
				} else {
					train_state->direction = BIDIB_TRAIN_DIRECTION_BACKWARD;
				}
				train_state->set_speed_step = bidib_dcc_speed_to_lib_format(params.speed);
			}
			train_state->ack = BIDIB_DCC_ACK_PENDING;
			if (params.active & (1 << 1)) {
				for (size_t i = 0; i < 5; i++) {
					peripheral_state = bidib_state_get_train_peripheral_state_by_bit(
							train_state, (uint8_t) i);
					if (peripheral_state != NULL) {
						peripheral_state->state =
								(uint8_t) ((function_bits[i / 8] >> (i % 8)) & 0x01);
					}
				}
			}
			if (params.active & (1 << 2)) {
				for (size_t i = 8; i < 12; i++) {
					peripheral_state = bidib_state_get_train_peripheral_state_by_bit(
							train_state, (uint8_t) i);
					if (peripheral_state != NULL) {
						peripheral_state->state =
								(uint8_t) ((function_bits[i / 8] >> (i % 8)) & 0x01);
					}
				}
			}
			if (params.active & (1 << 3)) {
				for (size_t i = 12; i < 16; i++) {
					peripheral_state = bidib_state_get_train_peripheral_state_by_bit(
							train_state, (uint8_t) i);
					if (peripheral_state != NULL) {
						peripheral_state->state =
								(uint8_t) ((function_bits[i / 8] >> (i % 8)) & 0x01);
					}
				}
			}
			if (params.active & (1 << 4)) {
				for (size_t i = 16; i < 24; i++) {
					peripheral_state = bidib_state_get_train_peripheral_state_by_bit(
							train_state, (uint8_t) i);
					if (peripheral_state != NULL) {
						peripheral_state->state =
								(uint8_t) ((function_bits[i / 8] >> (i % 8)) & 0x01);
					}
				}
			}
			if (params.active & (1 << 5)) {
				for (size_t i = 24; i < 32; i++) {
					peripheral_state = bidib_state_get_train_peripheral_state_by_bit(
							train_state, (uint8_t) i);
					if (peripheral_state != NULL) {
						peripheral_state->state =
								(uint8_t) ((function_bits[i / 8] >> (i % 8)) & 0x01);
					}
				}
			}
		}
	} else {
		syslog(LOG_ERR, "No train configured for dcc address 0x%02x%02x",
		       params.dcc_address.addrh, params.dcc_address.addrl);
		if (bidib_state_dcc_addr_in_use(params.dcc_address)) {
			syslog(LOG_ERR, "Dcc address 0x%02x%02x is already in use, remove the "
					       "unconfigured train to avoid conflicts",
			       params.dcc_address.addrh, params.dcc_address.addrl);
		}
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_cs_accessory_manual(t_bidib_node_address node_address,
                                     t_bidib_dcc_address dcc_address, uint8_t data) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	bool point;
	t_bidib_dcc_accessory_mapping *accessory_mapping =
			bidib_state_get_dcc_accessory_mapping_ref_by_dccaddr(node_address, dcc_address, &point);
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	t_bidib_dcc_accessory_state *accessory_state;
	if (accessory_mapping != NULL &&
	    (accessory_state =
			     bidib_state_get_dcc_accessory_state_ref(accessory_mapping->id->str, point)) != NULL) {
		accessory_state->data.state_value = (uint8_t) (data & 0x1F);
		if (data & (1 << 5)) {
			accessory_state->data.coil_on = true;
		} else {
			accessory_state->data.coil_on = false;
		}
		accessory_state->data.switch_time = 0;
	} else {
		syslog(LOG_ERR, "No dcc accessory configured for dcc address 0x%02x%02x",
		       dcc_address.addrh, dcc_address.addrl);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_cs_accessory(t_bidib_node_address node_address,
                              t_bidib_cs_accessory_mod params) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	bool point;
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_dcc_accessory_mapping *accessory_mapping =
			bidib_state_get_dcc_accessory_mapping_ref_by_dccaddr(node_address, params.dcc_address, &point);
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	t_bidib_dcc_accessory_state *accessory_state;
	if (accessory_mapping != NULL &&
	    (accessory_state =
			     bidib_state_get_dcc_accessory_state_ref(accessory_mapping->id->str, point)) != NULL) {
		accessory_state->data.state_id = NULL;
		accessory_state->data.state_value = (uint8_t) (params.data & 0x1F);
		if (params.data & (1 << 5)) {
			accessory_state->data.coil_on = true;
		} else {
			accessory_state->data.coil_on = false;
		}
		if (params.data & (1 << 6)) {
			accessory_state->data.output_controls_timing = false;
		} else {
			accessory_state->data.output_controls_timing = true;
		}
		if (params.time & (1 << 7)) {
			accessory_state->data.time_unit = BIDIB_TIMEUNIT_SECONDS;
		} else {
			accessory_state->data.time_unit = BIDIB_TIMEUNIT_MILLISECONDS;
		}
		accessory_state->data.switch_time = (uint8_t) (params.time & 0x7F);
	} else {
		syslog(LOG_ERR, "No dcc accessory configured for dcc address 0x%02x%02x",
		       params.dcc_address.addrh, params.dcc_address.addrl);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_lc_stat(t_bidib_node_address node_address, t_bidib_peripheral_port port,
                         uint8_t portstat) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_peripheral_mapping *peripheral_mapping =
			bidib_state_get_peripheral_mapping_ref_by_port(node_address, port);
	t_bidib_peripheral_state *peripheral_state;
	if (peripheral_mapping != NULL &&
	    (peripheral_state = bidib_state_get_peripheral_state_ref(peripheral_mapping->id->str)) != NULL) {
		peripheral_state->data.state_id = NULL;
		t_bidib_aspect *aspect_mapping;
		for (size_t i = 0; i < peripheral_mapping->aspects->len; i++) {
			aspect_mapping = &g_array_index(peripheral_mapping->aspects, t_bidib_aspect, i);
			if (aspect_mapping->value == portstat) {
				peripheral_state->data.state_id = aspect_mapping->id->str;
			}
		}
		if (peripheral_state->data.state_id == NULL) {
			syslog(LOG_WARNING, "Aspect 0x%02x of peripheral %s is not mapped in config files",
			       portstat, peripheral_mapping->id->str);
		}
		peripheral_state->data.state_value = portstat;
	} else {
		syslog(LOG_ERR, "No peripheral on port 0x%02x 0x%02x configured for node address "
				       "0x%02x 0x%02x 0x%02x 0x0",
		       port.port0, port.port1, node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_lc_wait(t_bidib_node_address node_address, t_bidib_peripheral_port port,
                         uint8_t time) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_peripheral_mapping *peripheral_mapping =
			bidib_state_get_peripheral_mapping_ref_by_port(node_address, port);
	t_bidib_peripheral_state *peripheral_state;
	if (peripheral_mapping != NULL &&
	    (peripheral_state = bidib_state_get_peripheral_state_ref(peripheral_mapping->id->str)) != NULL) {
		if (time & (1 << 7)) {
			peripheral_state->data.time_unit = BIDIB_TIMEUNIT_SECONDS;
		} else {
			peripheral_state->data.time_unit = BIDIB_TIMEUNIT_MILLISECONDS;
		}
		peripheral_state->data.wait = (uint8_t) (time & 0x7F);
	} else {
		syslog(LOG_ERR, "No peripheral on port 0x%02x 0x%02x configured for node address "
				       "0x%02x 0x%02x 0x%02x 0x0",
		       port.port0, port.port1, node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_log_train_detect(bool detected, t_bidib_dcc_address *dcc_address,
                                  t_bidib_segment_state_intern *segment_state) {
	t_bidib_train_state_intern *train_state =
		bidib_state_get_train_state_ref_by_dccaddr(*dcc_address);
	if (detected) {
		if (train_state == NULL) {
			syslog(LOG_NOTICE, "Segment: %s is being entered by: unknown train (0x%02x%02x)",
			       segment_state->id->str, dcc_address->addrh, dcc_address->addrl);
		} else {
			syslog(LOG_NOTICE, "Segment: %s is being entered by: %s",
			       segment_state->id->str, train_state->id->str);
		}
	} else {
		if (train_state == NULL) {
			syslog(LOG_NOTICE, "Segment: %s is being exited by: unknown train (0x%02x%02x)",
			       segment_state->id->str, dcc_address->addrh, dcc_address->addrl);
		} else {
			syslog(LOG_NOTICE, "Segment: %s is being exited by: %s",
			       segment_state->id->str, train_state->id->str);
		}
	}
}

void bidib_state_bm_occ(t_bidib_node_address node_address, uint8_t number, bool occ) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_segment_state_intern *segment_state =
			bidib_state_get_segment_state_ref_by_nodeaddr(node_address, number);
	if (segment_state != NULL) {
		segment_state->occupied = occ;
		if (!occ && segment_state->dcc_addresses->len > 0) {
			t_bidib_dcc_address *dcc_address;
			for (size_t j = 0; j < segment_state->dcc_addresses->len; j++) {
				dcc_address = &g_array_index(segment_state->dcc_addresses,
				                             t_bidib_dcc_address, j);
				bidib_state_log_train_detect(false, dcc_address,
				                             segment_state);
			}
			g_array_remove_range(segment_state->dcc_addresses, 0,
			                     segment_state->dcc_addresses->len);
		}
		bidib_state_update_train_available();
	} else {
		syslog(LOG_ERR, "No segment with number 0x%02x configured for node address "
				        "0x%02x 0x%02x 0x%02x 0x0",
		       number, node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_bm_multiple(t_bidib_node_address node_address, uint8_t number,
                             uint8_t size, uint8_t *data) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_segment_state_intern *segment_state;
	for (size_t i = 0; i < size; i++) {
		if (number + i < 255) {
			segment_state = bidib_state_get_segment_state_ref_by_nodeaddr(
					node_address, (uint8_t) (number + i));
			if (segment_state != NULL) {
				if (data[i / 8] & (1 << i % 8)) {
					segment_state->occupied = true;
				} else {
					segment_state->occupied = false;
					if (segment_state->dcc_addresses->len > 0) {
						t_bidib_dcc_address *dcc_address;
						for (size_t j = 0; j < segment_state->dcc_addresses->len; j++) {
							dcc_address = &g_array_index(segment_state->dcc_addresses,
							                             t_bidib_dcc_address, j);
							bidib_state_log_train_detect(false, dcc_address,
							                             segment_state);
						}
						g_array_remove_range(segment_state->dcc_addresses, 0,
						                     segment_state->dcc_addresses->len);
					}
				}
			} else if (data[i / 8] & (1 << i % 8)) {
				syslog(LOG_ERR, "No segment with number 0x%02x configured for "
						        "node address 0x%02x 0x%02x 0x%02x 0x0",
				       (unsigned int) (number + i), node_address.top,
				       node_address.sub, node_address.subsub);
			}
		}
	}
	bidib_state_update_train_available();
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_bm_confidence(t_bidib_node_address node_address, uint8_t conf_void,
                               uint8_t freeze, uint8_t nosignal) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	pthread_mutex_lock(&bidib_state_boards_mutex);
	t_bidib_board *board = bidib_state_get_board_ref_by_nodeaddr(node_address);
	if (board != NULL) {
		t_bidib_segment_state_intern *segment_state;
		t_bidib_segment_mapping *segment_mapping;
		for (size_t i = 0; i < board->segments->len; i++) {
			segment_mapping = &g_array_index(board->segments, t_bidib_segment_mapping, i);
			segment_state = bidib_state_get_segment_state_ref(segment_mapping->id->str);
			if (conf_void == 0) {
				segment_state->confidence.conf_void = false;
			} else {
				segment_state->confidence.conf_void = true;
			}
			if (freeze == 0) {
				segment_state->confidence.freeze = false;
			} else {
				segment_state->confidence.freeze = true;
			}
			if (nosignal == 0) {
				segment_state->confidence.nosignal = false;
			} else {
				segment_state->confidence.nosignal = true;
			}
		}
	} else {
		syslog(LOG_ERR, "No board configured for node address 0x%02x 0x%02x 0x%02x 0x0",
		       node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_boards_mutex);
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_bm_address_log_changes(t_bidib_segment_state_intern *segment_state,
                                        uint8_t address_count, uint8_t *addresses) {
	t_bidib_dcc_address *dcc_address_old, dcc_address_new;

	// check for new addresses
	bool already_detected = false;
	if (!(address_count == 1 && addresses[0] == 0x00 && addresses[1] == 0x00)) {
		// ignore 0x0000 (free)
		for (size_t i = 0; i < address_count; i++) {
			if ((addresses[(i * 2) + 1] & (1 << 6)) == 0) {
				// ignore dcc accessory, only add trains, see:
				// http://bidib.org/protokoll/bidib_occ_e.html#T-addressformat
				dcc_address_new.addrl = addresses[i * 2];
				dcc_address_new.addrh = (uint8_t) (addresses[(i * 2) + 1] & 0x3F);
				for (size_t j = 0; j < segment_state->dcc_addresses->len; j++) {
					dcc_address_old = &g_array_index(segment_state->dcc_addresses,
					                                 t_bidib_dcc_address, j);
					if (dcc_address_old->addrl == dcc_address_new.addrl &&
					    dcc_address_old->addrh == dcc_address_new.addrh) {
						already_detected = true;
						break;
					}
				}
				if (!already_detected) {
					bidib_state_log_train_detect(true, &dcc_address_new,
					                             segment_state);
				} else {
					already_detected = false;
				}
			}
		}
	}

	// check for lost addresses
	bool still_in_segment = false;
	for (size_t i = 0; i < segment_state->dcc_addresses->len; i++) {
		dcc_address_old = &g_array_index(segment_state->dcc_addresses,
		                                 t_bidib_dcc_address, i);
		for (size_t j = 0; j < address_count; j++) {
			dcc_address_new.addrl = addresses[j * 2];
			dcc_address_new.addrh = (uint8_t) (addresses[(j * 2) + 1] & 0x3F);
			if (dcc_address_old->addrl == dcc_address_new.addrl &&
			    dcc_address_old->addrh == dcc_address_new.addrh) {
				still_in_segment = true;
				break;
			}
		}
		if (!still_in_segment) {
			bidib_state_log_train_detect(false, dcc_address_old,
			                             segment_state);
		} else {
			still_in_segment = false;
		}
	}
}

void bidib_state_bm_address(t_bidib_node_address node_address, uint8_t number,
                            uint8_t address_count, uint8_t *addresses) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_segment_state_intern *segment_state =
			bidib_state_get_segment_state_ref_by_nodeaddr(node_address, number);
	if (segment_state != NULL) {
		bidib_state_bm_address_log_changes(segment_state, address_count, addresses);
		if (segment_state->dcc_addresses->len > 0) {
			g_array_remove_range(segment_state->dcc_addresses, 0,
			                     segment_state->dcc_addresses->len);
		}
		t_bidib_dcc_address dcc_address;
		if (!(address_count == 1 && addresses[0] == 0x00 && addresses[1] == 0x00)) {
			for (size_t i = 0; i < address_count; i++) {
				if ((addresses[(i * 2) + 1] & (1 << 6)) == 0) {
					// ignore dcc accessory, only add trains, see:
					// http://bidib.org/protokoll/bidib_occ_e.html#T-addressformat
					dcc_address.addrl = addresses[i * 2];
					dcc_address.addrh = (uint8_t) (addresses[(i * 2) + 1] & 0x3F);
					g_array_append_val(segment_state->dcc_addresses, dcc_address);
				}
			}
		}
		bidib_state_update_train_available();
	} else if (!(address_count == 1 && addresses[0] == 0x00 && addresses[1] == 0x00)) {
		// ignore free messages for unconnected segments (happens after track output is turned on)
		syslog(LOG_ERR, "No segment with number 0x%02x configured for node address"
		       " 0x%02x 0x%02x 0x%02x 0x00",
		       number, node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_bm_current(t_bidib_node_address node_address, uint8_t number,
                            uint8_t current) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_segment_state_intern *segment_state =
			bidib_state_get_segment_state_ref_by_nodeaddr(node_address, number);
	if (segment_state != NULL) {
		if (current == 0) {
			segment_state->power_consumption.known = true;
			segment_state->power_consumption.overcurrent = false;
			segment_state->power_consumption.current = 0;
		} else if (current < 16) {
			segment_state->power_consumption.known = true;
			segment_state->power_consumption.overcurrent = false;
			segment_state->power_consumption.current = current;
		} else if (current < 64) {
			segment_state->power_consumption.known = true;
			segment_state->power_consumption.overcurrent = false;
			segment_state->power_consumption.current = (unsigned int) ((current - 12) * 4);
		} else if (current < 128) {
			segment_state->power_consumption.known = true;
			segment_state->power_consumption.overcurrent = false;
			segment_state->power_consumption.current = (unsigned int) ((current - 51) * 16);
		} else if (current < 192) {
			segment_state->power_consumption.known = true;
			segment_state->power_consumption.overcurrent = false;
			segment_state->power_consumption.current = (unsigned int) ((current - 108) * 64);
		} else if (current < 251) {
			segment_state->power_consumption.known = true;
			segment_state->power_consumption.overcurrent = false;
			segment_state->power_consumption.current = (unsigned int) ((current - 171) * 256);
		} else if (current < 254) {
			segment_state->power_consumption.known = false;
		} else if (current < 255) {
			segment_state->power_consumption.known = true;
			segment_state->power_consumption.overcurrent = true;
		} else {
			segment_state->power_consumption.known = false;
		}
	} else {
		syslog(LOG_ERR, "No segment with number 0x%02x configured for node address "
				       "0x%02x 0x%02x 0x%02x 0x0",
		       number, node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_bm_speed(t_bidib_dcc_address dcc_address, uint8_t speedl,
                          uint8_t speedh) {
	pthread_mutex_lock(&bidib_state_trains_mutex);
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_state_intern *train_state =
			bidib_state_get_train_state_ref_by_dccaddr(dcc_address);
	if (train_state != NULL) {
		train_state->detected_kmh_speed = (speedh << 8) | speedl;
	} else {
		syslog(LOG_ERR, "No train configured for dcc address 0x%02x 0x%02x",
		       dcc_address.addrl, dcc_address.addrh);
	}
	pthread_mutex_unlock(&bidib_state_trains_mutex);
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_bm_dyn_state(t_bidib_dcc_address dcc_address, uint8_t dyn_num,
                              uint8_t value) {
	pthread_mutex_lock(&bidib_state_trains_mutex);
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_train_state_intern *train_state =
			bidib_state_get_train_state_ref_by_dccaddr(dcc_address);
	if (train_state != NULL) {
		if (dyn_num == 1) {
			// signal quality
			train_state->decoder_state.signal_quality_known = true;
			train_state->decoder_state.signal_quality = value;
		} else if (dyn_num == 2) {
			// temperature
			train_state->decoder_state.temp_known = true;
			train_state->decoder_state.temp_celsius = value;
		} else if (dyn_num == 3) {
			// energy storage
			train_state->decoder_state.energy_storage_known = true;
			train_state->decoder_state.energy_storage = value;
		} else if (dyn_num == 4) {
			// storage 2
			train_state->decoder_state.container2_storage_known = true;
			train_state->decoder_state.container2_storage = value;
		} else if (dyn_num == 5) {
			// storage 3
			train_state->decoder_state.container3_storage_known = true;
			train_state->decoder_state.container3_storage = value;
		}
	} else {
		syslog(LOG_ERR, "No train configured for dcc address 0x%02x 0x%02x",
		       dcc_address.addrl, dcc_address.addrh);
	}
	pthread_mutex_unlock(&bidib_state_trains_mutex);
	pthread_mutex_unlock(&bidib_state_track_mutex);
}

void bidib_state_boost_diagnostic(t_bidib_node_address node_address, uint8_t length,
                                  uint8_t *diag_list) {
	pthread_mutex_lock(&bidib_state_track_mutex);
	t_bidib_booster_state *booster_state =
			bidib_state_get_booster_state_ref_by_nodeaddr(node_address);
	if (booster_state != NULL) {
		for (size_t i = 0; i < length; i++) {
			switch (diag_list[i]) {
				case 0:
					if (diag_list[i + 1] == 0) {
						booster_state->data.power_consumption.known = true;
						booster_state->data.power_consumption.overcurrent = false;
						booster_state->data.power_consumption.current = 0;
					} else if (diag_list[i + 1] < 16) {
						booster_state->data.power_consumption.known = true;
						booster_state->data.power_consumption.overcurrent = false;
						booster_state->data.power_consumption.current = diag_list[i + 1];
					} else if (diag_list[i + 1] < 64) {
						booster_state->data.power_consumption.known = true;
						booster_state->data.power_consumption.overcurrent = false;
						booster_state->data.power_consumption.current =
								(unsigned int) ((diag_list[i + 1] - 12) * 4);
					} else if (diag_list[i + 1] < 128) {
						booster_state->data.power_consumption.known = true;
						booster_state->data.power_consumption.overcurrent = false;
						booster_state->data.power_consumption.current =
								(unsigned int) ((diag_list[i + 1] - 51) * 16);
					} else if (diag_list[i + 1] < 192) {
						booster_state->data.power_consumption.known = true;
						booster_state->data.power_consumption.overcurrent = false;
						booster_state->data.power_consumption.current =
								(unsigned int) ((diag_list[i + 1] - 108) * 64);
					} else if (diag_list[i + 1] < 251) {
						booster_state->data.power_consumption.known = true;
						booster_state->data.power_consumption.overcurrent = false;
						booster_state->data.power_consumption.current =
								(unsigned int) ((diag_list[i + 1] - 171) * 256);
					} else if (diag_list[i + 1] < 254) {
						booster_state->data.power_consumption.known = false;
					} else if (diag_list[i + 1] < 255) {
						booster_state->data.power_consumption.known = true;
						booster_state->data.power_consumption.overcurrent = true;
					} else {
						booster_state->data.power_consumption.known = false;
					}
					break;
				case 1:
					if (diag_list[i + 1] < 251) {
						booster_state->data.voltage_known = true;
						booster_state->data.voltage = diag_list[i + 1];
					} else {
						booster_state->data.voltage_known = false;
					}
					break;
				case 2:
					booster_state->data.temp_known = true;
					booster_state->data.temp_celsius = diag_list[i + 1];
					break;
				default:
					break;
			}
		}
	} else {
		syslog(LOG_ERR, "No booster configured with node address 0x%02x 0x%02x 0x%02x 0x0",
		       node_address.top, node_address.sub, node_address.subsub);
	}
	pthread_mutex_unlock(&bidib_state_track_mutex);
}
