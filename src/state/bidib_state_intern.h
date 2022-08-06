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

#ifndef BIDIB_STATE_INTERN_H
#define BIDIB_STATE_INTERN_H

#include <glib.h>
#include <stdint.h>

#include "../../include/definitions/bidib_definitions_custom.h"


typedef struct {
	GArray *points_board;
	GArray *points_dcc;
	GArray *signals_board;
	GArray *signals_dcc;
	GArray *peripherals;
	GArray *segments;
	GArray *trains;
	GArray *boosters;
	GArray *track_outputs;
} t_bidib_track_state_intern;

typedef struct {
	GString *id;
	bool on_track;
	t_bidib_train_orientation orientation;
	int set_speed_step;
	t_bidib_cs_ack ack;
	unsigned int detected_kmh_speed;
	GArray *peripherals;
	t_bidib_train_decoder_state decoder_state;
} t_bidib_train_state_intern;

typedef struct {
	GString *id;
	GString *length;
	bool occupied;
	t_bidib_segment_state_confidence confidence;
	t_bidib_power_consumption power_consumption;
	GArray *dcc_addresses;
} t_bidib_segment_state_intern;

typedef struct {
	GString *id;
	uint8_t bit;
} t_bidib_train_peripheral_mapping;

typedef struct {
	GString *id;
	t_bidib_dcc_address dcc_addr;
	uint8_t dcc_speed_steps;
	GArray *calibration;
	GArray *peripherals;
} t_bidib_train;

typedef struct {
	GString *id;
	uint8_t value;
} t_bidib_aspect;

typedef struct {
	uint8_t port;
	uint8_t value;
} t_bidib_dcc_aspect_port_value;

typedef struct {
	GString *id;
	GArray *port_values;
} t_bidib_dcc_aspect;

typedef struct {
	GString *id;
	uint8_t number;
	GArray *aspects;
} t_bidib_board_accessory_mapping;

typedef struct {
	GString *id;
	t_bidib_dcc_address dcc_addr;
	uint8_t extended_accessory;
	GArray *aspects;
} t_bidib_dcc_accessory_mapping;

typedef struct {
	uint8_t port0;
	uint8_t port1;
} t_bidib_peripheral_port;

typedef struct {
	GString *id;
	uint8_t number;
	t_bidib_peripheral_port port;
	GArray *aspects;
} t_bidib_peripheral_mapping;

typedef struct {
	GString *id;
	uint8_t addr;
} t_bidib_segment_mapping;

typedef struct {
	GString *id;
	t_bidib_unique_id_mod unique_id;
	bool connected;
	t_bidib_node_address node_addr;
	bool secack_on;
	GArray *features;
	GArray *points_board;
	GArray *points_dcc;
	GArray *signals_board;
	GArray *signals_dcc;
	GArray *peripherals;
	GArray *segments;
} t_bidib_board;

typedef struct {
	GString *id;
	GString *value;
} t_bidib_state_initial_value;

typedef struct {
	GString *train;
	GString *id;
	uint8_t value;
} t_bidib_state_train_initial_value;

typedef struct {
	GArray *points;
	GArray *signals;
	GArray *peripherals;
	GArray *trains;
} t_bidib_state_initial_values;

extern pthread_rwlock_t bidib_state_trains_rwlock;
extern pthread_rwlock_t bidib_state_track_rwlock;
extern pthread_rwlock_t bidib_state_boards_rwlock;

extern t_bidib_state_initial_values bidib_initial_values;
extern t_bidib_track_state_intern bidib_track_state;
extern GArray *bidib_boards;
extern GArray *bidib_trains;


/**
 * Reads the configs and initializes the allocation table.
 *
 * @param config_dir the directory in which the config files are stored.
 * @return 0 if successful, otherwise 1.
 */
int bidib_state_init(const char *config_dir);

/**
 * Initializes the allocation table.
 */
void bidib_state_init_allocation_table(void);

/**
 * Queries the occupancy states of all segments.
 */
void bidib_state_query_occupancy(void);

