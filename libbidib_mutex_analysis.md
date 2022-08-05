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


--

Orders:

1. state_boards_mutex
2. state_trains_mutex
3. state_track_mutex
X. action_id_mutex

OR IS IT REALLY?

- trains_mutex before track_mutex according to bidib_send_cs_drive_intern in bidib_lowlevel_track.c

--
- All methods that lock a mutex in the following files do not call a method that locks another mutex, except for action_id_mutex:
	- bidib_highlevel_actions_ids
	- bidib_highlevel_admin
	- bidib_highlevel_getter
	- 


bidib_highlevel_setter - bidib_set_peripheral:
1. Locks bidib_state_boards_mutex
2. Calls bidib_send_lc_output
	1. Calls bidib_buffer_message_with_data
		1. Calls bidib_node_state_get_and_incr_send_seqnum
			1. Locks bidib_node_state_table_mutex
			2. Unlocks bidib_node_state_table_mutex
			3. return
		2. return
	2. return
3. Unlocks bidib_state_boards_mutex
4. return

bidib_highlevel_setter - bidib_set_train_speed: 				! ORDER TRAINS -> TRACK -> BOARDS
1. Locks bidib_state_boards_mutex
2. Unlocks bidib_state_boards_mutex
3. Calls bidib_send_cs_drive
	1. Calls bidib_send_cs_drive_intern
		1. Calls bidib_buffer_message_with_data
			1. Calls bidib_node_state_get_and_incr_send_seqnum
				1. Locks bidib_node_state_table_mutex
				2. Unlocks bidib_node_state_table_mutex
				3. return
			2. return
		2. Locks bidib_state_trains_mutex
		3. Calls bidib_state_cs_drive
			1. Locks bidib_state_track_mutex
			2. (MAY) Call bidib_state_dcc_addr_in_use
				3. Locks bidib_state_boards_mutex
				4. Unlocks bidib_state_boards_mutex
			3. Unlocks bidib_state_track_mutex
			4. return
		4. Unlocks bidib_state_trains_mutex
		5. return
	2. return
4. return

bidib_highlevel_setter - bidib_emergency_stop_train:
- Same as bidib_set_train_speed


bidib_highlevel_setter - bidib_set_train_peripheral   			! ORDER BOARDS -> TRAINS -> TRACK -> BOARDS **ALARM**
1. Locks bidib_state_boards_mutex
2. Locks bidib_state_trains_mutex
3. Unlocks bidib_state_boards_mutex
4. Calls bidib_send_cs_drive_intern (lock param false)
	1. Calls bidib_buffer_message_with_data
		1. Calls bidib_node_state_get_and_incr_send_seqnum
			1. Locks bidib_node_state_table_mutex
			2. Unlocks bidib_node_state_table_mutex
			3. return
		2. return
	2. Calls bidib_state_cs_drive
		1. Locks bidib_state_track_mutex
		2. (MAY) Call bidib_state_dcc_addr_in_use
			3. Locks bidib_state_boards_mutex 
			4. Unlocks bidib_state_boards_mutex
		3. Unlocks bidib_state_track_mutex
		4. return
	3. return
5. Unlocks bidib_state_trains_mutex

bidib_highlevel_setter - bidib_set_booster_power_state
1. Locks bidib_state_boards_mutex
2. Unlocks bidib_state_boards_mutex
3. Calls bidib_send_boost_off
	1. Calls bidib_buffer_message_with_data
		1. Calls bidib_node_state_get_and_incr_send_seqnum
			1. Locks bidib_node_state_table_mutex
			2. Unlocks bidib_node_state_table_mutex
			3. return
		2. return
	2. return
4. return

bidib_highlevel_setter - bidib_set_track_output_state
1. Locks bidib_state_boards_mutex
2. Unlocks bidib_state_boards_mutex
3. Calls bidib_send_cs_set_state
	1. Calls bidib_buffer_message_with_data
		1. Calls bidib_node_state_get_and_incr_send_seqnum
			1. Locks bidib_node_state_table_mutex
			2. Unlocks bidib_node_state_table_mutex
			3. return
		2. return
	2. return
4. return

bidib_highlevel_setter - bidib_set_track_output_state_all
1. Locks bidib_state_boards_mutex
2. Calls bidib_send_cs_set_state * ? TIMES
	1. Calls bidib_buffer_message_with_data
		1. Calls bidib_node_state_get_and_incr_send_seqnum
			1. Locks bidib_node_state_table_mutex
			2. Unlocks bidib_node_state_table_mutex
			3. return
		2. return
	2. return
3. Unlocks bidib_state_boards_mutex
-> Somewhat inconsistent compared to bidib_set_track_output_state, though not a big deal

bidib_lowlevel_track: bidib_send_cs_drive_intern -> bidib_state_trains_mutex before bidib_state_track_mutex

bidib_state

