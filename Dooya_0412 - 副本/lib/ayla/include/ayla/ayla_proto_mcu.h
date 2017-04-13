/*
 * Copyright 2011 Ayla Networks, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are
 * those of the authors and should not be interpreted as representing official
 * policies, either expressed or implied, of Ayla Networks, Inc.
 */
#ifndef __AYLA_PROTO_MCU_H__
#define __AYLA_PROTO_MCU_H__

#define ASPI_LEN_MAX	384	/* max length, arbitrary for debugging */
#define ADS_BIT			1	/* destination bit for service */
#define NOT_ADS_BIT		0xFE	/* all destinations except service */
#define ALL_DESTS_BIT		0xFF	/* all destinations */

/*
 * Protocols.
 */
enum aspi_proto {
	ASPI_PROTO_CMD = 0,     /* command/control operations */
	ASPI_PROTO_DATA = 1,    /* Ayla Data service operations */
	ASPI_PROTO_PING = 2,	/* echo test protocol */
	ASPI_PROTO_LOG = 3      /* send log message */
};

/*
 * Ayla cmd structure:
 */
struct ayla_cmd {
	u8	protocol;	/* protocol: see below */
	u8	opcode;		/* opcode: see below */
	be16	req_id;		/* request identifier */
} PACKED;

/*
 * Command opcodes
 */
enum ayla_cmd_op {
	/*
	 * Control ops.
	 */
	ACMD_NOP = 0,		/* no-op */
	ACMD_RESP = 1,		/* response message from slave */
	ACMD_GET_CONF = 2,	/* get configuration item */
	ACMD_SET_CONF = 3,	/* set configuration item */
	ACMD_SAVE_CONF = 4,	/* save running configuration for startup */
	ACMD_GET_STAT = 5,	/* get status variable */
	ACMD_NAK = 6,		/* negative acknowledgment with error */
	ACMD_LOAD_STARTUP = 7,	/* load startup configuration and reset */
	ACMD_LOAD_FACTORY = 8,	/* load factory configuration and reset */
	ACMD_OTA_STAT = 9,	/* status regarding OTA firmware updates */
	ACMD_OTA_CMD = 0xa,	/* update module with available OTA */
	ACMD_COMMIT = 0xb,	/* commit configuration change */
	ACMD_LOG = 0xc,		/* logging operations */
	ACMD_MCU_OTA = 0xd,	/* MCU firmware update report/start */
	ACMD_MCU_OTA_LOAD = 0xe, /* MCU firmware image part */
	ACMD_MCU_OTA_STAT = 0xf, /* report status of MCU fw update */
	ACMD_MCU_OTA_BOOT = 0x10, /* boot to a different image */
	ACMD_CONF_UPDATE = 0x11, /* config update */
	ACMD_WIFI_JOIN = 0x12,	/* configure and try to join a wifi network */
	ACMD_WIFI_DELETE = 0x13	/* leave and forget a wifi network */
};

enum ayla_data_op {

	/*
	 * Ayla Data Service operations
	 */
	AD_REQ_TLV = 2,		/* request TLVs from data service */

	/*
	 * Commands from Slave module to MCU master
	 */
	AD_RECV_TLV = 3,	/* TLV values from data service */
	__AD_RESVD_ACK = 4,	/* Obsolete - Acknowledgement */
	AD_NAK = 5,		/* Negative Acknowledgement */
	AD_SEND_PROP = 6,	/* send property */
	AD_SEND_PROP_RESP = 7,	/* response to send-property request */
	AD_SEND_NEXT_PROP = 8,	/* request to send next property */
	AD_SEND_TLV =	0x09,	/* send TLV to data service */
	AD_DP_REQ =	0x0a,	/* Request data point value */
	AD_DP_RESP =	0x0b,	/* Response for data point request */
	AD_DP_CREATE =	0x0c,	/* Create a new data point */
	AD_DP_FETCHED =	0x0d,	/* Indicate current data point fetched */
	AD_DP_STOP =	0x0e,	/* Stop current data point transfer */
	AD_DP_SEND =	0x0f,	/* Send data point value */ 
	AD_CONNECT =	0x11,	/* connectivity status info */
	AD_ECHO_FAIL =	0x12,	/* echo failure for a prop */
	AD_LISTEN_ENB =	0x13,	/* enable prop/cmd fetches from ads */
	AD_ERROR =	0x14,	/*
				 * General error. Not tied to a specific
				 * request from MCU.
				 */
	AD_CONFIRM =	0x15,	/* Confirmation of successful dp post/put */
	AD_PROP_NOTIFY = 0x16	/* Indication of pending prop update */
};

/*
 * Ayla TLV for commands.
 */
struct ayla_tlv {
	u8	type;		/* type code */
	u8	len;		/* length of value */
				/* value follows immediately */
} PACKED;

#define TLV_VAL(tlv)	((void *)(tlv + 1))
#define TLV_MAX_LEN	255
#define TLV_MAX_STR_LEN	1024

