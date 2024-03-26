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
 * - Christof Lehanka <https://github.com/clehanka>
 * - Bernhard Luedtke <https://github.com/BLuedtke>
 * - Eugene Yip <https://github.com/eyip002>
 *
 */

#ifndef TESTSUITE_H
#define TESTSUITE_H

#include "../test_common.h"

// Setup
t_testsuite_test_result *testsuite_initTestSuite();

// Test cases
void testsuite_case_signal();
void testsuite_case_pointParallel(t_testsuite_test_result *result);
void testsuite_case_pointSerial(t_testsuite_test_result *result);
void testsuite_case_swtbahnFullTrackCoverage(const char *train);
void testsuite_case_swtbahnFullMultipleTrains(const char *train1, const char *train2);
void testsuite_case_swtbahnFullShortRoute(const char *train);

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

#endif
