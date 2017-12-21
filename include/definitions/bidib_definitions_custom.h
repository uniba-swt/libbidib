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

#ifndef BIDIB_DEFINITIONS_CUSTOM_H
#define BIDIB_DEFINITIONS_CUSTOM_H

#include <stdbool.h>
#include <stddef.h>

#include "bidib_messages.h"


/**
 * The 0x00 byte is implicitly set at the bottom (subsubsub). Start at
 * top to define the byte on top of the stack.
 * If you don't need all address bytes, set the remaining bytes to 0x00.
 */
typedef struct {
	unsigned char top;
	unsigned char sub;
	unsigned char subsub;
} t_bidib_node_address;

typedef struct {
	bool known_and_connected;
	t_bidib_node_address address;
} t_bidib_node_address_query;

typedef struct {
	unsigned char name_length;
	unsigned char *name;
	unsigned char value_length;
	unsigned char *value;
} t_bidib_vendor_data;

typedef struct {
	unsigned char class_id;
	unsigned char class_id_ext;
	unsigned char vendor_id;
	unsigned char product_id1;
	unsigned char product_id2;
	unsigned char product_id3;
	unsigned char product_id4;
} t_bidib_unique_id_mod;

typedef struct {
	unsigned char addrl; /**< LSB of the address */
	unsigned char addrh; /**< MSB bytes of the address */
} t_bidib_dcc_address;

typedef struct {
	bool known;
	t_bidib_dcc_address dcc_address;
} t_bidib_dcc_address_query;

typedef struct {
	t_bidib_dcc_address dcc_address;
	unsigned char data;
	unsigned char time;
} t_bidib_cs_accessory_mod;

typedef struct {
	t_bidib_dcc_address dcc_address;
	unsigned char bin_numl;
	unsigned char bin_numh;
	unsigned char data;
} t_bidib_bin_state_mod;

typedef struct {
	t_bidib_dcc_address dcc_address;
	unsigned char dcc_format;
	unsigned char active;
	unsigned char speed;
	unsigned char function1;
	unsigned char function2;
	unsigned char function3;
	unsigned char function4;
} t_bidib_cs_drive_mod;

typedef struct {
	t_bidib_dcc_address dcc_address;
	unsigned char addrxl;
	unsigned char addrxh;
	unsigned char mid;
	unsigned char opcode;
	unsigned char cv_addrl;
	unsigned char cv_addrh;
	unsigned char cv_addrx;
	unsigned char data0;
	unsigned char data1;
	unsigned char data2;
	unsigned char data3;
} t_bidib_cs_pom_mod;

typedef struct {
	unsigned char opcode;
	unsigned char cv_addrl;
	unsigned char cv_addrh;
	unsigned char data;
} t_bidib_cs_prog_mod;

typedef struct {
	unsigned char data0;
	unsigned char data1;
	unsigned char data2;
	unsigned char data3;
	unsigned char data4;
	unsigned char data5;
} t_bidib_macro_params;

typedef struct {
	unsigned char start0;
	unsigned char start1;
	unsigned char end0;
	unsigned char end1;
} t_bidib_port_query_address_range;

typedef struct {
	unsigned char select0;
	unsigned char select1;
	t_bidib_port_query_address_range range;
} t_bidib_port_query_params;

typedef enum {
	BIDIB_EXEC_STATE_REACHED = 0x02,
	BIDIB_EXEC_STATE_REACHED_VERIFIED = 0x00,
	BIDIB_EXEC_STATE_NOTREACHED = 0x03,
	BIDIB_EXEC_STATE_NOTREACHED_VERIFIED = 0x01,
	BIDIB_EXEX_STATE_ERROR = 0x80
} t_bidib_accessory_execution_state;

typedef struct {
	char *state_id;
	unsigned char state_value;
	t_bidib_accessory_execution_state execution_state;
	unsigned char wait_details;
} t_bidib_board_accessory_state_data;

typedef struct {
	char *id;
	t_bidib_board_accessory_state_data data;
} t_bidib_board_accessory_state;

typedef enum {
	BIDIB_TIMEUNIT_MILLISECONDS,
	BIDIB_TIMEUNIT_SECONDS
} t_bidib_time_unit;

