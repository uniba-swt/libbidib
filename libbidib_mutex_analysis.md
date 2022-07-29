libbidib_mutex_analysis.md

# bidib_action_id_mutex
- Used in: 
	- src/highlevel/bidib_highlevel_action_ids.c (definition)
	- src/highlevel/bidib_highlevel_util.c (initialisation)


# bidib_node_state_table_mutex
- Used in:
	- bidib_highlevel_util.c (initialisation)
	- bidib_transmission_node_states.c (definition)

*bidib_transmission_node_states*
- Functions that lock and unlock once
	- bidib_node_try_send
	- bidib_node_state_update
	- bidib_node_update_stall
	- bidib_node_state_get_and_incr_receive_seqnum
	- bidib_node_state_get_and_incr_send_seqnum
	- bidib_node_state_set_receive_seqnum
	- bidib_node_state_table_reset


# bidib_uplink_queue_mutex
- Used in:
	- bidib_highlevel_util.c (initialisation)
	- bidib_transmission_receive.c (definition)

- Functions that lock and unlock once (bidib_transmission_receive)
	- bidib_uplink_queue_reset
	- bidib_uplink_queue_add
	- bidib_read_message


# bidib_uplink_error_queue_mutex
- Used in 
	- bidib_highlevel_util.c (initialisation)
	- bidib_transmission_receive.c (definition)

- Functions that lock and unlock once (bidib_transmission_receive)
	- bidib_uplink_error_queue_reset
	- bidib_uplink_error_queue_add
	- bidib_read_error_message


# bidib_uplink_intern_queue_mutex
- Used in 
	- bidib_highlevel_util.c (initialisation)
	- bidib_transmission_receive.c (definition)

- Functions that lock and unlock once (bidib_transmission_receive)
	- bidib_uplink_intern_queue_reset
	- bidib_uplink_intern_queue_add
	- bidib_read_intern_message


# bidib_send_buffer_mutex
- Used in
	- bidib_highlevel_util.c (initialisation) 
	- bidib_transmission_send.c (definition)

- Functions that lock and unlock once (bidib_transmission_send)
	- bidib_state_packet_capacity
	- bidib_flush
	- bidib_auto_flush
	- bidib_add_to_buffer

# bidib_state_track_mutex
- Used in
	- bidib_highlevel_util.c (initialisation) 
	- bidib_state.c (definition)
	- bidib_state_setter.c
	- bidib_highlevel_setter.c
	- bidib_highlevel_getter.c

- Functions that lock and unlock once (bidib_state)
	- bidib_state_point_exists
	- bidib_state_signal_exists
	- bidib_state_peripheral_exists
	- bidib_state_segment_exists
	- bidib_state_add_booster
	- bidib_state_add_track_output
	- bidib_state_add_train_state
	- bidib_state_reset

- Functions that lock and unlock once (bidib_state_setter)
	- bidib_state_boost_state
	- bidib_state_accessory_state
	- bidib_state_cs_state
	- bidib_state_cs_drive_ack (suspicious unlock order)
	- bidib_state_cs_accessory_ack
	- bidib_state_cs_drive
	- bidib_state_cs_accessory_manual
	- bidib_state_cs_accessory
	- bidib_state_lc_stat
	- bidib_state_lc_wait
	- bidib_state_bm_occ
	- bidib_state_bm_multiple
	- bidib_state_bm_confidence
	- bidib_state_bm_address
	- bidib_state_bm_current
	- bidib_state_bm_speed (suspicious unlock order)
	- bidib_state_bm_dyn_state (suspicious unlock order)

- Functions that lock and unlock once (bidib_highlevel_getter)
	- bidib_get_state
	- bidib_get_point_state
	- bidib_get_signal_state
	- bidib_get_peripheral_state
	- bidib_get_segment_state
	- bidib_get_booster_state
	- bidib_get_track_output_state
	- bidib_get_trains_on_track
	- bidib_get_train_peripheral_state
	- bidib_get_train_on_track
	- bidib_get_train_position
	- bidib_get_train_speed_step
	- bidib_get_train_speed_kmh

- Functions that lock and unlock once (bidib_highlevel_setter)
- Functions that lock and unlock *multiple times* (bidib_highlevel_setter)
	- bidib_switch_point (lock placement?) (overall method questionable)
		- Had an incorrect double unlock for another mutex
	- bidib_set_signal (lock placement?) (overall method questionable)
		- Had an incorrect double unlock for another mutex