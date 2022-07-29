libbidib-reverse-engineering.md


Functions that show certain patterns/get called with certain locks

- bidib_state_get_board_ref
	- Called with `bidib_state_boards_mutex` locked:   xxxxxxxxxxxxx
	- Called with `bidib_state_boards_mutex` unlocked: xxxxxx
		- from bidib_get_board_features -> apparently only called in parser test
		- from bidib_get_board_points (highlevel_getter)
		- from bidib_get_board_signals ""
		- from bidib_get_board_peripherals ""
		- from bidib_get_board_segments ""
		- from bidib_config_parse_single_board_setup (config_parser_track)
- bidib_state_get_board_ref_by_nodeaddr
	- Called with `bidib_state_boards_mutex` locked:   xxxxxxxxx
	- Called with `bidib_state_boards_mutex` unlocked: xxxx