typedef enum {
	BIDIB_DCC_ACK_REJECTED,
	BIDIB_DCC_ACK_ACCEPTED_SOON,
	BIDIB_DCC_ACK_ACCEPTED_DELAY,
	BIDIB_DCC_ACK_OUTPUT,
	BIDIB_DCC_ACK_PENDING
} t_bidib_cs_ack;

typedef struct {
	char *state_id;
	unsigned char state_value;
	bool coil_on;
	bool output_controls_timing;
	t_bidib_cs_ack ack;
	t_bidib_time_unit time_unit;
	unsigned char switch_time; /**< Time unit specified in time_unit, 127s max */
} t_bidib_dcc_accessory_state_data;

typedef struct {
	char *id;
	t_bidib_dcc_accessory_state_data data;
} t_bidib_dcc_accessory_state;

typedef struct {
	char *state_id;
	unsigned char state_value;
	t_bidib_time_unit time_unit;
	unsigned char wait; /**< Time unit specified in time_unit, 127s max */
} t_bidib_peripheral_state_data;

typedef struct {
	char *id;
	t_bidib_peripheral_state_data data;
} t_bidib_peripheral_state;

typedef struct {
	bool available;
	t_bidib_peripheral_state_data data;
} t_bidib_peripheral_state_query;

typedef struct {
	bool conf_void;
	bool freeze;
	bool nosignal;
} t_bidib_segment_state_confidence;

typedef struct {
	bool known;
	bool overcurrent;
	unsigned int current; /**< in mA, only meaningful if
                           * known == true and overcurrent == false */
} t_bidib_power_consumption;

typedef struct {
	bool occupied;
	t_bidib_segment_state_confidence confidence;
	t_bidib_power_consumption power_consumption;
	size_t dcc_address_cnt;
	t_bidib_dcc_address *dcc_addresses;
} t_bidib_segment_state_data;

typedef struct {
	char *id;
	t_bidib_segment_state_data data;
} t_bidib_segment_state;

typedef struct {
	bool known;
	t_bidib_segment_state_data data;
} t_bidib_segment_state_query;

typedef struct {
	char *id;
	unsigned char state;
} t_bidib_train_peripheral_state;

typedef struct {
	bool available;
	unsigned char state;
} t_bidib_train_peripheral_state_query;

typedef enum {
	BIDIB_TRAIN_DIRECTION_FORWARD,
	BIDIB_TRAIN_DIRECTION_BACKWARD
} t_bidib_train_direction;

typedef struct {
	bool signal_quality_known;
	unsigned char signal_quality; /**< in percent, 0-100 */
	bool temp_known;
	signed char temp_celsius; /**< Discrete steps of one degree */
	bool energy_storage_known;
	unsigned char energy_storage; /**< in percent, 0-100 */
	bool container2_storage_known;
	unsigned char container2_storage;
	bool container3_storage_known;
	unsigned char container3_storage;
} t_bidib_train_decoder_state;

typedef struct {
	bool on_track;
	t_bidib_train_direction direction;
	int set_speed_step;
	t_bidib_cs_ack ack;
	int detected_kmh_speed;
	size_t peripheral_cnt;
	t_bidib_train_peripheral_state *peripherals;
	t_bidib_train_decoder_state decoder_state;
} t_bidib_train_state_data;

typedef struct {
	char *id;
	t_bidib_train_state_data data;
} t_bidib_train_state;

typedef struct {
	bool known;
	t_bidib_train_state_data data;
} t_bidib_train_state_query;

typedef enum {
	BIDIB_BSTR_OFF = BIDIB_BST_STATE_OFF,
	BIDIB_BSTR_OFF_SHORT = BIDIB_BST_STATE_OFF_SHORT,
	BIDIB_BSTR_OFF_HOT = BIDIB_BST_STATE_OFF_HOT,
	BIDIB_BSTR_OFF_NOPOWER = BIDIB_BST_STATE_OFF_NOPOWER,
	BIDIB_BSTR_OFF_GO_REQ = BIDIB_BST_STATE_OFF_GO_REQ,
	BIDIB_BSTR_OFF_HERE = BIDIB_BST_STATE_OFF_HERE,
	BIDIB_BSTR_OFF_NO_DCC = BIDIB_BST_STATE_OFF_NO_DCC,
	BIDIB_BSTR_ON = BIDIB_BST_STATE_ON,
	BIDIB_BSTR_ON_LIMIT = BIDIB_BST_STATE_ON_LIMIT,
	BIDIB_BSTR_ON_HOT = BIDIB_BST_STATE_ON_HOT,
	BIDIB_BSTR_ON_STOP_REQ = BIDIB_BST_STATE_ON_STOP_REQ,
	BIDIB_BSTR_ON_HERE = BIDIB_BST_STATE_ON_HERE
} t_bidib_booster_power_state;

