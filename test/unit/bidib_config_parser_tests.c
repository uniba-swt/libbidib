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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <yaml.h>
#include <syslog.h>

#include "../../src/parser/bidib_config_parser_intern.h"
#include "../../include/bidib.h"
#include "../../include/definitions/bidib_definitions_custom.h"


static void no_parser_errors(void **state __attribute__((unused))) {
	assert_int_equal(bidib_state_init("../test/unit/config_tests_config"), 0);
}

static void board_config_correctly_parsed(void **state __attribute__((unused))) {
	t_bidib_id_list_query boards_query = bidib_get_boards();
	assert_int_equal(boards_query.length, 3);
	assert_string_equal(boards_query.ids[0], "board1");
	assert_string_equal(boards_query.ids[1], "board2");
	assert_string_equal(boards_query.ids[2], "board3");
	bidib_free_id_list_query(boards_query);

	t_bidib_unique_id_query uid_query = bidib_get_uniqueid("board1");
	assert_int_equal(uid_query.known, true);
	assert_int_equal(uid_query.unique_id.class_id, 0x02);
	assert_int_equal(uid_query.unique_id.class_id_ext, 0x23);
	assert_int_equal(uid_query.unique_id.vendor_id, 0x45);
	assert_int_equal(uid_query.unique_id.product_id1, 0x67);
	assert_int_equal(uid_query.unique_id.product_id2, 0x89);
	assert_int_equal(uid_query.unique_id.product_id3, 0xAB);
	assert_int_equal(uid_query.unique_id.product_id4, 0xCD);
	uid_query = bidib_get_uniqueid("board2");
	assert_int_equal(uid_query.known, true);
	assert_int_equal(uid_query.unique_id.class_id, 0x01);
	assert_int_equal(uid_query.unique_id.class_id_ext, 0x23);
	assert_int_equal(uid_query.unique_id.vendor_id, 0x45);
	assert_int_equal(uid_query.unique_id.product_id1, 0x67);
	assert_int_equal(uid_query.unique_id.product_id2, 0x89);
	assert_int_equal(uid_query.unique_id.product_id3, 0xAB);
	assert_int_equal(uid_query.unique_id.product_id4, 0xCE);
	uid_query = bidib_get_uniqueid("board3");
	assert_int_equal(uid_query.known, true);
	assert_int_equal(uid_query.unique_id.class_id, 0xF4);
	assert_int_equal(uid_query.unique_id.class_id_ext, 0x12);
	assert_int_equal(uid_query.unique_id.vendor_id, 0x74);
	assert_int_equal(uid_query.unique_id.product_id1, 0xA8);
	assert_int_equal(uid_query.unique_id.product_id2, 0xE5);
	assert_int_equal(uid_query.unique_id.product_id3, 0x6B);
	assert_int_equal(uid_query.unique_id.product_id4, 0x93);

	t_bidib_board_features_query features_query = bidib_get_board_features("board1");
	assert_int_equal(features_query.length, 2);
	assert_non_null(features_query.features);
	assert_int_equal(features_query.features[0].number, 1);
	assert_int_equal(features_query.features[0].value, 0);
	assert_int_equal(features_query.features[1].number, 4);
	assert_int_equal(features_query.features[1].value, 1);
	bidib_free_board_features_query(features_query);
	features_query = bidib_get_board_features("board2");
	assert_int_equal(features_query.length, 0);
	assert_null(features_query.features);
	bidib_free_board_features_query(features_query);
	features_query = bidib_get_board_features("board3");
	assert_int_equal(features_query.length, 1);
	assert_non_null(features_query.features);
	assert_int_equal(features_query.features[0].number, 1);
	assert_int_equal(features_query.features[0].value, 0);
	bidib_free_board_features_query(features_query);
}

