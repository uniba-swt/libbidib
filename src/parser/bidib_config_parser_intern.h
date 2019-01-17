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

#ifndef BIDIB_CONFIG_PARSER_INTERN_H
#define BIDIB_CONFIG_PARSER_INTERN_H

#include <yaml.h>
#include <stdbool.h>

#include "../state/bidib_state_intern.h"


/**
 * Reads the config files and create the track state structure.
 *
 * @param config_dir the directory containing the config files.
 * @return true if an error occurred and false if successful.
 */
bool bidib_config_parse(const char *config_dir);

/**
 * Initializes a yaml parser for a config file.
 *
 * @param config_dir the directory containing the config files.
 * @param config_file the name of the config file.
 * @param fh destination for the file handler.
 * @param parser destination for the parser.
 * @return true if successful, otherwise false.
 */
bool bidib_config_init_parser(const char *config_dir, const char *config_file,
                              FILE **fh, yaml_parser_t *parser);

/**
 * Converts a string to a byte.
 *
 * @param string the string.
 * @param byte the destination for the byte.
 * @return true if successful, otherwise false.
 */
bool bidib_string_to_byte(char *string, uint8_t *byte);

/**
 * Converts a string to a unique id.
 *
 * @param string the string.
 * @param uid the destination for the unique id.
 * @return true if successful, otherwise false.
 */
bool bidib_string_to_uid(char *string, t_bidib_unique_id_mod *uid);

/**
 * Converts a string to a dcc address.
 *
 * @param string the string.
 * @param dcc_address the destination for the dcc_address.
 * @return true if successful, otherwise false.
 */
bool bidib_string_to_dccaddr(char *string, t_bidib_dcc_address *dcc_address);

/**
 * Converts a string to a port.
 *
 * @param string the string.
 * @param port the destination of the port.
 * @return true if successful, otherwise false.
 */
bool bidib_string_to_port(char *string, t_bidib_peripheral_port *port);

/**
 * Parses a scalar and a section afterwards.
 *
 * @param parser the parser.
 * @param scalar the name of the scalar.
 * @param section_elem_action handler for an item in the section.
 * @return true if successful, otherwise false.
 */
bool bidib_config_parse_scalar_then_section(yaml_parser_t *parser, char *scalar,
                                            bool (*section_elem_action)(yaml_parser_t *));

/**
 * Parses the board config file.
 *
 * @param config_dir the directory containing the config files.
 * @return true if successful, otherwise false.
 */
int bidib_config_parse_board_config(const char *config_dir);

/**
 * Parses the track config file.
 *
 * @param config_dir the directory containing the config files.
 * @return true if successful, otherwise false.
 */
int bidib_config_parse_track_config(const char *config_dir);

/**
 * Parses the train config file.
 *
 * @param config_dir the directory containing the config files.
 * @return true if successful, otherwise false.
 */
int bidib_config_parse_train_config(const char *config_dir);


#endif
