#include <stdio.h>
#include <unistd.h>
#include <bidib.h>
#include <stdlib.h>

#include <glib.h>

/*
 * Small test program to check if the library is working with a real bidib
 * system. The system must be set up corresponding to the config files in
 * 'project_root/test/system_tests_config'.
 */


void print_queue(unsigned char *(*pop_msg)(void)) {
	unsigned char *msg = NULL;
	while ((msg = pop_msg())) {
		printf(" -");
		for (size_t i = 0; i <= msg[0]; i++) {
			printf(" 0x%x", msg[i]);
		}
		printf("\n");
		free(msg);
	}
}

void print_board_accessory_state_query(t_bidib_unified_accessory_state_query query) {
	if (query.known && query.type == BIDIB_ACCESSORY_BOARD) {
		printf("state: %s (0x%02x) exec: 0x%02x\n",
			   query.board_accessory_state.state_id, 
			   query.board_accessory_state.state_value, 
			   query.board_accessory_state.execution_state);
	}
}

void print_points(void) {
		GString *points = g_string_new("");
		t_bidib_id_list_query query = bidib_get_connected_points();
		for (size_t i = 0; i < query.length; i++) {
			t_bidib_unified_accessory_state_query point_state =
				bidib_get_point_state(query.ids[i]);
			
			GString *execution_state = g_string_new("");
			if (point_state.type == BIDIB_ACCESSORY_BOARD) {
				g_string_printf(execution_state, "(target state%s reached)", 
				                point_state.board_accessory_state.execution_state ? 
				                " not" : "");
			}
			char execution_state_str[execution_state->len + 1];
			strcpy(execution_state_str, execution_state->str);
			g_string_free(execution_state, true);
			
			g_string_append_printf(points, "%s%s - state: %s %s",
			                       i != 0 ? "\n" : "", query.ids[i],
			                       point_state.type == BIDIB_ACCESSORY_BOARD ?
			                       point_state.board_accessory_state.state_id :
			                       point_state.dcc_accessory_state.state_id,
			                       execution_state_str);
			bidib_free_unified_accessory_state_query(point_state);
		}
		bidib_free_id_list_query(query);
		char response[points->len + 1];
		strcpy(response, points->str);
		g_string_free(points, true);

		printf("%s\n", response);
}

void print_segment_state_query(t_bidib_segment_state_query seg_state_query) {
	if (seg_state_query.known) {
		printf("occ: %s", seg_state_query.data.occupied ? "yes" : "no");
		
		t_bidib_id_query id_query;
		for (size_t i = 0; i < seg_state_query.data.dcc_address_cnt; i++) {
			if (i == 0) {
				printf(" - ");
			} else if (i > 0) {
				printf(", ");
			}
			
			id_query = bidib_get_train_id(seg_state_query.data.dcc_addresses[i]);
			if (id_query.known) {
				printf("%s", id_query.id);
			} else {
				printf("unknown");
			}
			bidib_free_id_query(id_query);
		}

		printf("\n");
	}
}

void print_train_state_query(t_bidib_train_state_query query) {
	if (query.known) {
		printf("speed step: %d, km/h speed: %d, direction: %d ack: 0x%x\n",
			   query.data.set_speed_step, query.data.detected_kmh_speed,
			   query.data.direction, query.data.ack);
	}
}

void print_train_position_query(t_bidib_train_position_query query) {
	printf("(");
	for (size_t i = 0; i < query.length; i++) {
		if (i > 0) {
			printf(", ");
		}
		printf("%s", query.segments[i]);
	}
	printf(")");
}

const char train_name[] = "cargo_db"; 