static void track_config_correctly_parsed(void **state __attribute__((unused))) {
	t_bidib_id_list_query query = bidib_get_board_points("board1");
	assert_int_equal(query.length, 3);
	assert_string_equal(query.ids[0], "point1");
	assert_string_equal(query.ids[1], "point2");
	assert_string_equal(query.ids[2], "point3");
	bidib_free_id_list_query(query);

	t_bidib_unified_accessory_state_query acc_query = bidib_get_point_state("point1");
	assert_int_equal(acc_query.known, true);
	assert_int_equal(acc_query.type, BIDIB_ACCESSORY_BOARD);
	assert_string_equal(acc_query.board_accessory_state.state_id, "unknown");
	assert_int_equal(acc_query.board_accessory_state.state_value, 0x00);
	assert_int_equal(acc_query.board_accessory_state.execution_state, BIDIB_EXEC_STATE_REACHED);
	assert_int_equal(acc_query.board_accessory_state.wait_details, 0x00);
	query = bidib_get_point_aspects("point1");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "normal");
	assert_string_equal(query.ids[1], "reverse");
	bidib_free_id_list_query(query);

	bidib_free_unified_accessory_state_query(acc_query);
	acc_query = bidib_get_point_state("point2");
	assert_int_equal(acc_query.known, true);
	assert_int_equal(acc_query.type, BIDIB_ACCESSORY_BOARD);
	assert_string_equal(acc_query.board_accessory_state.state_id, "unknown");
	assert_int_equal(acc_query.board_accessory_state.state_value, 0x00);
	assert_int_equal(acc_query.board_accessory_state.execution_state, BIDIB_EXEC_STATE_REACHED);
	assert_int_equal(acc_query.board_accessory_state.wait_details, 0x00);
	bidib_free_unified_accessory_state_query(acc_query);
	query = bidib_get_point_aspects("point2");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "normal");
	assert_string_equal(query.ids[1], "reverse");
	bidib_free_id_list_query(query);

	acc_query = bidib_get_point_state("point3");
	assert_int_equal(acc_query.known, true);
	assert_int_equal(acc_query.type, BIDIB_ACCESSORY_DCC);
	assert_string_equal(acc_query.board_accessory_state.state_id, "unknown");
	assert_int_equal(acc_query.dcc_accessory_state.state_value, 0x00);
	assert_int_equal(acc_query.dcc_accessory_state.time_unit, BIDIB_TIMEUNIT_MILLISECONDS);
	assert_int_equal(acc_query.dcc_accessory_state.switch_time, 0x00);
	bidib_free_unified_accessory_state_query(acc_query);
	query = bidib_get_point_aspects("point3");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "normal");
	assert_string_equal(query.ids[1], "reverse");
	bidib_free_id_list_query(query);

	query = bidib_get_board_points("board2");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);
	query = bidib_get_board_points("board3");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_board_signals("board1");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "signal1");
	assert_string_equal(query.ids[1], "signal2");
	bidib_free_id_list_query(query);
	acc_query = bidib_get_signal_state("signal1");
	assert_int_equal(acc_query.known, true);
	assert_int_equal(acc_query.type, BIDIB_ACCESSORY_BOARD);
	assert_string_equal(acc_query.board_accessory_state.state_id, "unknown");
	assert_int_equal(acc_query.board_accessory_state.state_value, 0x00);
	assert_int_equal(acc_query.board_accessory_state.execution_state, BIDIB_EXEC_STATE_REACHED);
	assert_int_equal(acc_query.board_accessory_state.wait_details, 0x00);
	bidib_free_unified_accessory_state_query(acc_query);
	query = bidib_get_signal_aspects("signal1");
	assert_int_equal(query.length, 3);
	assert_string_equal(query.ids[0], "green");
	assert_string_equal(query.ids[1], "orange");
	assert_string_equal(query.ids[2], "red");
	bidib_free_id_list_query(query);

	acc_query = bidib_get_signal_state("signal2");
	assert_int_equal(acc_query.known, true);
	assert_int_equal(acc_query.type, BIDIB_ACCESSORY_DCC);
	assert_string_equal(acc_query.board_accessory_state.state_id, "unknown");
	assert_int_equal(acc_query.dcc_accessory_state.state_value, 0x00);
	assert_int_equal(acc_query.dcc_accessory_state.time_unit, BIDIB_TIMEUNIT_MILLISECONDS);
	assert_int_equal(acc_query.dcc_accessory_state.switch_time, 0x00);
	bidib_free_unified_accessory_state_query(acc_query);
	query = bidib_get_signal_aspects("signal2");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "green");
	assert_string_equal(query.ids[1], "red");
	bidib_free_id_list_query(query);

	query = bidib_get_board_signals("board2");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);
	query = bidib_get_board_signals("board3");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_board_peripherals("board1");
	assert_int_equal(query.length, 1);
	assert_string_equal(query.ids[0], "led1");
	bidib_free_id_list_query(query);
	t_bidib_peripheral_state_query per_query = bidib_get_peripheral_state("led1");
	assert_int_equal(per_query.available, true);
	assert_string_equal(per_query.data.state_id, "unknown");
	assert_int_equal(per_query.data.state_value, 0x00);
	assert_int_equal(per_query.data.time_unit, BIDIB_TIMEUNIT_MILLISECONDS);
	assert_int_equal(per_query.data.wait, 0x00);
	bidib_free_peripheral_state_query(per_query);
	query = bidib_get_peripheral_aspects("led1");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "state1");
	assert_string_equal(query.ids[1], "state2");
	bidib_free_id_list_query(query);

	query = bidib_get_board_peripherals("board2");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "led2");
	assert_string_equal(query.ids[1], "led3");
	bidib_free_id_list_query(query);
	per_query = bidib_get_peripheral_state("led2");
	assert_int_equal(per_query.available, true);
	assert_string_equal(per_query.data.state_id, "unknown");
	assert_int_equal(per_query.data.state_value, 0x00);
	assert_int_equal(per_query.data.time_unit, BIDIB_TIMEUNIT_MILLISECONDS);
	assert_int_equal(per_query.data.wait, 0x00);
	bidib_free_peripheral_state_query(per_query);
	query = bidib_get_peripheral_aspects("led2");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "state1");
	assert_string_equal(query.ids[1], "state2");
	bidib_free_id_list_query(query);

	per_query = bidib_get_peripheral_state("led3");
	assert_int_equal(per_query.available, true);
	assert_string_equal(per_query.data.state_id, "unknown");
	assert_int_equal(per_query.data.state_value, 0x00);
	assert_int_equal(per_query.data.time_unit, BIDIB_TIMEUNIT_MILLISECONDS);
	assert_int_equal(per_query.data.wait, 0x00);
	bidib_free_peripheral_state_query(per_query);
	query = bidib_get_peripheral_aspects("led3");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "state1");
	assert_string_equal(query.ids[1], "state2");
	bidib_free_id_list_query(query);

	query = bidib_get_board_peripherals("board3");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_board_segments("board1");
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "seg1");
	assert_string_equal(query.ids[1], "seg2");
	bidib_free_id_list_query(query);
	t_bidib_segment_state_query seg_query = bidib_get_segment_state("seg1");
	assert_int_equal(seg_query.known, true);
	assert_int_equal(seg_query.data.occupied, false);
	assert_int_equal(seg_query.data.confidence.conf_void, false);
	assert_int_equal(seg_query.data.confidence.freeze, false);
	assert_int_equal(seg_query.data.confidence.nosignal, false);
	assert_int_equal(seg_query.data.power_consumption.known, false);
	assert_int_equal(seg_query.data.dcc_address_cnt, 0);
	bidib_free_segment_state_query(seg_query);
	seg_query = bidib_get_segment_state("seg2");
	assert_int_equal(seg_query.known, true);
	assert_int_equal(seg_query.data.occupied, false);
	assert_int_equal(seg_query.data.confidence.conf_void, false);
	assert_int_equal(seg_query.data.confidence.freeze, false);
	assert_int_equal(seg_query.data.confidence.nosignal, false);
	assert_int_equal(seg_query.data.power_consumption.known, false);
	assert_int_equal(seg_query.data.dcc_address_cnt, 0);
	bidib_free_segment_state_query(seg_query);
	query = bidib_get_board_segments("board2");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);
	query = bidib_get_board_segments("board3");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_boosters();
	assert_int_equal(query.length, 1);
	assert_string_equal(query.ids[0], "board1");
	bidib_free_id_list_query(query);
	t_bidib_booster_state_query booster_state = bidib_get_booster_state("board1");
	assert_int_equal(booster_state.known, true);
	assert_int_equal(booster_state.data.power_state, BIDIB_BSTR_OFF);
	assert_int_equal(booster_state.data.power_state_simple, BIDIB_BSTR_SIMPLE_OFF);
	assert_int_equal(booster_state.data.power_consumption.known, false);
	assert_int_equal(booster_state.data.voltage_known, false);
	assert_int_equal(booster_state.data.temp_known, false);

	query = bidib_get_connected_points();
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_connected_signals();
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_connected_peripherals();
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_connected_segments();
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	query = bidib_get_connected_boosters();
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);
}