/**
 * Converts a booster power state to a simple booster power state.
 *
 * @param state the power state.
 * @return the simplified power state.
 */
t_bidib_booster_power_state_simple bidib_booster_normal_to_simple(
		t_bidib_booster_power_state state);

/**
 * Converts a bidib speed to the format for the library.
 *
 * @param speed the speed in bidib format.
 * @return the speed in a range from -126...126.
 */
int bidib_dcc_speed_to_lib_format(uint8_t speed);

/**
 * Converts a lib speed to bidib format.
 *
 * @param speed the speed in lib format.
 * @return the speed in bidib format.
 */
uint8_t bidib_lib_speed_to_dcc_format(int speed);

/**
 * Converts the void, freeze and nosignal booster confidence values to
 * a confidence alert level.
 *
 * @param confidence the void, freeze, and nosignal confidence values.
 * @return the confidence level.
 */
t_bidib_bm_confidence_level bidib_bm_confidence_to_level(t_bidib_segment_state_confidence confidence);

/**
 * Sets the features of the boards according to the config file.
 */
void bidib_state_set_board_features(void);

/**
 * Sets the initial values for all accessories and peripherals.
 */
void bidib_state_set_initial_values(void);

/**
 * Resets the track state to default/initial values.
 */
void bidib_state_reset(void);

/**
 * Frees the memory allocated by the config/allocation table.
 */
void bidib_state_free(void);

/**
 * Checks whether two unique ids are equal.
 *
 * @param uid1 unique id 1.
 * @param uid2 unique id 2.
 * @return true if equal, otherwise false.
 */
bool bidib_state_uids_equal(const t_bidib_unique_id_mod *const uid1,
                            const t_bidib_unique_id_mod *const uid2);

/**
 * Adds a board to the current state.
 *
 * @param board the new board.
 * @return true if NULL or board already exists, otherwise false.
 */
bool bidib_state_add_board(t_bidib_board board);

/**
 * Frees the memory allocated by a t_bidib_board.
 *
 * @param board the board.
 */
void bidib_state_free_single_board(t_bidib_board board);

/**
 * Adds a booster to the current state.
 *
 * @param booster_state the new booster.
 */
void bidib_state_add_booster(t_bidib_booster_state booster_state);

/**
 * Add a track output state to the current state.
 *
 * @param track_output_state the new track output state.
 */
void bidib_state_add_track_output(t_bidib_track_output_state track_output_state);

/**
 * Frees the memory allocated by a booster state.
 *
 * @param booster_state the booster state.
 */
void bidib_state_free_single_booster_state(t_bidib_booster_state booster_state);

/**
 * Frees the memory allocated by a track output state.
 *
 * @param to_state the track output state.
 */
void bidib_state_free_single_track_output_state(t_bidib_track_output_state to_state);

/**
 * Adds a board point state to the track state.
 *
 * @param point_state the new point state.
 * @return true if NULL or point already exists, otherwise false.
 */
bool bidib_state_add_board_point_state(t_bidib_board_accessory_state point_state);

/**
 * Adds a board signal state to the track state.
 *
 * @param signal_state the new signal state.
 * @return true if NULL or signal already exists, otherwise false.
 */
bool bidib_state_add_board_signal_state(t_bidib_board_accessory_state signal_state);

/**
 * Frees the memory allocated by a board accessory state.
 *
 * @param accessory_state the accessory state.
 */
void bidib_state_free_single_board_accessory_state(t_bidib_board_accessory_state accessory_state);

/**
 * Add a dcc point state to the track state.
 *
 * @param point_state the new point state.
 * @param dcc_address the dcc address of the point.
 * @return true if NULL or point already exists, otherwise false.
 */
bool bidib_state_add_dcc_point_state(t_bidib_dcc_accessory_state point_state,
                                     t_bidib_dcc_address dcc_address);

/**
 * Adds a dcc signal state to the track state.
 *
 * @param signal_state the new signal state.
 * @param dcc_address the dcc address of the signal.
 * @return true if NULL or signal already exists, otherwise false.
 */
