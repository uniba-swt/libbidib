Writer-Reader_Analysis.md


# transmission

## bidib_transmission_util
- bidib_communication_works
	* W bidib_seq_num_enabled
	* W bidib_discard_rx

## bidib_transmission_serial_port
- bidib_serial_port_set_options
	* W fd
- bidib_detect_baudrate
	* W fd
- bidib_serial_port_init
	* W fd
- bidib_serial_port_read
	* R fd
- bidib_serial_port_write
	* W fd
- bidib_serial_port_close
	* W fd

## bidib_transmission_send
- bidib_set_write_dest
	* W write_byte
- bidib_send_byte
	* W write_byte
- bidib_send_delimiter
	* W write_byte
- bidib_flush_impl
	* W write_byte
	* W buffer_index
	* R buffer
- bidib_buffer_message_without_data
	* R bidib_seq_num_enabled
- bidib_buffer_message_with_data
	* R bidib_seq_num_enabled

## bidib_transmission_receive
- bidib_set_read_src
	* W uplink_queue
	* W uplink_error_queue
	* W uplink_intern_queue
	* W read_byte
- bidib_set_lowlevel_debug_mode
	* W bidib_lowlevel_debug_mode
- bidib_uplink_queue_free
	* R bidib_running
	* W uplink_queue
- bidib_uplink_error_queue_free
	* R bidib_running
	* W uplink_error_queue
- bidib_uplink_intern_queue_free
	* R bidib_running
	* W uplink_intern_queue
- bidib_log_sys_error
	* R bidib_boards
- bidib_log_boost_stat_error
	* R bidib_boards
- bidib_log_boost_stat_okay
	* R bidib_boards
- bidib_handle_received_message
	* R bidib_lowlevel_debug_mode
- bidib_receive_packet
	* R bidib_running
	* R bidib_discard_rx
	* W read_byte
- bidib_receive_first_pkt_magic
	* R bidib_running
	* R bidib_discard_rx
	* W read_byte
- bidib_auto_receive
	* R bidib_running
	* R bidib_discard_rx
	* W read_byte (call to bidib_receive_packet)

## bidib_transmission_node_states
- bidib_node_state_table_init
	* W node_state_table
- bidib_node_query
	* W node_state_table
- bidib_node_stall_ready
	* W node_state_table
- bidib_node_try_queued_messages
	* R bidib_response_info
	* W node_state_table
- bidib_node_state_table_free
	* W node_state_table


# state

## bidib_state
- bidib_state_init
	* W bidib_initial_values
	* W bidib_track_state
	* W bidib_boards
	* W bidib_trains
	* W _configs_ (black magic)
- bidib_state_query_nodetab
	* W bidib_boards
- bidib_state_set_initial_values
	* W bidib_initial_values
	* R bidib_track_state
- bidib_state_dcc_addr_in_use
	* R bidib_trains
- bidib_state_add_train
	* W bidib_trains
- bidib_state_add_initial_point_value, bidib_state_add_initial_signal_value, bidib_state_add_initial_peripheral_value, bidib_state_add_initial_train_value
	* W bidib_initial_values
- bidib_state_update_train_available
	* R bidib_trains
	* W bidib_track_state
- bidib_state_query_occupancy
	* R bidib_boards
- bidib_state_add_dcc_point_state, bidib_state_add_dcc_signal_state
	* R bidib_trains

## bidib_state_setter
- bidib_state_cs_drive
	* W bidib_trains
- bidib_state_log_train_detect
	* R bidib_trains
	* R bidib_track_state
- bidib_state_bm_address_log_changes
	* R bidib_trains
	* R bidib_track_state
- bidib_state_bm_address
	* --
	* --
- bidib_state_cs_accessory
	* R bidib_boards
	* W bidib_track_state
- bidib_state_cs_accessory_manual
	* R bidib_boards
	* W bidib_track_state
- bidib_state_cs_accessory_ack
	* R bidib_boards
	* W bidib_track_state


## bidib_state_getter
- bidib_state_get_board_ref
	* R bidib_boards
- bidib_state_get_board_ref_by_nodeaddr
	* R bidib_boards
- bidib_state_get_board_ref_by_uniqueid
	* R bidib_boards