static void train_config_correctly_parsed(void **state __attribute__((unused))) {
	t_bidib_id_list_query query = bidib_get_trains();
	assert_int_equal(query.length, 2);
	assert_string_equal(query.ids[0], "train1");
	assert_string_equal(query.ids[1], "train2");
	bidib_free_id_list_query(query);
	query = bidib_get_trains_on_track();
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	t_bidib_dcc_address_query dcc_query = bidib_get_train_dcc_addr("train1");
	assert_int_equal(dcc_query.known, true);
	assert_int_equal(dcc_query.dcc_address.addrh, 0x01);
	assert_int_equal(dcc_query.dcc_address.addrl, 0x23);
	t_bidib_id_query id_query = bidib_get_train_id(dcc_query.dcc_address);
	assert_int_equal(id_query.known, true);
	assert_string_equal(id_query.id, "train1");
	bidib_free_id_query(id_query);
	dcc_query = bidib_get_train_dcc_addr("train2");
	assert_int_equal(dcc_query.known, true);
	assert_int_equal(dcc_query.dcc_address.addrh, 0x45);
	assert_int_equal(dcc_query.dcc_address.addrl, 0x67);
	id_query = bidib_get_train_id(dcc_query.dcc_address);
	assert_int_equal(id_query.known, true);
	assert_string_equal(id_query.id, "train2");
	bidib_free_id_query(id_query);
	dcc_query = bidib_get_train_dcc_addr("train3");
	assert_int_equal(dcc_query.known, false);
	t_bidib_dcc_address dcc_address = {0x01, 0x01, 0x00};
	id_query = bidib_get_train_id(dcc_address);
	assert_int_equal(id_query.known, false);
	bidib_free_id_query(id_query);

	query = bidib_get_train_peripherals("train1");
	assert_int_equal(query.length, 3);
	assert_string_equal(query.ids[0], "light1");
	assert_string_equal(query.ids[1], "light2");
	assert_string_equal(query.ids[2], "horn");
	bidib_free_id_list_query(query);
	t_bidib_train_peripheral_state_query train_peripheral_state;
	train_peripheral_state = bidib_get_train_peripheral_state("train1", "light1");
	assert_int_equal(train_peripheral_state.available, true);
	assert_int_equal(train_peripheral_state.state, 0x00);
	train_peripheral_state = bidib_get_train_peripheral_state("train1", "light2");
	assert_int_equal(train_peripheral_state.available, true);
	assert_int_equal(train_peripheral_state.state, 0x00);
	train_peripheral_state = bidib_get_train_peripheral_state("train1", "horn");
	assert_int_equal(train_peripheral_state.available, true);
	assert_int_equal(train_peripheral_state.state, 0x00);
	train_peripheral_state = bidib_get_train_peripheral_state("train1", "light3");
	assert_int_equal(train_peripheral_state.available, false);
	query = bidib_get_train_peripherals("train2");
	assert_int_equal(query.length, 1);
	assert_string_equal(query.ids[0], "light");
	bidib_free_id_list_query(query);
	train_peripheral_state = bidib_get_train_peripheral_state("train2", "light");
	assert_int_equal(train_peripheral_state.available, true);
	assert_int_equal(train_peripheral_state.state, 0x00);
	query = bidib_get_train_peripherals("train3");
	assert_int_equal(query.length, 0);
	bidib_free_id_list_query(query);

	t_bidib_train_speed_step_query speed_step_query = bidib_get_train_speed_step("train1");
	assert_int_equal(speed_step_query.known_and_avail, false);
	speed_step_query = bidib_get_train_speed_step("train2");
	assert_int_equal(speed_step_query.known_and_avail, false);
	speed_step_query = bidib_get_train_speed_step("train3");
	assert_int_equal(speed_step_query.known_and_avail, false);

	t_bidib_train_speed_kmh_query speed_kmh_query = bidib_get_train_speed_kmh("train1");
	assert_int_equal(speed_kmh_query.known_and_avail, false);
	speed_kmh_query = bidib_get_train_speed_kmh("train2");
	assert_int_equal(speed_kmh_query.known_and_avail, false);
	speed_kmh_query = bidib_get_train_speed_kmh("train3");
	assert_int_equal(speed_kmh_query.known_and_avail, false);

	t_bidib_train_position_query position_query = bidib_get_train_position("train1");
	assert_int_equal(position_query.length, 0);
	bidib_free_train_position_query(position_query);
	position_query = bidib_get_train_position("train2");
	assert_int_equal(position_query.length, 0);
	bidib_free_train_position_query(position_query);
	position_query = bidib_get_train_position("train3");
	assert_int_equal(position_query.length, 0);
	bidib_free_train_position_query(position_query);

	t_bidib_train_state_query train_state = bidib_get_train_state("train1");
	assert_int_equal(train_state.known, true);
	assert_int_equal(train_state.data.on_track, false);
	assert_int_equal(train_state.data.decoder_state.signal_quality_known, false);
	assert_int_equal(train_state.data.decoder_state.temp_known, false);
	assert_int_equal(train_state.data.decoder_state.energy_storage_known, false);
	assert_int_equal(train_state.data.decoder_state.container2_storage_known, false);
	assert_int_equal(train_state.data.decoder_state.container3_storage_known, false);
	bidib_free_train_state_query(train_state);
	train_state = bidib_get_train_state("train2");
	assert_int_equal(train_state.known, true);
	assert_int_equal(train_state.data.on_track, false);
	assert_int_equal(train_state.data.decoder_state.signal_quality_known, false);
	assert_int_equal(train_state.data.decoder_state.temp_known, false);
	assert_int_equal(train_state.data.decoder_state.energy_storage_known, false);
	assert_int_equal(train_state.data.decoder_state.container2_storage_known, false);
	assert_int_equal(train_state.data.decoder_state.container3_storage_known, false);
	bidib_free_train_state_query(train_state);
	train_state = bidib_get_train_state("train3");
	assert_int_equal(train_state.known, false);
	bidib_free_train_state_query(train_state);
}