bidib_state - bidib_state_init_allocation_table
1. Locks bidib_state_boards_mutex
2. Calls bidib_state_query_nodetab * ? TIMES
	1. typical bidib_node_state_table_mutex flow (lock+unlock)
	2. Calls bidib_flush
		1. Locks bidib_send_buffer_mutex
		2. Unlocks bidib_send_buffer_mutex
	3. Calls bidib_read_intern_message
		1. Locks bidib_uplink_intern_queue_mutex
		2. Unlocks bidib_uplink_intern_queue_mutex
	4. typical bidib_node_state_table_mutex flow (lock+unlock)
	5. Calls bidib_flush
		1. Locks bidib_send_buffer_mutex
		2. Unlocks bidib_send_buffer_mutex
	6. Calls bidib_read_intern_message
		1. Locks bidib_uplink_intern_queue_mutex
		2. Unlocks bidib_uplink_intern_queue_mutex
3. Unlocks bidib_state_boards_mutex

bidib_state - bidib_state_set_board_features
- Typical lock bidib_state_boards_mutex, lock bidib_node_state_table_mutex, unlock bidib_node_state_table_mutex, unlock bidib_state_boards_mutex

bidib_state_setter - bidib_state_boost_state 				! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Calls bidib_state_get_booster_state_ref_by_nodeaddr
	1. Locks bidib_state_boards_mutex 
	2. Unlocks bidib_state_boards_mutex
3. Unlocks bidib_state_track_mutex

bidib_state_setter - bidib_state_accessory_state 			! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Locks bidib_state_boards_mutex
3. Unlocks bidib_state_boards_mutex
4. Unlocks bidib_state_track_mutex

bidib_state_setter - bidib_state_cs_state
1. Locks bidib_state_track_mutex
2. Calls bidib_state_get_track_output_state_ref_by_nodeaddr
	1. Locks bidib_state_boards_mutex
	2. Unlocks bidib_state_boards_mutex
3. Unlocks bidib_state_track_mutex


bidib_state_setter - bidib_state_cs_drive_ack 				! ORDER TRAINS -> TRACK -> BOARDS
1. Locks bidib_state_trains_mutex
2. Locks bidib_state_track_mutex
3. Calls bidib_state_dcc_addr_in_use
	1. Locks bidib_state_boards_mutex
	2. Unlocks bidib_state_boards_mutex
4. Unlocks bidib_state_track_mutex
5. Unlocks bidib_state_trains_mutex

bidib_state_setter - bidib_state_cs_accessory_ack 			! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Locks bidib_state_boards_mutex
3. Unlocks bidib_state_boards_mutex
4. Unlocks bidib_state_track_mutex

bidib_state_setter - bidib_state_cs_drive 					! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Calls bidib_state_dcc_addr_in_use
	1. Locks bidib_state_boards_mutex
	2. Unlocks bidib_state_boards_mutex
3. Unlocks bidib_state_track_mutex

bidib_state_setter - bidib_state_cs_accessory_manual 		! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Locks bidib_state_boards_mutex
3. Unlocks bidib_state_boards_mutex
4. Unlocks bidib_state_track_mutex

//Outdated
bidib_state_setter - bidib_state_cs_accessory 				! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Locks bidib_state_boards_mutex
3. Unlocks bidib_state_boards_mutex
4. Unlocks bidib_state_track_mutex

bidib_state_setter - bidib_state_lc_stat 					! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Calls bidib_state_get_peripheral_mapping_ref_by_port
	1. Locks bidib_state_boards_mutex
	2. Unlocks bidib_state_boards_mutex
3. Unlocks bidib_state_track_mutex
Same for bidib_state_lc_wait
Similar for bidib_state_bm_occ (different method, same locks)
Similar for bidib_state_bm_multiple (different method, multiple state board locks)
Similar for bidib_state_bm_address (different method, same locks)


bidib_state_setter - bidib_state_bm_confidence 				! ORDER TRACK -> BOARDS
1. Locks bidib_state_track_mutex
2. Locks bidib_state_boards_mutex
3. Unlocks bidib_state_boards_mutex
4. Unlocks bidib_state_track_mutex

bidib_state_setter - bidib_state_bm_speed					! ORDER TRAINS -> TRACK; **BUT INCORRECT UNLOCK ORDER**
1. Locks bidib_state_trains_mutex
2. Locks bidib_state_track_mutex
3. Unlocks bidib_state_trains_mutex
4. Unlocks bidib_state_track_mutex 							!!!!!
- Same for bidib_state_bm_dyn_state

WAS HERE: 5th occurrence in bidib_transmission_receive
-> nothing of much note thereafter

--

bidib_state_track_mutex locked:
- bidib_state_get_board_accessory_state_ref
- bidib_state_get_dcc_accessory_state_ref

bidib_state_track_mutex locked:
- bidib_state_get_train_state_ref