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

#ifndef BIDIB_H
#define BIDIB_H

// Highlevel functions
#include "highlevel/bidib_highlevel_admin.h"
#include "highlevel/bidib_highlevel_getter.h"
#include "highlevel/bidib_highlevel_setter.h"
#include "highlevel/bidib_highlevel_util.h"

// Lowlevel send functions
#include "lowlevel/bidib_lowlevel_system.h"
#include "lowlevel/bidib_lowlevel_feature.h"
#include "lowlevel/bidib_lowlevel_userconfig.h"
#include "lowlevel/bidib_lowlevel_firmware.h"
#include "lowlevel/bidib_lowlevel_track.h"
#include "lowlevel/bidib_lowlevel_accessory.h"
#include "lowlevel/bidib_lowlevel_portconfig.h"
#include "lowlevel/bidib_lowlevel_occupancy.h"
#include "lowlevel/bidib_lowlevel_booster.h"

// Message and structure definitions
#include "definitions/bidib_messages.h"
#include "definitions/bidib_definitions_custom.h"

#endif
