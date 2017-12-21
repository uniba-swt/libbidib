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

#include "bidib_transmission_intern.h"
#include "../../include/definitions/bidib_messages.h"


// structure: {length, response size, response1, response2, ..}
const int bidib_response_info[0x80][5] = {
		//-- system messages
		{0},                                                    // 0x00
		{2, 6,  MSG_SYS_MAGIC},                                 // 0x01
		{2, 6,  MSG_SYS_P_VERSION},                             // 0x02
		{1, 0},                                                 // 0x03
		{1, 0},                                                 // 0x04
		{2, 11, MSG_SYS_UNIQUE_ID},                             // 0x05
		{2, 7,  MSG_SYS_SW_VERSION},                            // 0x06
		{2, 5,  MSG_SYS_PONG},                                  // 0x07
		{2, 5,  MSG_SYS_IDENTIFY_STATE},                        // 0x08
		{1, 0},                                                 // 0x09
		{2, 5,  MSG_PKT_CAPACITY},                              // 0x0A
		{2, 5,  MSG_NODETAB_COUNT},                             // 0x0B
		{4, 13, MSG_NODETAB, MSG_NODE_NA, MSG_NODETAB_COUNT},   // 0x0C
		{1, 0},                                                 // 0x0D
		{2, 10, MSG_SYS_ERROR},                                 // 0x0E
		{2, 6,  MSG_FW_UPDATE_STAT},                            // 0x0F

		//-- feature and user config messages
		{2, 5,  MSG_FEATURE_COUNT},                             // 0x10
		{3, 6,  MSG_FEATURE, MSG_FEATURE_NA},                   // 0x11
		{2, 6,  MSG_FEATURE},                                   // 0x12
		{2, 6,  MSG_FEATURE},                                   // 0x13
		{2, 5,  MSG_VENDOR_ACK},                                // 0x14
		{2, 5,  MSG_VENDOR_ACK},                                // 0x15
		{2, 32, MSG_VENDOR},                                    // 0x16
		{2, 32, MSG_VENDOR},                                    // 0x17
		{1, 0},                                                 // 0x18
		{2, 30, MSG_STRING},                                    // 0x19
		{2, 30, MSG_STRING},                                    // 0x1A
		{0},                                                    // 0x1B
		{0},                                                    // 0x1C
		{0},                                                    // 0x1D
		{0},                                                    // 0x1E
		{0},                                                    // 0x1F

		//-- occupancy messages
		{4, 21, MSG_BM_MULTIPLE, MSG_BM_OCC, MSG_BM_FREE},      // 0x20
		{1, 0},                                                 // 0x21
		{1, 0},                                                 // 0x22
		{1, 0},                                                 // 0x23
		{1, 0},                                                 // 0x24
		{2, 7,  MSG_BM_CONFIDENCE},                             // 0x25
		{0},                                                    // 0x26
		{0},                                                    // 0x27
		{0},                                                    // 0x28
		{0},                                                    // 0x29
		{0},                                                    // 0x2A
		{0},                                                    // 0x2B
		{0},                                                    // 0x2C
		{0},                                                    // 0x2D
		{0},                                                    // 0x2E
		{0},                                                    // 0x2F

		//-- booster messages
		{2, 5,  MSG_BOOST_STAT},                                // 0x30
		{2, 5,  MSG_BOOST_STAT},                                // 0x31
		{2, 5,  MSG_BOOST_STAT},                                // 0x32
		{0},                                                    // 0x33
		{0},                                                    // 0x34
		{0},                                                    // 0x35
		{0},                                                    // 0x36
		{0},                                                    // 0x37

		//-- accessory control messages
		{2, 9,  MSG_ACCESSORY_STATE},                           // 0x38
		{2, 9,  MSG_ACCESSORY_STATE},                           // 0x39
		{2, 9,  MSG_ACCESSORY_PARA},                            // 0x3A
		{2, 9,  MSG_ACCESSORY_PARA},                            // 0x3B
		{0},                                                    // 0x3C
		{0},                                                    // 0x3D
		{0},                                                    // 0x3E

		//-- switch/light/servo control messages
		{1, 0},                                                 // 0x3F
		{3, 7,  MSG_LC_STAT, MSG_LC_NA},                        // 0x40
		{3, 10, MSG_LC_CONFIG, MSG_LC_NA},                      // 0x41
		{3, 10, MSG_LC_CONFIG, MSG_LC_NA},                      // 0x42
		{3, 6,  MSG_LC_KEY, MSG_LC_NA},                         // 0x43
		{3, 7,  MSG_LC_STAT, MSG_LC_NA},                        // 0x44
		{1, 0},                                                 // 0x45
		{2, 40, MSG_LC_CONFIGX},                                // 0x46
		{2, 40, MSG_LC_CONFIGX},                                // 0x47

		//-- macro messages
		{2, 6,  MSG_LC_MACRO_STATE},                            // 0x48
		{2, 10, MSG_LC_MACRO},                                  // 0x49
		{2, 10, MSG_LC_MACRO},                                  // 0x4A
		{2, 10, MSG_LC_MACRO_PARA},                             // 0x4B
		{2, 10, MSG_LC_MACRO_PARA},                             // 0x4C
		{0},                                                    // 0x4D
		{0},                                                    // 0x4E
		{0},                                                    // 0x4F
		{0},                                                    // 0x50
		{0},                                                    // 0x51
		{0},                                                    // 0x52
		{0},                                                    // 0x53
		{0},                                                    // 0x54
		{0},                                                    // 0x55
		{0},                                                    // 0x56
		{0},                                                    // 0x57
		{0},                                                    // 0x58
		{0},                                                    // 0x59
		{0},                                                    // 0x5A
		{0},                                                    // 0x5B
		{0},                                                    // 0x5C
		{0},                                                    // 0x5D
		{0},                                                    // 0x5E
		{0},                                                    // 0x5F

		//-- dcc gen messages
		{1, 0},                                                 // 0x60
		{0},                                                    // 0x61
		{2, 5,  MSG_CS_STATE},                                  // 0x62
		{0},                                                    // 0x63
		{2, 7,  MSG_CS_DRIVE_ACK},                              // 0x64
		{2, 7,  MSG_CS_ACCESSORY_ACK},                          // 0x65
		{2, 7,  MSG_CS_DRIVE_ACK},                              // 0x66
		{2, 10, MSG_CS_POM_ACK},                                // 0x67
		{2, 11, MSG_CS_RCPLUS_ACK},                             // 0x68
		{0},                                                    // 0x69
		{0},                                                    // 0x6A
		{0},                                                    // 0x6B
		{0},                                                    // 0x6C
		{0},                                                    // 0x6D
		{0},                                                    // 0x6E
		{2, 5,  MSG_CS_PROG_STATE},                             // 0x6F
		{0},                                                    // 0x70
		{0},                                                    // 0x71
		{0},                                                    // 0x72
		{0},                                                    // 0x73
		{0},                                                    // 0x74
		{0},                                                    // 0x75
		{0},                                                    // 0x76
		{0},                                                    // 0x77
		{0},                                                    // 0x78
		{0},                                                    // 0x79
		{0},                                                    // 0x7A
		{0},                                                    // 0x7B
		{0},                                                    // 0x7C
		{0},                                                    // 0x7D
		{0},                                                    // 0x7E
		{0},                                                    // 0x7F
};
