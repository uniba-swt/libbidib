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

#include <syslog.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "../../include/highlevel/bidib_highlevel_util.h"
#include "../../include/highlevel/bidib_highlevel_setter.h"
#include "../../include/lowlevel/bidib_lowlevel_system.h"
#include "bidib_highlevel_intern.h"
#include "../transmission/bidib_transmission_intern.h"
#include "../transmission/bidib_transmission_serial_port_intern.h"
#include "../state/bidib_state_intern.h"


static pthread_t bidib_receiver_thread = 0;
static pthread_t bidib_autoflush_thread = 0;
static pthread_t bidib_heartbeat_thread = 0;

// Pthread locks that protect read/write access to the bidib_boards,
// bidib_track_state, bidib_trains data structures. 
// They do NOT protect the concurrent sending of low-level commands 
// to the BiDiB master node over a serial connection.
pthread_rwlock_t bidib_trains_rwlock;
pthread_rwlock_t bidib_boards_rwlock;

pthread_mutex_t trackstate_accessories_mutex;
pthread_mutex_t trackstate_peripherals_mutex;
pthread_mutex_t trackstate_segments_mutex;
pthread_mutex_t trackstate_reversers_mutex;
pthread_mutex_t trackstate_trains_mutex;
pthread_mutex_t trackstate_boosters_mutex;
pthread_mutex_t trackstate_track_outputs_mutex;


volatile bool bidib_running = false;
volatile bool bidib_discard_rx = true;
volatile bool bidib_lowlevel_debug_mode = false;

static void bidib_init_rwlocks(void) {
	pthread_rwlock_init(&bidib_trains_rwlock, NULL);
	pthread_rwlock_init(&bidib_boards_rwlock, NULL);
}

static void bidib_init_mutexes(void) {
	// New fine grained mutexes for trackstate
	pthread_mutex_init(&trackstate_accessories_mutex, NULL);
	pthread_mutex_init(&trackstate_peripherals_mutex, NULL);
	pthread_mutex_init(&trackstate_segments_mutex, NULL);
	pthread_mutex_init(&trackstate_reversers_mutex, NULL);
	pthread_mutex_init(&trackstate_trains_mutex, NULL);
	pthread_mutex_init(&trackstate_boosters_mutex, NULL);
	pthread_mutex_init(&trackstate_track_outputs_mutex, NULL);
	
	
	pthread_mutex_lock(&trackstate_accessories_mutex);
	pthread_mutex_lock(&trackstate_peripherals_mutex);
	pthread_mutex_lock(&trackstate_segments_mutex);
	pthread_mutex_lock(&trackstate_reversers_mutex);
	pthread_mutex_lock(&trackstate_trains_mutex);
	pthread_mutex_lock(&trackstate_boosters_mutex);
	pthread_mutex_lock(&trackstate_track_outputs_mutex);
	
	pthread_mutex_unlock(&trackstate_track_outputs_mutex);
	pthread_mutex_unlock(&trackstate_boosters_mutex);
	pthread_mutex_unlock(&trackstate_trains_mutex);
	pthread_mutex_unlock(&trackstate_reversers_mutex);
	pthread_mutex_unlock(&trackstate_segments_mutex);
	pthread_mutex_unlock(&trackstate_peripherals_mutex);
	pthread_mutex_unlock(&trackstate_accessories_mutex);
	
	// End of new fine grained mutexes for trackstate initialization
	
	pthread_mutex_init(&bidib_node_state_table_mutex, NULL);
	pthread_mutex_init(&bidib_send_buffer_mutex, NULL);
	pthread_mutex_init(&bidib_uplink_queue_mutex, NULL);
	pthread_mutex_init(&bidib_uplink_error_queue_mutex, NULL);
	pthread_mutex_init(&bidib_uplink_intern_queue_mutex, NULL);
	pthread_mutex_init(&bidib_action_id_mutex, NULL);
	
	pthread_mutex_lock(&bidib_node_state_table_mutex);
	pthread_mutex_lock(&bidib_send_buffer_mutex);
	pthread_mutex_lock(&bidib_uplink_queue_mutex);
	pthread_mutex_lock(&bidib_uplink_error_queue_mutex);
	pthread_mutex_lock(&bidib_uplink_intern_queue_mutex);
	pthread_mutex_lock(&bidib_action_id_mutex);

	pthread_mutex_unlock(&bidib_action_id_mutex);
	pthread_mutex_unlock(&bidib_uplink_intern_queue_mutex);
	pthread_mutex_unlock(&bidib_uplink_error_queue_mutex);
	pthread_mutex_unlock(&bidib_uplink_queue_mutex);
	pthread_mutex_unlock(&bidib_send_buffer_mutex);
	pthread_mutex_unlock(&bidib_node_state_table_mutex);
}

