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


static const char UNKNOWN[] = "UNKNOWN_TYPE";

const char *const bidib_message_string_mapping[256] = {
		UNKNOWN,                                                // 0x00
		"MSG_SYS_GET_MAGIC",                                    // 0x01
		"MSG_SYS_GET_P_VERSION",                                // 0x02
		"MSG_SYS_ENABLE",                                       // 0x03
		"MSG_SYS_DISABLE",                                      // 0x04
		"MSG_SYS_GET_UNIQUE_ID",                                // 0x05
		"MSG_SYS_GET_SW_VERSION",                               // 0x06
		"MSG_SYS_PING",                                         // 0x07
		"MSG_SYS_IDENTIFY",                                     // 0x08
		"MSG_SYS_RESET",                                        // 0x09
		"MSG_GET_PKT_CAPACITY",                                 // 0x0A
		"MSG_NODETAB_GETALL",                                   // 0x0B
		"MSG_NODETAB_GETNEXT",                                  // 0x0C
		"MSG_NODE_CHANGED_ACK",                                 // 0x0D
		"MSG_SYS_GET_ERROR",                                    // 0x0E
		"MSG_FW_UPDATE_OP",                                     // 0x0F

		//-- feature and user config messages
		"MSG_FEATURE_GET_ALL",                                  // 0x10
		"MSG_FEATURE_GETNEXT",                                  // 0x11
		"MSG_FEATURE_GET",                                      // 0x12
		"MSG_FEATURE_SET",                                      // 0x13
		"MSG_VENDOR_ENABLE",                                    // 0x14
		"MSG_VENDOR_DISABLE",                                   // 0x15
		"MSG_VENDOR_SET",                                       // 0x16
		"MSG_VENDOR_GET",                                       // 0x17
		"MSG_SYS_CLOCK",                                        // 0x18
		"MSG_STRING_GET",                                       // 0x19
		"MSG_STRING_SET",                                       // 0x1A
		UNKNOWN,                                                // 0x1B
		UNKNOWN,                                                // 0x1C
		UNKNOWN,                                                // 0x1D
		UNKNOWN,                                                // 0x1E
		UNKNOWN,                                                // 0x1F

		//-- occupancy messages
		"MSG_BM_GET_RANGE",                                     // 0x20
		"MSG_BM_MIRROR_MULTIPLE",                               // 0x21
		"MSG_BM_MIRROR_OCC",                                    // 0x22
		"MSG_BM_MIRROR_FREE",                                   // 0x23
		"MSG_BM_ADDR_GET_RANGE",                                // 0x24
		"MSG_BM_GET_CONFIDENCE",                                // 0x25
		"MSG_BM_MIRROR_POSITION",                               // 0x26
		UNKNOWN,                                                // 0x27
		UNKNOWN,                                                // 0x28
		UNKNOWN,                                                // 0x29
		UNKNOWN,                                                // 0x2A
		UNKNOWN,                                                // 0x2B
		UNKNOWN,                                                // 0x2C
		UNKNOWN,                                                // 0x2D
		UNKNOWN,                                                // 0x2E
		UNKNOWN,                                                // 0x2F

		//-- booster messages
		"MSG_BOOST_OFF",                                        // 0x30
		"MSG_BOOST_ON",                                         // 0x31
		"MSG_BOOST_QUERY",                                      // 0x32
		UNKNOWN,                                                // 0x33
		UNKNOWN,                                                // 0x34
		UNKNOWN,                                                // 0x35
		UNKNOWN,                                                // 0x36
		UNKNOWN,                                                // 0x37

		//-- accessory control messages
		"MSG_ACCESSORY_SET",                                    // 0x38
		"MSG_ACCESSORY_GET",                                    // 0x39
		"MSG_ACCESSORY_PARA_SET",                               // 0x3A
		"MSG_ACCESSORY_PARA_GET",                               // 0x3B
		UNKNOWN,                                                // 0x3C
		UNKNOWN,                                                // 0x3D
		UNKNOWN,                                                // 0x3E

		//-- switch/light/servo control messages
		"MSG_LC_PORT_QUERY_ALL",                                // 0x3F
		"MSG_LC_OUTPUT",                                        // 0x40
		UNKNOWN,                                                // 0x41
		UNKNOWN,                                                // 0x42
		UNKNOWN,                                                // 0x43
		"MSG_LC_PORT_QUERY",                                    // 0x44
		"MSG_LC_CONFIGX_GET_ALL",                               // 0x45
		"MSG_LC_CONFIGX_SET",                                   // 0x46
		"MSG_LC_CONFIGX_GET",                                   // 0x47

		//-- macro messages
		"MSG_LC_MACRO_HANDLE",                                  // 0x48
		"MSG_LC_MACRO_SET",                                     // 0x49
		"MSG_LC_MACRO_GET",                                     // 0x4A
		"MSG_LC_MACRO_PARA_SET",                                // 0x4B
		"MSG_LC_MACRO_PARA_GET",                                // 0x4C
		UNKNOWN,                                                // 0x4D
		UNKNOWN,                                                // 0x4E
		UNKNOWN,                                                // 0x4F
		UNKNOWN,                                                // 0x50
		UNKNOWN,                                                // 0x51
		UNKNOWN,                                                // 0x52
		UNKNOWN,                                                // 0x53
		UNKNOWN,                                                // 0x54
		UNKNOWN,                                                // 0x55
		UNKNOWN,                                                // 0x56
		UNKNOWN,                                                // 0x57
		UNKNOWN,                                                // 0x58
		UNKNOWN,                                                // 0x59
		UNKNOWN,                                                // 0x5A
		UNKNOWN,                                                // 0x5B
		UNKNOWN,                                                // 0x5C
		UNKNOWN,                                                // 0x5D
		UNKNOWN,                                                // 0x5E
		UNKNOWN,                                                // 0x5F

		//-- dcc gen messages
		"MSG_CS_ALLOCATE",                                      // 0x60
		UNKNOWN,                                                // 0x61
		"MSG_CS_SET_STATE",                                     // 0x62
		UNKNOWN,                                                // 0x63
		"MSG_CS_DRIVE",                                         // 0x64
		"MSG_CS_ACCESSORY",                                     // 0x65
		"MSG_CS_BIN_STATE",                                     // 0x66
		"MSG_CS_POM",                                           // 0x67
		"MSG_CS_RCPLUS",                                        // 0x68
		UNKNOWN,                                                // 0x69
		UNKNOWN,                                                // 0x6A
		UNKNOWN,                                                // 0x6B
		UNKNOWN,                                                // 0x6C
		UNKNOWN,                                                // 0x6D
		UNKNOWN,                                                // 0x6E
		"MSG_CS_PROG",                                          // 0x6F
		UNKNOWN,                                                // 0x70
		UNKNOWN,                                                // 0x71
		UNKNOWN,                                                // 0x72
		UNKNOWN,                                                // 0x73
		UNKNOWN,                                                // 0x74
		UNKNOWN,                                                // 0x75
		UNKNOWN,                                                // 0x76
		UNKNOWN,                                                // 0x77
		UNKNOWN,                                                // 0x78
		UNKNOWN,                                                // 0x79
		UNKNOWN,                                                // 0x7A
		UNKNOWN,                                                // 0x7B
		UNKNOWN,                                                // 0x7C
		UNKNOWN,                                                // 0x7D
		UNKNOWN,                                                // 0x7E
		UNKNOWN,                                                // 0x7F

		//-- UPLINK

		//-- system messages
		UNKNOWN,                                                // 0x80
		"MSG_SYS_MAGIC",                                        // 0x81
		"MSG_SYS_PONG",                                         // 0x82
		"MSG_SYS_P_VERSION",                                    // 0x83
		"MSG_SYS_UNIQUE_ID",                                    // 0x84
		"MSG_SYS_SW_VERSION",                                   // 0x85
		"MSG_SYS_ERROR",                                        // 0x86
		"MSG_SYS_IDENTIFY_STATE",                               // 0x87
		"MSG_NODETAB_COUNT",                                    // 0x88
		"MSG_NODETAB",                                          // 0x89
		"MSG_PKT_CAPACITY",                                     // 0x8A
		"MSG_NODE_NA",                                          // 0x8B
		"MSG_NODE_LOST",                                        // 0x8C
		"MSG_NODE_NEW",                                         // 0x8D
		"MSG_STALL",                                            // 0x8E
		"MSG_FW_UPDATE_STAT",                                   // 0x8F

		//-- feature and user config messages
		"MSG_FEATURE",                                          // 0x90
		"MSG_FEATURE_NA",                                       // 0x91
		"MSG_FEATURE_COUNT",                                    // 0x92
		"MSG_VENDOR",                                           // 0x93
		"MSG_VENDOR_ACK",                                       // 0x94
		"MSG_STRING",                                           // 0x95
		UNKNOWN,                                                // 0x96
		UNKNOWN,                                                // 0x97
		UNKNOWN,                                                // 0x98
		UNKNOWN,                                                // 0x99
		UNKNOWN,                                                // 0x9A
		UNKNOWN,                                                // 0x9B
		UNKNOWN,                                                // 0x9C
		UNKNOWN,                                                // 0x9D
		UNKNOWN,                                                // 0x9E
		UNKNOWN,                                                // 0x9F

		//-- occupancy messages
		"MSG_BM_OCC",                                           // 0xA0
		"MSG_BM_FREE",                                          // 0xA1
		"MSG_BM_MULTIPLE",                                      // 0xA2
		"MSG_BM_ADDRESS",                                       // 0xA3
		UNKNOWN,                                                // 0xA4
		"MSG_BM_CV",                                            // 0xA5
		"MSG_BM_SPEED",                                         // 0xA6
		"MSG_BM_CURRENT",                                       // 0xA7
		"MSG_BM_XPOM",                                          // 0xA8
		"MSG_BM_CONFIDENCE",                                    // 0xA9
		"MSG_BM_DYN_STATE",                                     // 0xAA
		"MSG_BM_RCPLUS",                                        // 0xAB
		"MSG_BM_POSITION",                                      // 0xAC
		UNKNOWN,                                                // 0xAD
		UNKNOWN,                                                // 0xAE
		UNKNOWN,                                                // 0xAF

		//-- booster messages
		"MSG_BOOST_STAT",                                       // 0xB0
		"MSG_BOOST_CURRENT",                                    // 0xB1
		"MSG_BOOST_DIAGNOSTIC",                                 // 0xB2
		UNKNOWN,                                                // 0xB3
		UNKNOWN,                                                // 0xB4
		UNKNOWN,                                                // 0xB5
		UNKNOWN,                                                // 0xB6
		UNKNOWN,                                                // 0xB7

		//-- accessory control messages
		"MSG_ACCESSORY_STATE",                                  // 0xB8
		"MSG_ACCESSORY_PARA",                                   // 0xB9
		"MSG_ACCESSORY_NOTIFY",                                 // 0xBA
		UNKNOWN,                                                // 0xBB
		UNKNOWN,                                                // 0xBC
		UNKNOWN,                                                // 0xBD
		UNKNOWN,                                                // 0xBE
		UNKNOWN,                                                // 0xBF

		//-- switch/light/servo control messages
		"MSG_LC_STAT",                                          // 0xC0
		"MSG_LC_NA",                                            // 0xC1
		"MSG_LC_CONFIG",                                        // 0xC2
		"MSG_LC_KEY",                                           // 0xC3
		"MSG_LC_WAIT",                                          // 0xC4
		UNKNOWN,                                                // 0xC5
		"MSG_LC_CONFIGX",                                       // 0xC6
		UNKNOWN,                                                // 0xC7

		//-- macro messages
		"MSG_LC_MACRO_STATE",                                   // 0xC8
		"MSG_LC_MACRO",                                         // 0xC9
		"MSG_LC_MACRO_PARA",                                    // 0xCA
		UNKNOWN,                                                // 0xCB
		UNKNOWN,                                                // 0xCC
		UNKNOWN,                                                // 0xCD
		UNKNOWN,                                                // 0xCE
		UNKNOWN,                                                // 0xCF
		UNKNOWN,                                                // 0xD0
		UNKNOWN,                                                // 0xD1
		UNKNOWN,                                                // 0xD2
		UNKNOWN,                                                // 0xD3
		UNKNOWN,                                                // 0xD4
		UNKNOWN,                                                // 0xD5
		UNKNOWN,                                                // 0xD6
		UNKNOWN,                                                // 0xD7
		UNKNOWN,                                                // 0xD8
		UNKNOWN,                                                // 0xD9
		UNKNOWN,                                                // 0xDA
		UNKNOWN,                                                // 0xDB
		UNKNOWN,                                                // 0xDC
		UNKNOWN,                                                // 0xDD
		UNKNOWN,                                                // 0xDE
		UNKNOWN,                                                // 0xDF

		//-- dcc gen messages
		UNKNOWN,                                                // 0xE0
		"MSG_CS_STATE",                                         // 0xE1
		"MSG_CS_DRIVE_ACK",                                     // 0xE2
		"MSG_CS_ACCESSORY_ACK",                                 // 0xE3
		"MSG_CS_POM_ACK",                                       // 0xE4
		"MSG_CS_DRIVE_MANUAL",                                  // 0xE5
		"MSG_CS_DRIVE_EVENT",                                   // 0xE6
		"MSG_CS_ACCESSORY_MANUAL",                              // 0xE7
		"MSG_CS_RCPLUS_ACK",                                    // 0xE8
		UNKNOWN,                                                // 0xE9
		UNKNOWN,                                                // 0xEA
		UNKNOWN,                                                // 0xEB
		UNKNOWN,                                                // 0xEC
		UNKNOWN,                                                // 0xED
		UNKNOWN,                                                // 0xEE
		"MSG_CS_PROG_STATE",                                    // 0xEF
		UNKNOWN,                                                // 0xF0
		UNKNOWN,                                                // 0xF1
		UNKNOWN,                                                // 0xF2
		UNKNOWN,                                                // 0xF3
		UNKNOWN,                                                // 0xF4
		UNKNOWN,                                                // 0xF5
		UNKNOWN,                                                // 0xF6
		UNKNOWN,                                                // 0xF7
		UNKNOWN,                                                // 0xF8
		UNKNOWN,                                                // 0xF9
		UNKNOWN,                                                // 0xFA
		UNKNOWN,                                                // 0xFB
		UNKNOWN,                                                // 0xFC
		UNKNOWN,                                                // 0xFD
		UNKNOWN,                                                // 0xFE
		UNKNOWN                                                 // 0xFF
};