bool bidib_state_add_dcc_signal_state(t_bidib_dcc_accessory_state signal_state,
                                      t_bidib_dcc_address dcc_address);

/**
 * Frees the memory allocated by a dcc accessory state.
 *
 * @param accessory_state the accessory state.
 */
void bidib_state_free_single_dcc_accessory_state(t_bidib_dcc_accessory_state accessory_state);

/**
 * Adds a peripheral state to the track state.
 *
 * @param peripheral_state the new peripheral state.
 * @return true if NULL or peripheral already exists, otherwise false.
 */
bool bidib_state_add_peripheral_state(t_bidib_peripheral_state peripheral_state);

/**
 * Frees the memory allocated by a peripheral state.
 *
 * @param peripheral_state the peripheral state.
 */
void bidib_state_free_single_peripheral_state(t_bidib_peripheral_state peripheral_state);

/**
 * Adds a segment state to the track state.
 *
 * @param segment_state the new segment state.
 * @return true if NULL or segment already exists, otherwise false.
 */
bool bidib_state_add_segment_state(t_bidib_segment_state_intern segment_state);

/**
 * Frees the memory allocated by a segment state.
 *
 * @param segment_state the segment state.
 */
void bidib_state_free_single_segment_state(t_bidib_segment_state segment_state);

/**
 * Frees the memory allocated by a intern segment state.
 *
 * @param segment_state the intern segment state.
 */
void bidib_state_free_single_segment_state_intern(t_bidib_segment_state_intern segment_state);

/**
 * Checks whether a dcc address is already used by a train, point or signal.
 * Must only be called with bidib_state_trains_rwlock >=read acquired.
 *
 * @param dcc_address the dcc address which should be checked.
 * @return true if the dcc address is already in use, otherwise false.
 */
bool bidib_state_dcc_addr_in_use(t_bidib_dcc_address dcc_address);

/**
 * Adds a train to the current state.
 *
 * @param train the new train.
 * @return true if NULL or train already exists, otherwise false.
 */
bool bidib_state_add_train(t_bidib_train train);

/**
 * Adds a train state to the track state.
 *
 * @param train_state the new train state.
 */
void bidib_state_add_train_state(t_bidib_train_state_intern train_state);

/**
 * Adds an initial point value.
 *
 * @param value pointer to the value.
 */
void bidib_state_add_initial_point_value(t_bidib_state_initial_value value);

/**
 * Adds an initial signal value.
 *
 * @param value pointer to the value.
 */
void bidib_state_add_initial_signal_value(t_bidib_state_initial_value value);

/**
 * Adds an initial peripheral value.
 *
 * @param value pointer to the value.
 */
void bidib_state_add_initial_peripheral_value(t_bidib_state_initial_value value);

/**
 * Adds an initial train light value.
 *
 * @param value pointer to the value.
 */
void bidib_state_add_initial_train_value(t_bidib_state_train_initial_value value);

/**
 * Updates the available state for all trains.
 * Must only be called with bidib_state_trains_rwlock >=read acquired,
 * and bidib_state_track_rwlock write acquired.
 */
void bidib_state_update_train_available(void);

/**
 * Frees the memory allocated by a train.
 *
 * @param train the train.
 */
void bidib_state_free_single_train(t_bidib_train train);

/**
 * Frees the memory allocated by a train state.
 *
 * @param train_state the train state.
 */
void bidib_state_free_single_train_state(t_bidib_train_state train_state);

/**
 * Frees the memory allocated by a intern train state.
 *
 * @param train_state the intern train state.
 */
void bidib_state_free_single_train_state_intern(t_bidib_train_state_intern train_state);

/**
 * Frees the memory allocated by a initial value.
 *
 * @param value the initial value.
 */
void bidib_state_free_single_initial_value(t_bidib_state_initial_value value);

/**
 * Frees the memory allocated by a initial train value.
 *
 * @param value the initial train value.
 */
void bidib_state_free_single_train_initial_value(t_bidib_state_train_initial_value value);

/**
 * Resets the parameters for all trains, e.g. speed and peripherals.
 */
void bidib_state_reset_train_params(void);


#endif