enum ayla_tlv_type {
	ATLV_INVALID = 0,	/* Invalid TLV type */
	ATLV_NAME = 1,		/* variable name, UTF-8 */
	ATLV_INT = 2,		/* integer, with length 1, 2, 4, or 8 */
	ATLV_UINT = 3,		/* unsigned integer.  Not used for SPI */
	ATLV_BIN = 4,		/* unstructured bytes */
	ATLV_UTF8 = 5,		/* text */
	ATLV_CONF = 6,		/* configuration name indices */
	ATLV_ERR = 7,		/* error number */
	ATLV_FORMAT = 8,	/* formatting hint */
	ATLV_FRAG = 9,		/* fragment descriptor for longer values */
	ATLV_NOP = 0x0a,	/* no-op, ignored TLV inserted for alignment */
	ATLV_FLOAT = 0x0b,	/* IEEE floating point value */
	ATLV_BOOL = 0x0f,	/* boolean, length 1, value 1 or 0 */
	ATLV_CONT = 0x10,	/* continuation token for AD_SEND_NEXT_PROP */
	ATLV_OFF = 0x11,	/* offset in data point or other transfer */
	ATLV_LEN = 0x12,	/* length of data point or other transfer */
	ATLV_LOC = 0x13,	/* location of data point or other item */
	ATLV_EOF = 0x14,	/* end of file, e.g., end of data point */
	ATLV_BCD = 0x15,	/* fixed-point decimal number */
	ATLV_CENTS = 0x16,	/* integer value 100 times the actual value */
	ATLV_NODES = 0x17,	/* bitmap of dests or src for prop updates */
	ATLV_ECHO = 0x18,	/* indicates prop update is an echo, len = 0 */
	ATLV_FEATURES = 0x19,	/* one-byte bitmap of the supported features */
	ATLV_SCHED = 0x20,	/* schedule property */
	ATLV_UTC = 0x21,	/* indicates date/time in schedules are UTC */
	ATLV_AND = 0x22,	/* ANDs the top two conditions in schedule */
	ATLV_DISABLE = 0x23,	/* disables the schedule */
	ATLV_INRANGE = 0x24,	/* stack is true if current time is in range */
	ATLV_ATSTART = 0x25,	/* stack is true if current time is at start */
	ATLV_ATEND = 0x26,	/* stack is true if current time is at end */
	ATLV_STARTDATE = 0x27,	/* date must be after value */
	ATLV_ENDDATE = 0x28,	/* date must be before value */
	ATLV_DAYSOFMON = 0x29,	/* 32-bit mask indicating which day of month */
	ATLV_DAYSOFWK = 0x2a,	/* days of week specified as 7-bit mask */
	ATLV_DAYOCOFMO = 0x2b,	/* day occurrence in month */
	ATLV_MOOFYR = 0x2c,	/* months of year */
	ATLV_STTIMEEACHDAY = 0x2d,	/* time of day must be after value */
	ATLV_ENDTIMEEACHDAY = 0x2e,	/* time of day must be before value */
	ATLV_DURATION = 0x2f,	/* must not last more than this (secs) */
	ATLV_TIMEBFEND = 0x30,	/* time must be <value> secs before end */
	ATLV_INTERVAL = 0x31,	/* start every <value> secs since start */
	ATLV_SETPROP = 0x32,	/* value is the property to be toggled */
	ATLV_VERSION = 0x33,	/* version of schedule */
	ATLV_FILE = 0x80	/* mask for 0x80 thru 0xff incl. 15-bit len */
};

/*
 * TLV for a 32-bit integer.
 */
struct ayla_tlv_int {
	struct ayla_tlv head;
	be32	data;
} PACKED;

/*
 * Formatting-hint TLV.
 * This TLV should be between the name and value TLVs, if used.
 * It is only a hint for automatically-generated web pages.
 */
struct ayla_tlv_fmt {
	struct ayla_tlv head;
	u8	fmt_flags;	/* formatting hint flag (see below) */
} PACKED;

/*
 * fmt_flag values.
 */
#define AFMT_READ_ONLY	(1 << 0) /* indicates variable is not settable */
#define AFMT_HEX	(1 << 1) /* value should be formatted in hex */

/*
 * Error numbers for NAKs.
 */
#define AERR_TIMEOUT	1	/* timeout with data service */
#define AERR_LEN_ERR	2	/* TLV extends past end of received buffer */
#define AERR_UNK_TYPE	3	/* unknown TLV type */
#define AERR_UNK_VAR	4	/* unknown property or variable name */
#define AERR_INVAL_TLV	5	/* invalid TLV sequence (e.g., not conf) */
#define AERR_INVAL_OP	6	/* invalid opcode */
#define AERR_INVAL_DP	7	/* invalid data point location */
#define AERR_INVAL_OFF	8	/* invalid offset */
#define AERR_INVAL_REQ	9	/* invalid request sequence */
#define AERR_INVAL_NAME 0x0a	/* invalid property name */
#define AERR_CONN_ERR	0x0b	/* connection to the ADS fail */
#define AERR_ADS_BUSY	0x0c	/* ADS busy */
#define AERR_INTERNAL	0x0d	/* internal error */
#define AERR_CHECKSUM	0x0e	/* checksum error */
#define AERR_ALREADY	0x0f	/* already done */
#define AERR_BOOT	0x10	/* MCU did not boot to new image */
#define AERR_OVERFLOW	0x11	/*
				 * MCU took too long to recv data response from
				 * the request it made to the module. It needs
				 * to re-do the request.
				 */

/* Features Supported by the MCU (Used in ATLV_FEATURES TLV) */
#define MCU_LAN_SUPPORT		1	/* MCU supports LAN-mode */
#define MCU_OTA_SUPPORT		2	/* MCU supports host OTA upgragdes */
#define MCU_TIME_SUBSCRIPTION	4	/* MCU wants time-related updates */
#define MCU_DATAPOINT_CONFIRM	8	/* MCU wants confirmation on dp posts */

#endif /* __AYLA_PROTO_MCU_H__ */