const char *const bidib_error_string_mapping[0x31] = {
		"BIDIB_ERR_NONE",                                       // 0x00
		"BIDIB_ERR_TXT",                                        // 0x01
		"BIDIB_ERR_CRC",                                        // 0x02
		"BIDIB_ERR_SIZE",                                       // 0x03
		"BIDIB_ERR_SEQUENCE",                                   // 0x04
		"BIDIB_ERR_PARAMETER",                                  // 0x05
		UNKNOWN,                                                // 0x06
		UNKNOWN,                                                // 0x07
		UNKNOWN,                                                // 0x08
		UNKNOWN,                                                // 0x09
		UNKNOWN,                                                // 0x0A
		UNKNOWN,                                                // 0x0B
		UNKNOWN,                                                // 0x0C
		UNKNOWN,                                                // 0x0D
		UNKNOWN,                                                // 0x0E
		UNKNOWN,                                                // 0x0F
		"BIDIB_ERR_BUS",                                        // 0x10
		"BIDIB_ERR_ADDRSTACK",                                  // 0x11
		"BIDIB_ERR_IDDOUBLE",                                   // 0x12
		"BIDIB_ERR_SUBCRC",                                     // 0x13
		"BIDIB_ERR_SUBTIME",                                    // 0x14
		"BIDIB_ERR_SUBPAKET",                                   // 0x15
		"BIDIB_ERR_OVERRUN",                                    // 0x16
		UNKNOWN,                                                // 0x17
		UNKNOWN,                                                // 0x18
		UNKNOWN,                                                // 0x19
		UNKNOWN,                                                // 0x1A
		UNKNOWN,                                                // 0x1B
		UNKNOWN,                                                // 0x1C
		UNKNOWN,                                                // 0x1D
		UNKNOWN,                                                // 0x1E
		UNKNOWN,                                                // 0x1F
		"BIDIB_ERR_HW",                                         // 0x20
		"BIDIB_ERR_RESET_REQUIRED",                             // 0x21
		UNKNOWN,                                                // 0x22
		UNKNOWN,                                                // 0x23
		UNKNOWN,                                                // 0x24
		UNKNOWN,                                                // 0x25
		UNKNOWN,                                                // 0x26
		UNKNOWN,                                                // 0x27
		UNKNOWN,                                                // 0x28
		UNKNOWN,                                                // 0x29
		UNKNOWN,                                                // 0x2A
		UNKNOWN,                                                // 0x2B
		UNKNOWN,                                                // 0x2C
		UNKNOWN,                                                // 0x2D
		UNKNOWN,                                                // 0x2E
		UNKNOWN,                                                // 0x2F
		"BIDIB_ERR_NO_SECACK_BY_HOST"                           // 0x30
};
