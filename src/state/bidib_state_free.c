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
 * - Bernhard Luedtke <https://github.com/BLuedtke>
 *
 */

#include <memory.h>
#include <stdlib.h>
#include <glib.h>

#include "bidib_state_intern.h"
#include "../transmission/bidib_transmission_intern.h"


void bidib_state_free_single_initial_value(t_bidib_state_initial_value value) {
	if (value.id != NULL) {
		g_string_free(value.id, TRUE);
	}
	if (value.value != NULL) {
		g_string_free(value.value, TRUE);
	}
}

void bidib_state_free_single_train_initial_value(t_bidib_state_train_initial_value value) {
	if (value.id != NULL) {
		g_string_free(value.id, TRUE);
	}
	if (value.train != NULL) {
		g_string_free(value.train, TRUE);
	}
}

void bidib_state_free_single_board_accessory_state(t_bidib_board_accessory_state accessory_state) {
	if (accessory_state.id != NULL) {
		free(accessory_state.id);
	}
}

void bidib_state_free_single_dcc_accessory_state(t_bidib_dcc_accessory_state accessory_state) {
	if (accessory_state.id != NULL) {
		free(accessory_state.id);
	}
}

void bidib_state_free_single_peripheral_state(t_bidib_peripheral_state peripheral_state) {
	if (peripheral_state.id != NULL) {
		free(peripheral_state.id);
	}
}

void bidib_state_free_single_reverser_state(t_bidib_reverser_state reverser_state) {
	if (reverser_state.data.state_id != NULL) {
		free(reverser_state.data.state_id);
	}
	if (reverser_state.id != NULL) {
		free(reverser_state.id);
	}
}

void bidib_state_free_single_segment_state(t_bidib_segment_state segment_state) {
	if (segment_state.id != NULL) {
		free(segment_state.id);
	}
	if (segment_state.data.dcc_addresses != NULL) {
		free(segment_state.data.dcc_addresses);
	}
}

void bidib_state_free_single_segment_state_intern(t_bidib_segment_state_intern segment_state) {
	if (segment_state.id != NULL) {
		g_string_free(segment_state.id, TRUE);
	}
	if (segment_state.length != NULL) {
		g_string_free(segment_state.length, TRUE);
	}
	if (segment_state.dcc_addresses != NULL) {
		g_array_free(segment_state.dcc_addresses, TRUE);
	}
}

void bidib_state_free_single_train_state(t_bidib_train_state train_state) {
	if (train_state.id != NULL) {
		free(train_state.id);
	}
	if (train_state.data.peripherals != NULL) {
		for (size_t i = 0; i < train_state.data.peripheral_cnt; i++) {
			if (train_state.data.peripherals[i].id != NULL) {
				free(train_state.data.peripherals[i].id);
			}
		}
		free(train_state.data.peripherals);
	}
}

void bidib_state_free_single_train_state_intern(t_bidib_train_state_intern train_state) {
	if (train_state.id != NULL) {
		g_string_free(train_state.id, TRUE);
	}
	if (train_state.peripherals != NULL) {
		t_bidib_train_peripheral_state state_i;
		for (size_t i = 0; i < train_state.peripherals->len; i++) {
			state_i = g_array_index(train_state.peripherals,
			                        t_bidib_train_peripheral_state, i);
			if (state_i.id != NULL) {
				free(state_i.id);
			}
		}
		g_array_free(train_state.peripherals, TRUE);
	}
}

void bidib_state_free_single_booster_state(t_bidib_booster_state booster_state) {
	if (booster_state.id != NULL) {
		free(booster_state.id);
	}
}

void bidib_state_free_single_track_output_state(t_bidib_track_output_state to_state) {
	if (to_state.id != NULL) {
		free(to_state.id);
	}
}