int main(void) {
	if(bidib_start_serial("/dev/cu.usbserial-A907DHAS", "../config", 0)) {
		printf("Unable to start serial communication.\n");
	} else {
		t_bidib_id_list_query ids_query = bidib_get_boards_connected();
		printf("Connected boards: %zu\n", ids_query.length);
		for (size_t i = 0; i < ids_query.length; i++) {
			printf(" - %s\n", ids_query.ids[i]);
		}
		bidib_free_id_list_query(ids_query);

		bidib_identify("master", 0x01);
		bidib_identify("lightcontrol", 0x01);
		bidib_identify("onecontrol", 0x01);
		bidib_flush();

		int protocol_version = bidib_get_protocol_version("master");
		int software_version = bidib_get_software_version("master");
		bidib_flush();
		printf("Protocol version: %d\n", protocol_version);
		printf("Software version: %d\n", software_version);
		usleep(100000);
		printf("Message queue:\n");
		print_queue(&bidib_read_message);

		t_bidib_unified_accessory_state_query accessory_query =
			bidib_get_point_state("point1");
		printf("point1 ");
		print_board_accessory_state_query(accessory_query);
		bidib_switch_point("point1", "reverse");
		bidib_flush();
		usleep(100000);
		accessory_query = bidib_get_point_state("point1");
		printf("point1 ");
		print_board_accessory_state_query(accessory_query);
		
		accessory_query = bidib_get_point_state("point8");
		printf("point8 ");
		print_board_accessory_state_query(accessory_query);
		bidib_switch_point("point8", "reverse");
		bidib_flush();
		usleep(100000);
		accessory_query = bidib_get_point_state("point8");
		printf("point8 ");
		print_board_accessory_state_query(accessory_query);
		usleep(500000);
		bidib_switch_point("point8", "normal");
		bidib_flush();
		usleep(100000);
		accessory_query = bidib_get_point_state("point8");
		printf("point8 ");
		print_board_accessory_state_query(accessory_query);
		
		print_points();

		accessory_query = bidib_get_signal_state("signal1");
		printf("signal1 ");
		print_board_accessory_state_query(accessory_query);
		accessory_query = bidib_get_signal_state("signal2-top");
		printf("signal2-top ");
		print_board_accessory_state_query(accessory_query);
		accessory_query = bidib_get_signal_state("signal2-bot");
		printf("signal2-bot ");
		print_board_accessory_state_query(accessory_query);
		usleep(1000000);
		bidib_set_signal("signal1", "green");
		bidib_flush();

		t_bidib_segment_state_query seq_query =
			bidib_get_segment_state("seg1");
		printf("seg1 ");
		print_segment_state_query(seq_query);
		bidib_free_segment_state_query(seq_query);
		ids_query = bidib_get_trains_on_track();
		t_bidib_train_position_query train_position_query;
		printf("Number of trains on the track: %ld\n", ids_query.length);
		for (size_t i = 0; i < ids_query.length; i++) {
			printf(" - %s ", ids_query.ids[i]);
			
			train_position_query = bidib_get_train_position(ids_query.ids[i]);
			print_train_position_query(train_position_query);
			printf("\n");
		}
		bidib_free_id_list_query(ids_query);
		bidib_free_train_position_query(train_position_query);
		
		printf("cargo ");
		fflush(stdout);
		bidib_set_calibrated_train_speed(train_name, 1, "master");
		bidib_flush();
		usleep(4000000);
		t_bidib_train_state_query train_state_query = bidib_get_train_state(train_name);
		print_train_state_query(train_state_query);
		bidib_free_train_state_query(train_state_query);

		printf("cargo ");
		fflush(stdout);
		bidib_set_calibrated_train_speed(train_name, -1, "master");
		bidib_flush();
		usleep(4000000);
		train_state_query = bidib_get_train_state(train_name);
		print_train_state_query(train_state_query);
		bidib_free_train_state_query(train_state_query);

		printf("cargo ");
		fflush(stdout);
		bidib_set_train_speed(train_name, 0, "master");
		bidib_set_train_peripheral(train_name, "head_light", 0x00, "master");
		bidib_flush();
		usleep(1000000);
		train_state_query = bidib_get_train_state(train_name);
		print_train_state_query(train_state_query);
		bidib_free_train_state_query(train_state_query);

		bidib_identify("master", 0x00);
		bidib_identify("lightcontrol", 0x00);
		bidib_identify("onecontrol", 0x00);
		bidib_flush();

		printf("Error queue:\n");
		print_queue(&bidib_read_error_message);

		bidib_stop();
	}
	
	return 0;
}