static void overall_state_generated_correctly(void **state __attribute__((unused))) {
	t_bidib_track_state track_state = bidib_get_state();
	assert_int_equal(track_state.points_board_count, 2);
	assert_string_equal(track_state.points_board[0].id, "point1");
	assert_string_equal(track_state.points_board[1].id, "point2");

	assert_int_equal(track_state.points_dcc_count, 1);
	assert_string_equal(track_state.points_dcc[0].id, "point3");

	assert_int_equal(track_state.signals_board_count, 1);
	assert_string_equal(track_state.signals_board[0].id, "signal1");

	assert_int_equal(track_state.signals_dcc_count, 1);
	assert_string_equal(track_state.signals_dcc[0].id, "signal2");

	assert_int_equal(track_state.peripherals_count, 3);
	assert_string_equal(track_state.peripherals[0].id, "led1");
	assert_string_equal(track_state.peripherals[1].id, "led2");
	assert_string_equal(track_state.peripherals[2].id, "led3");

	assert_int_equal(track_state.segments_count, 2);
	assert_string_equal(track_state.segments[0].id, "seg1");
	assert_string_equal(track_state.segments[1].id, "seg2");

	assert_int_equal(track_state.trains_count, 2);
	assert_string_equal(track_state.trains[0].id, "train1");
	assert_string_equal(track_state.trains[1].id, "train2");

	assert_int_equal(track_state.booster_count, 1);
	assert_string_equal(track_state.booster[0].id, "board1");

	assert_int_equal(track_state.track_outputs_count, 1);
	assert_string_equal(track_state.track_outputs[0].id, "board3");

	bidib_free_track_state(track_state);
}

int main(void) {
	openlog("swtbahn", 0, LOG_LOCAL0);
	syslog(LOG_INFO, "bidib_config_parser_tests: %s", "Config-parser tests started");
	const struct CMUnitTest tests[] = {
			cmocka_unit_test(no_parser_errors),
			cmocka_unit_test(board_config_correctly_parsed),
			cmocka_unit_test(track_config_correctly_parsed),
			cmocka_unit_test(train_config_correctly_parsed),
			cmocka_unit_test(overall_state_generated_correctly)
	};
	int ret = cmocka_run_group_tests(tests, NULL, NULL);
	bidib_state_free();
	syslog(LOG_INFO, "bidib_config_parser_tests: %s", "Config-parser tests stopped");
	closelog();
	return ret;
}