void bidib_state_free_single_board(t_bidib_board board) {
	g_string_free(board.id, TRUE);

	g_array_free(board.features, TRUE);

	t_bidib_board_accessory_mapping tmp_board_acc_mapping;
	t_bidib_dcc_accessory_mapping tmp_dcc_acc_mapping;
	t_bidib_peripheral_mapping tmp_peripheral_mapping;
	t_bidib_aspect tmp_aspect;
	t_bidib_dcc_aspect tmp_dcc_aspect;

	for (size_t i = 0; i < board.points_board->len; i++) {
		tmp_board_acc_mapping = g_array_index(board.points_board,
		                                      t_bidib_board_accessory_mapping, i);
		g_string_free(tmp_board_acc_mapping.id, TRUE);
		for (size_t j = 0; j < tmp_board_acc_mapping.aspects->len; j++) {
			tmp_aspect = g_array_index(tmp_board_acc_mapping.aspects,
			                           t_bidib_aspect, j);
			g_string_free(tmp_aspect.id, TRUE);
		}
		g_array_free(tmp_board_acc_mapping.aspects, TRUE);
	}
	g_array_free(board.points_board, TRUE);

	for (size_t i = 0; i < board.points_dcc->len; i++) {
		tmp_dcc_acc_mapping = g_array_index(board.points_dcc,
		                                    t_bidib_dcc_accessory_mapping, i);
		g_string_free(tmp_dcc_acc_mapping.id, TRUE);
		for (size_t j = 0; j < tmp_dcc_acc_mapping.aspects->len; j++) {
			tmp_dcc_aspect = g_array_index(tmp_dcc_acc_mapping.aspects,
			                               t_bidib_dcc_aspect, j);
			g_string_free(tmp_dcc_aspect.id, TRUE);
			g_array_free(tmp_dcc_aspect.port_values, TRUE);
		}
		g_array_free(tmp_dcc_acc_mapping.aspects, TRUE);
	}
	g_array_free(board.points_dcc, TRUE);

	for (size_t i = 0; i < board.signals_board->len; i++) {
		tmp_board_acc_mapping = g_array_index(board.signals_board,
		                                      t_bidib_board_accessory_mapping, i);
		g_string_free(tmp_board_acc_mapping.id, TRUE);
		for (size_t j = 0; j < tmp_board_acc_mapping.aspects->len; j++) {
			tmp_aspect = g_array_index(tmp_board_acc_mapping.aspects,
			                           t_bidib_aspect, j);
			g_string_free(tmp_aspect.id, TRUE);
		}
		g_array_free(tmp_board_acc_mapping.aspects, TRUE);
	}
	g_array_free(board.signals_board, TRUE);

	for (size_t i = 0; i < board.signals_dcc->len; i++) {
		tmp_dcc_acc_mapping = g_array_index(board.signals_dcc,
		                                    t_bidib_dcc_accessory_mapping, i);
		g_string_free(tmp_dcc_acc_mapping.id, TRUE);
		for (size_t j = 0; j < tmp_dcc_acc_mapping.aspects->len; j++) {
			tmp_dcc_aspect = g_array_index(tmp_dcc_acc_mapping.aspects,
			                               t_bidib_dcc_aspect, j);
			g_string_free(tmp_dcc_aspect.id, TRUE);
			g_array_free(tmp_dcc_aspect.port_values, TRUE);
		}
		g_array_free(tmp_dcc_acc_mapping.aspects, TRUE);
	}
	g_array_free(board.signals_dcc, TRUE);

	for (size_t i = 0; i < board.peripherals->len; i++) {
		tmp_peripheral_mapping = g_array_index(board.peripherals,
		                                       t_bidib_peripheral_mapping, i);
		g_string_free(tmp_peripheral_mapping.id, TRUE);
		for (size_t j = 0; j < tmp_peripheral_mapping.aspects->len; j++) {
			tmp_aspect = g_array_index(tmp_peripheral_mapping.aspects,
			                           t_bidib_aspect, j);
			g_string_free(tmp_aspect.id, TRUE);
		}
		g_array_free(tmp_peripheral_mapping.aspects, TRUE);
	}
	g_array_free(board.peripherals, TRUE);

	for (size_t i = 0; i < board.segments->len; i++) {
		g_string_free(g_array_index(board.segments,
		                            t_bidib_segment_mapping, i).id, TRUE);
	}
	g_array_free(board.segments, TRUE);

	for (size_t i = 0; i < board.reversers->len; i++) {
		g_string_free(g_array_index(board.reversers,
		                            t_bidib_reverser_mapping, i).id, TRUE);
	}
	g_array_free(board.reversers, TRUE);
}

void bidib_state_free_single_train(t_bidib_train train) {
	if (train.id != NULL) {
		g_string_free(train.id, TRUE);
	}
	if (train.calibration != NULL) {
		g_array_free(train.calibration, TRUE);
	}
	if (train.peripherals != NULL) {
		for (size_t i = 0; i < train.peripherals->len; i++) {
			g_string_free(g_array_index(train.peripherals,
			                            t_bidib_train_peripheral_mapping, i).id, TRUE);
		}
		g_array_free(train.peripherals, TRUE);
	}
}


