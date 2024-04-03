/*
 *
 * Copyright (C) 2022 University of Bamberg, Software Technologies Research Group
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
 * - Bernhard Luedtke <https://github.com/BLuedtke>
 *
 */

#ifndef KINDERUNISUITE_H
#define KINDERUNISUITE_H

#include "../test_common.h" // IWYU pragma: keep

bool ku_scenario1_initial(const char *train1);
bool ku_scenario1_aktion(const char *train1);
bool ku_scenario1_reset(const char *train1);

bool ku_scenario2_initial(const char *train1, const char *train2);
bool ku_scenario2_aktion(const char *train1, const char *train2);
bool ku_scenario2_reset(const char *train1, const char *train2);

bool ku_scenario3_initial(const char *train1, const char *train2);
bool ku_scenario3_aktion(const char *train1, const char *train2);
bool ku_scenario3_reset(const char *train1, const char *train2);

bool ku_scenario4_initial(const char *train1, const char *train2);
bool ku_scenario4_aktion(const char *train1, const char *train2);
bool ku_scenario4_reset(const char *train1, const char *train2);

char read_char();

bool ku_scenario1_interactive(const char *train1);

#endif