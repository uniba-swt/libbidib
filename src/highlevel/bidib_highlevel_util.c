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
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../transmission/bidib_transmission_serial_port_intern.h"
#include "../state/bidib_state_intern.h"
#include "../../include/bidib.h"
#include "bidib_highlevel_intern.h"


static pthread_t bidib_receiver_thread = 0;
static pthread_t bidib_autoflush_thread = 0;

volatile bool bidib_running = false;
volatile bool bidib_lowlevel_debug_mode = false;


static void bidib_init_mutexes(void) {
	pthread_mutex_init(&bidib_node_state_table_mutex, NULL);
	pthread_mutex_init(&bidib_uplink_queue_mutex, NULL);
	pthread_mutex_init(&bidib_uplink_error_queue_mutex, NULL);
	pthread_mutex_init(&bidib_uplink_intern_queue_mutex, NULL);
	pthread_mutex_init(&bidib_send_buffer_mutex, NULL);
	pthread_mutex_init(&bidib_state_track_mutex, NULL);
	pthread_mutex_init(&bidib_state_boards_mutex, NULL);
	pthread_mutex_init(&bidib_state_trains_mutex, NULL);
	pthread_mutex_init(&bidib_action_id_mutex, NULL);
}

static void bidib_init_threads(unsigned int flush_interval) {
	pthread_create(&bidib_receiver_thread, NULL, bidib_auto_receive, NULL);
	if (flush_interval > 0) {
		unsigned int *arg = malloc(sizeof(unsigned int));
		*arg = flush_interval;
		pthread_create(&bidib_autoflush_thread, NULL, bidib_auto_flush, arg);
	}
}

int bidib_start_pointer(unsigned char (*read)(int *), void (*write)(unsigned char),
                        const char *config_dir, unsigned int flush_interval) {
	if (read == NULL || write == NULL || (!bidib_lowlevel_debug_mode && config_dir == NULL)) {
		return 1;
	}
	int error = 0;
	if (!bidib_running) {
		bidib_running = true;
		openlog("bidib", 0, LOG_LOCAL0);
		syslog(LOG_NOTICE, "%s", "libbidib started");

		bidib_node_state_table_init();

		bidib_init_mutexes();

		if (bidib_state_init(config_dir)) {
			error = 1;
		}

		bidib_set_read_src(read);
		bidib_set_write_dest(write);

		bidib_init_threads(flush_interval);

		if (!bidib_lowlevel_debug_mode) {
			if (!bidib_communication_works()) {
				error = 1;
			} else {
				bidib_send_sys_reset(0);
			}
		}
	}
	if (error) {
		bidib_stop();
	}
	return error;
}

int bidib_start_serial(const char *device, const char *config_dir, unsigned int flush_interval) {
	if (device == NULL || config_dir == NULL) {
		return 1;
	}
	int error = 0;
	if (!bidib_running) {
		bidib_running = true;
		openlog("bidib", 0, LOG_LOCAL0);
		syslog(LOG_NOTICE, "%s", "libbidib started");

		bidib_node_state_table_init();

		bidib_init_mutexes();
		if (bidib_state_init(config_dir) || bidib_serial_port_init(device)) {
			error = 1;
		} else {
			bidib_set_read_src(bidib_serial_port_read);
			bidib_set_write_dest(bidib_serial_port_write);

			bidib_init_threads(flush_interval);

			if (!bidib_lowlevel_debug_mode) {
				if (bidib_detect_baudrate()) {
					error = 1;
				} else {
					bidib_send_sys_reset(0);
				}
			}
		}
	}
	if (error) {
		bidib_stop();
	}
	return error;
}

void bidib_stop(void) {
	if (bidib_running) {
		// close the track
		bidib_set_track_output_state_all(BIDIB_CS_SOFTSTOP);
		bidib_flush();
		usleep(300000);
		bidib_state_reset_train_params();
		bidib_flush();
		usleep(300000);
		bidib_set_track_output_state_all(BIDIB_CS_OFF);
		bidib_flush();

		bidib_running = false;
		if (bidib_receiver_thread != 0) {
			pthread_join(bidib_receiver_thread, NULL);
			if (bidib_autoflush_thread != 0) {
				pthread_join(bidib_autoflush_thread, NULL);
			}
		}
		bidib_serial_port_close();
		bidib_node_state_table_free();
		bidib_uplink_queue_free();
		bidib_uplink_error_queue_free();
		bidib_uplink_intern_queue_free();
		bidib_state_free();
		syslog(LOG_NOTICE, "%s", "libbidib stopped");
		closelog();
		usleep(500000); // wait for thread clean up
	}
}