void bidib_state_free(void) {
	if (!bidib_running) {
		if (bidib_initial_values.points != NULL) {
			for (size_t i = 0; i < bidib_initial_values.points->len; i++) {
				bidib_state_free_single_initial_value(
						g_array_index(bidib_initial_values.points,
						              t_bidib_state_initial_value, i));
			}
			g_array_free(bidib_initial_values.points, TRUE);
			bidib_initial_values.points = NULL;
		}
		if (bidib_initial_values.signals != NULL) {
			for (size_t i = 0; i < bidib_initial_values.signals->len; i++) {
				bidib_state_free_single_initial_value(
						g_array_index(bidib_initial_values.signals,
						              t_bidib_state_initial_value, i));
			}
			g_array_free(bidib_initial_values.signals, TRUE);
			bidib_initial_values.signals = NULL;
		}
		if (bidib_initial_values.peripherals != NULL) {
			for (size_t i = 0; i < bidib_initial_values.peripherals->len; i++) {
				bidib_state_free_single_initial_value(
						g_array_index(bidib_initial_values.peripherals,
						              t_bidib_state_initial_value, i));
			}
			g_array_free(bidib_initial_values.peripherals, TRUE);
			bidib_initial_values.peripherals = NULL;
		}
		if (bidib_initial_values.trains != NULL) {
			for (size_t i = 0; i < bidib_initial_values.trains->len; i++) {
				bidib_state_free_single_train_initial_value(
						g_array_index(bidib_initial_values.trains,
						              t_bidib_state_train_initial_value, i));
			}
			g_array_free(bidib_initial_values.trains, TRUE);
			bidib_initial_values.trains = NULL;
		}

		if (bidib_track_state.points_board != NULL) {
			for (size_t i = 0; i < bidib_track_state.points_board->len; i++) {
				bidib_state_free_single_board_accessory_state(
						g_array_index(bidib_track_state.points_board,
						              t_bidib_board_accessory_state, i));
			}
			g_array_free(bidib_track_state.points_board, TRUE);
			bidib_track_state.points_board = NULL;

			for (size_t i = 0; i < bidib_track_state.points_dcc->len; i++) {
				bidib_state_free_single_dcc_accessory_state(
						g_array_index(bidib_track_state.points_dcc,
						              t_bidib_dcc_accessory_state, i));
			}
			g_array_free(bidib_track_state.points_dcc, TRUE);
			bidib_track_state.points_dcc = NULL;

			for (size_t i = 0; i < bidib_track_state.signals_board->len; i++) {
				bidib_state_free_single_board_accessory_state(
						g_array_index(bidib_track_state.signals_board,
						              t_bidib_board_accessory_state, i));
			}
			g_array_free(bidib_track_state.signals_board, TRUE);
			bidib_track_state.signals_board = NULL;

			for (size_t i = 0; i < bidib_track_state.signals_dcc->len; i++) {
				bidib_state_free_single_dcc_accessory_state(
						g_array_index(bidib_track_state.signals_dcc,
						              t_bidib_dcc_accessory_state, i));
			}
			g_array_free(bidib_track_state.signals_dcc, TRUE);
			bidib_track_state.signals_dcc = NULL;

			for (size_t i = 0; i < bidib_track_state.peripherals->len; i++) {
				bidib_state_free_single_peripheral_state(
						g_array_index(bidib_track_state.peripherals,
						              t_bidib_peripheral_state, i));
			}
			g_array_free(bidib_track_state.peripherals, TRUE);
			bidib_track_state.peripherals = NULL;

			for (size_t i = 0; i < bidib_track_state.segments->len; i++) {
				bidib_state_free_single_segment_state_intern(
						g_array_index(bidib_track_state.segments,
						              t_bidib_segment_state_intern, i));
			}
			g_array_free(bidib_track_state.segments, TRUE);
			bidib_track_state.segments = NULL;

			for (size_t i = 0; i < bidib_track_state.reversers->len; i++) {
				bidib_state_free_single_reverser_state(
						g_array_index(bidib_track_state.reversers,
						              t_bidib_reverser_state, i));
			}
			g_array_free(bidib_track_state.reversers, TRUE);
			bidib_track_state.reversers = NULL;

			for (size_t i = 0; i < bidib_track_state.trains->len; i++) {
				bidib_state_free_single_train_state_intern(
						g_array_index(bidib_track_state.trains,
						              t_bidib_train_state_intern, i));
			}
			g_array_free(bidib_track_state.trains, TRUE);
			bidib_track_state.trains = NULL;

			for (size_t i = 0; i < bidib_track_state.boosters->len; i++) {
				bidib_state_free_single_booster_state(
						g_array_index(bidib_track_state.boosters,
						              t_bidib_booster_state, i));
			}
			g_array_free(bidib_track_state.boosters, TRUE);
			bidib_track_state.boosters = NULL;

			for (size_t i = 0; i < bidib_track_state.track_outputs->len; i++) {
				bidib_state_free_single_track_output_state(
						g_array_index(bidib_track_state.track_outputs,
						              t_bidib_track_output_state, i));
			}
			g_array_free(bidib_track_state.track_outputs, TRUE);
			bidib_track_state.track_outputs = NULL;
		}

		if (bidib_boards != NULL) {
			for (size_t i = 0; i < bidib_boards->len; i++) {
				bidib_state_free_single_board(g_array_index(bidib_boards,
				                                            t_bidib_board, i));
			}
			g_array_free(bidib_boards, TRUE);
			bidib_boards = NULL;
		}

		if (bidib_trains != NULL) {
			for (size_t i = 0; i < bidib_trains->len; i++) {
				bidib_state_free_single_train(g_array_index(bidib_trains,
				                                            t_bidib_train, i));
			}
			g_array_free(bidib_trains, TRUE);
			bidib_trains = NULL;
		}
	}
}