typedef enum {
	BIDIB_BSTR_SIMPLE_ON,
	BIDIB_BSTR_SIMPLE_OFF,
	BIDIB_BSTR_SIMPLE_ERROR
} t_bidib_booster_power_state_simple;

typedef struct {
	t_bidib_booster_power_state power_state;
	t_bidib_booster_power_state_simple power_state_simple;
	t_bidib_power_consumption power_consumption;
	bool voltage_known;
	unsigned char voltage; /**< Voltage in 100 mV */
	bool temp_known;
	signed char temp_celsius; /**< Discrete steps of one degree */
} t_bidib_booster_state_data;

typedef struct {
	char *id;
	t_bidib_booster_state_data data;
} t_bidib_booster_state;

typedef struct {
	bool known;
	t_bidib_booster_state_data data;
} t_bidib_booster_state_query;

typedef enum {
	BIDIB_CS_OFF = BIDIB_CS_STATE_OFF,
	BIDIB_CS_STOP = BIDIB_CS_STATE_STOP,
	BIDIB_CS_SOFTSTOP = BIDIB_CS_STATE_SOFTSTOP,
	BIDIB_CS_GO = BIDIB_CS_STATE_GO,
	BIDIB_CS_GO_IGN_WD = BIDIB_CS_STATE_GO_IGN_WD,
	BIDIB_CS_PROG = BIDIB_CS_STATE_PROG,
	BIDIB_CS_PROGBUSY = BIDIB_CS_STATE_PROGBUSY,
	BIDIB_CS_BUSY = BIDIB_CS_STATE_BUSY,
	BIDIB_CS_QUERY = BIDIB_CS_STATE_QUERY
} t_bidib_cs_state;

typedef struct {
	char *id;
	t_bidib_cs_state cs_state;
} t_bidib_track_output_state;

typedef struct {
	bool known;
	t_bidib_cs_state cs_state;
} t_bidib_track_output_state_query;

typedef struct {
	size_t points_board_count;
	t_bidib_board_accessory_state *points_board;
	size_t points_dcc_count;
	t_bidib_dcc_accessory_state *points_dcc;
	size_t signals_board_count;
	t_bidib_board_accessory_state *signals_board;
	size_t signals_dcc_count;
	t_bidib_dcc_accessory_state *signals_dcc;
	size_t peripherals_count;
	t_bidib_peripheral_state *peripherals;
	size_t segments_count;
	t_bidib_segment_state *segments;
	size_t trains_count;
	t_bidib_train_state *trains;
	size_t booster_count;
	t_bidib_booster_state *booster;
	size_t track_outputs_count;
	t_bidib_track_output_state *track_outputs;
} t_bidib_track_state;

typedef enum {
	BIDIB_ACCESSORY_BOARD,
	BIDIB_ACCESSORY_DCC
} t_bidib_accessory_type;

typedef struct {
	bool known;
	t_bidib_accessory_type type;
	union {
		t_bidib_board_accessory_state_data board_accessory_state;
		t_bidib_dcc_accessory_state_data dcc_accessory_state;
	};
} t_bidib_unified_accessory_state_query;

typedef struct {
	bool known;
	char *id;
} t_bidib_id_query;

typedef struct {
	size_t length;
	char **ids;
} t_bidib_id_list_query;

typedef struct {
	unsigned char number;
	unsigned char value;
} t_bidib_board_feature;

typedef struct {
	size_t length;
	t_bidib_board_feature *features;
} t_bidib_board_features_query;

typedef struct {
	size_t length;
	char **segments;
} t_bidib_train_position_query;

typedef struct {
	bool known_and_avail;
	int speed_step;
} t_bidib_train_speed_step_query;

typedef struct {
	bool known_and_avail;
	int speed_kmh; /**< -1 means unknown speed */
} t_bidib_train_speed_kmh_query;

typedef struct {
	bool known;
	t_bidib_unique_id_mod unique_id;
} t_bidib_unique_id_query;

typedef struct {
	size_t length;
	t_bidib_unique_id_mod *unique_ids;
} t_bidib_unique_id_list_query;


#endif