- bidib_state_get_booster_state_ref
	* R bidib_track_state
- bidib_state_get_track_output_state_ref
	* R bidib_track_state
- bidib_state_get_board_accessory_mapping_ref
	* R bidib_boards
- bidib_state_get_board_accessory_mapping_ref_by_number
	* R bidib_boards
- bidib_state_get_board_accessory_state_ref
	* R bidib_track_state
- bidib_state_get_dcc_accessory_mapping_ref
	* R bidib_boards
- bidib_state_get_dcc_accessory_mapping_ref_by_dccaddr
	* R bidib_boards
- bidib_state_get_dcc_accessory_state_ref
	* R bidib_track_state
- bidib_state_get_peripheral_mapping_ref
	* R bidib_boards
- bidib_state_get_peripheral_mapping_ref_by_port
	* R bidib_boards
- bidib_state_get_peripheral_state_ref
	* R bidib_track_state
- bidib_state_get_segment_state_ref
	* R bidib_track_state
- bidib_state_get_train_ref
	* R bidib_trains
- bidib_state_get_train_state_ref
	* R bidib_track_state
- bidib_state_get_train_state_ref_by_dccaddr
	* R bidib_trains
	* R bidib_track_state
- bidib_state_get_train_peripheral_state_by_bit
	* R bidib_trains
- bidib_state_get_booster_state_ref_by_nodeaddr
	* R bidib_track_state
- bidib_state_get_track_output_state_ref_by_nodeaddr
	* R bidib_track_state
- bidib_state_get_segment_state_ref_by_nodeaddr
	* R bidib_track_state


## bidib_state_free
- bidib_state_free
	* R bidib_running
	* W bidib_initial_values
	* W bidib_track_state
	* W bidib_boards
	* W bidib_trains

# parser
skipped

# lowlevel

## bidib_lowlevel_track
- bidib_send_cs_drive_intern
	* W bidib_trains
- bidib_send_cs_accessory
	* R bidib_boards
	* W bidib_track_state

## bidib_lowlevel_system
- bidib_send_sys_reset
	* R bidib_initial_values
	* W bidib_boards

# highlevel

## bidib_highlevel_util
- bidib_start_pointer
	* R bidib_initial_values
	* W node_state_table
	* W bidib_initial_values
	* W bidib_track_state
	* W bidib_boards
	* W bidib_trains
	* W _configs_ (black magic)
	* W uplink_queue
	* W uplink_error_queue
	* W uplink_intern_queue
	* W read_byte
	* W write_byte
	* W bidib_receiver_thread
	* W bidib_autoflush_thread
	* W bidib_seq_num_enabled
	* W bidib_discard_rx
- bidib_start_serial
	* R bidib_lowlevel_debug_mode
	* W bidib_running
	* W bidib_discard_rx
	* W node_state_table
	* W _mutexes-init_
	* W bidib_initial_values
	* W bidib_track_state
	* W bidib_boards
	* W bidib_trains
	* W _configs_ (black magic)
	* W fd
	* W uplink_queue
	* W uplink_error_queue
	* W uplink_intern_queue
	* W read_byte
	* W write_byte
	* W bidib_receiver_thread
	* W bidib_autoflush_thread
- bidib_stop
	* W bidib_running
	* W bidib_receiver_thread
	* W bidib_autoflush_thread
	* W fd
	* W uplink_queue, uplink_error_queue, uplink_intern_queue
	* W bidib_initial_values
	* W bidib_track_state
	* W bidib_boards
	* W bidib_trains

## bidib_highlevel_setter
- bidib_set_train_speed_internal
	* W bidib_trains
- bidib_get_current_train_peripheral_bits
	* R bidib_track_state
	* R bidib_trains

## bidib_highlevel_getter
- bidib_get_state_peripherals
	* R bidib_track_state
- bidib_get_state_segments
	* R bidib_track_state
- bidib_get_state_trains
	* R bidib_track_state
- bidib_get_state_boosters
	* R bidib_track_state
- bidib_get_state_track_outputs
	* R bidib_track_state
- bidib_get_train_position_intern
	* R bidib_track_state
	* R bidib_trains
- 