static void bidib_init_threads(unsigned int flush_interval) {
	pthread_create(&bidib_receiver_thread, NULL, bidib_auto_receive, NULL);
	pthread_create(&bidib_heartbeat_thread, NULL, bidib_heartbeat_log, NULL);
	if (flush_interval > 0) {
		unsigned int *arg = malloc(sizeof(unsigned int));
		*arg = flush_interval;
		pthread_create(&bidib_autoflush_thread, NULL, bidib_auto_flush, arg);
	}
}

int bidib_start_pointer(uint8_t (*read)(int *), void (*write_n)(uint8_t*, int32_t), 
                        const char *config_dir, unsigned int flush_interval) {
	if (read == NULL || write_n == NULL || (!bidib_lowlevel_debug_mode && config_dir == NULL)) {
		return 1;
	}
	int error = 0;
	if (!bidib_running) {
		bidib_running = true;
		if (bidib_lowlevel_debug_mode) {
			bidib_discard_rx = false;
		}
		openlog("swtbahn", 0, LOG_LOCAL0);
		syslog_libbidib(LOG_NOTICE, "libbidib started");

		bidib_node_state_table_init();
		bidib_init_rwlocks();
		bidib_init_mutexes();

		if (bidib_state_init(config_dir)) {
			error = 1;
		}

		bidib_set_read_src(read);
		bidib_set_write_n_dest(write_n);

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
		if (bidib_lowlevel_debug_mode) {
			bidib_discard_rx = false;
		}
		openlog("swtbahn", 0, LOG_LOCAL0);
		syslog_libbidib(LOG_NOTICE, "libbidib started");

		bidib_node_state_table_init();

		bidib_init_rwlocks();
		bidib_init_mutexes();
		if (bidib_state_init(config_dir) || bidib_serial_port_init(device)) {
			error = 1;
		} else {
			bidib_set_read_src(bidib_serial_port_read);
			bidib_set_write_n_dest(bidib_serial_port_write_n);

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
		syslog_libbidib(LOG_NOTICE, "libbidib running and now stopping");
		// close the track
		bidib_set_track_output_state_all(BIDIB_CS_SOFTSTOP);
		bidib_flush();
		usleep(300000); // 0.3s
		bidib_state_reset_train_params();
		bidib_flush();
		usleep(300000); // 0.3s
		bidib_set_track_output_state_all(BIDIB_CS_OFF);
		bidib_flush();
		bidib_running = false;
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: waiting for threads to join");
		if (bidib_receiver_thread != 0) {
			pthread_join(bidib_receiver_thread, NULL);
		}
		if (bidib_autoflush_thread != 0) {
			pthread_join(bidib_autoflush_thread, NULL);
		}
		if (bidib_heartbeat_thread != 0) {
			pthread_join(bidib_heartbeat_thread, NULL);
		}
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: threads have joined");
		bidib_serial_port_close();
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: Serial port closed");
		bidib_node_state_table_free();
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: State table freed");
		bidib_uplink_queue_free();
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: Uplink queue freed");
		bidib_uplink_error_queue_free();
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: Uplink error queue freed");
		bidib_uplink_intern_queue_free();
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: Uplink intern queue freed");
		bidib_state_free();
		syslog_libbidib(LOG_NOTICE, "libbidib stopping: State freed");
		syslog_libbidib(LOG_NOTICE, "libbidib stopped");
		closelog();
		usleep(500000); // 0.5s, wait for thread clean up
	}
}

void syslog_libbidib(int priority, const char *format, ...) {
	char string[1024];
	va_list arg;
	va_start(arg, format);
	vsnprintf(string, 1024, format, arg);
	syslog(priority, "libbidib: %s", string);
}

void *bidib_heartbeat_log(void *par __attribute__((unused))) {
	while (bidib_running) {
		struct timespec tv;
		clock_gettime(CLOCK_MONOTONIC, &tv);
		syslog_libbidib(LOG_INFO, "Heartbeat, time %ld.%.ld", tv.tv_sec, tv.tv_nsec);
		for (int i = 0; i < 20; i++) {
			// 0.1s
			usleep(100000);
			if (!bidib_running) {
				break;
			}
		}
	}
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC, &tv);
	syslog_libbidib(LOG_INFO, 
	                "Heartbeat exits as libbidib is stopping, time %ld.%.ld", 
	                tv.tv_sec, tv.tv_nsec);
	return NULL;
}