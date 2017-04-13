/*
 * Copyright 2012 Ayla Networks, Inc.  All rights reserved.
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
#include <string.h>

#include <ayla/mcu_platform.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/serial_msg.h>

#ifdef AYLA_UART
#include <ayla/uart.h>
#else
#include <ayla/spi.h>
#endif

#include <ayla/byte_order.h>

struct prop_dp *prop_dp_active;

#define PROP_DP_STATS

#ifndef PROP_DP_STATS
#define STATS(x)
#else
#define STATS(x)	do { (prop_dp_stats.x++); } while (0)

/*
 * Optional debug counters for various conditions.
 * These can be deleted to save space but may help debugging.
 */
struct prop_dp_stats {
	u16 rx_resp;
	u16 rx_not_active;
	u16 rx_bad_req_id;
	u16 rx_len_err;

	u16 rx_tlv_len_err;
	u16 rx_no_loc;
	u16 rx_bad_loc;
	u16 rx_wrong_type;

	u16 rx_read_only;
	u16 rx_unk_tlv;
} prop_dp_stats;
#endif /* PROP_DP_STATS */

void prop_dp_init(struct prop_dp *dp,
	int (*prop_get)(struct prop *prop, size_t off, void *buf, size_t len),
	int (*prop_set)(struct prop *prop, size_t off, void *buf, size_t len, u8 eof))
{
	dp->state = DS_IDLE;
	dp->prop_get = prop_get;
	dp->prop_set = prop_set;
}

static void prop_dp_done(struct prop_dp *dp)
{
	dp->state = DS_IDLE;
	dp->prop->send_mask = 0;
	if (prop_dp_active == dp) {
		prop_dp_active = NULL;
	}
}

/*
 * prop_dp_abort() - abort any ongoing file operation
 */
int prop_dp_abort(void)
{
	struct ayla_cmd *cmd;
	int rc;

	if (!prop_dp_active) {
		/* nothing to abort */
		return 0;
	}
	if (prop_dp_active->state == DS_FETCHED) {
		/*
		 * we've already fetched the whole datapoint
		 * all that is left is marking it fetched
		 * doesn't make sense to abort it in this case.
		 */
		 return 0;
	}
	prop_dp_active->state = DS_ABORTING;
	cmd = serial_tx_buf_get(sizeof(*cmd), 0, NULL);
	if (!cmd) {
		return -1;
	}
	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_DP_STOP;
	put_ua_be16((void *)&cmd->req_id, prop_dp_active->req_id);
	rc = serial_tx_buf_send(0, cmd->req_id);
	if (!rc) {
		prop_dp_active->aborted = 1;
		prop_dp_done(prop_dp_active);
	}
	return rc;
}

static void prop_dp_fatal_err(struct prop_dp *dp)
{
	prop_dp_done(dp);
}

static int prop_dp_create_tx(struct prop_dp *dp)
{
	struct prop *prop;
	struct ayla_cmd *cmd;
	struct ayla_tlv *name;
	u16 name_len;
	int tot_len;
	int rc;

	prop = dp->prop;
	name_len = strlen(prop->name);
	tot_len = sizeof(*cmd) + sizeof(*name) + name_len;
	if (tot_len > ASPI_LEN_MAX) {
		dp->prop->send_mask = 0;
		return 0;
	}

	cmd = serial_tx_buf_get(tot_len, 1, NULL);
	if (!cmd) {
		return -1;
	}

	dp->state = DS_CREATE_RESP;
	dp->prop->send_mask = 0;

	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_DP_CREATE;
	tlv_req_id++;
	dp->req_id = tlv_req_id;
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);

	name = (struct ayla_tlv *)(cmd + 1);
	name->type = ATLV_NAME;
	name->len = name_len;
	memcpy(name + 1, prop->name, name_len);

	rc = serial_tx_buf_send(1, dp->req_id);
	if (!rc) {
		/*
		 * create + dp_send must be sent one after another
		 * this lets props.c know not to do anything else
		 */
		prop_dp_active = dp;
		dp->aborted = 0;
	}
	return rc;
}

static int prop_dp_data_tx(struct prop_dp *dp)
{
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	s32 val_len;
	s32 old_len;
	u32 max_len;
	size_t tot_len;
	size_t next_off;
	int eof;
	int rc;

	tot_len = sizeof(*cmd) + sizeof(*tlv) + dp->loc_len;
	val_len = dp->tot_len;
	next_off = dp->next_off;
	if (next_off || val_len) {
		tot_len += sizeof(*tlv) + sizeof(u32);
	}

	/*
	 * determine how much of the value we can send.
	 */
	eof = 0;
	if (!val_len) {
		val_len = MAX_U8;
	} else {
		max_len = val_len - next_off;
		if (max_len > MAX_U8) {
			max_len = MAX_U8;
		}
		if (val_len > max_len) {
			val_len = max_len;
		}
		if (next_off + val_len == dp->tot_len &&
		    max_len + tot_len + sizeof(*tlv) < ASPI_LEN_MAX) {
			tot_len += sizeof(*tlv);
			eof = 1;
		}
	}
	tot_len += sizeof(*tlv) + val_len;

	cmd = serial_tx_buf_get(tot_len, 1, NULL);
	if (!cmd) {
		return -1;
	}

	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_DP_SEND;
	put_ua_be16((void *)&cmd->req_id, dp->req_id);

	tlv = (struct ayla_tlv *)(cmd + 1);
	tlv->type = ATLV_LOC;
	tlv->len = dp->loc_len;
	memcpy(tlv + 1, dp->loc, dp->loc_len);

	if (next_off) {
		tlv = (struct ayla_tlv *)((u8 *)(tlv + 1) + tlv->len);
		tlv->type = ATLV_OFF;
		tlv->len = sizeof(u32);
		put_ua_be32(tlv + 1, next_off);
	} else if (dp->tot_len) {
		tlv = (struct ayla_tlv *)((u8 *)(tlv + 1) + tlv->len);
		tlv->type = ATLV_LEN;
		tlv->len = sizeof(u32);
		put_ua_be32(tlv + 1, dp->tot_len);
	}

	tlv = (struct ayla_tlv *)((u8 *)(tlv + 1) + tlv->len);
	tlv->type = ATLV_BIN;
	old_len = val_len;
	val_len = dp->prop_get(dp->prop, next_off, tlv + 1, val_len);
	if (val_len < 0) {		/* XXX can't happen? */
		prop_dp_fatal_err(dp);
#ifndef AYLA_UART
		serial_tx_cancel();
#endif
		return 0;
	}
	tlv->len = (u8)val_len;
	if (val_len < old_len) {
		if (!eof) {
			tot_len += sizeof(*tlv);
			eof = 1;
		}
#ifdef AYLA_UART
		uart_tx_buf_trim(tot_len + val_len - old_len);
#else
		spi_tx_buf_trim(tot_len + val_len - old_len);
#endif
	}

	if (eof) {
		tlv = (struct ayla_tlv *)((u8 *)(tlv + 1) + tlv->len);
		tlv->type = ATLV_EOF;
		tlv->len = 0;
	}

	/*
	 * Send the packet.
	 */
	rc = serial_tx_buf_send(1, dp->req_id);
	if (!rc) {
		if (eof) {
			prop_dp_done(dp);
		} else {
			dp->next_off = next_off + val_len;
		}
	}
	return rc;
}

/*
 * Send a request for a long data point.
 * The request contains a location TLV, and possibly an offset TLV.
 */
static int prop_dp_request(struct prop_dp *dp)
{
	struct prop *prop;
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	int tot_len;
	int rc;

	prop = dp->prop;
	tot_len = sizeof(*cmd) + sizeof(*tlv) + dp->loc_len;
	if (dp->next_off) {
		tot_len += sizeof(*tlv) + sizeof(u32);
	}
	if (tot_len > ASPI_LEN_MAX) {
		prop_dp_fatal_err(dp);
		return 0;
	}

	cmd = serial_tx_buf_get(tot_len, 1, NULL);
	if (!cmd) {
		return -1;
	}

	dp->state = DS_RECV;
	prop->send_mask = 0;

	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_DP_REQ;
	tlv_req_id++;
	dp->req_id = tlv_req_id;
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);

	tlv = (struct ayla_tlv *)(cmd + 1);
	tlv->type = ATLV_LOC;
	tlv->len = dp->loc_len;;
	memcpy(tlv + 1, dp->loc, dp->loc_len);

	if (dp->next_off) {
		tlv = (struct ayla_tlv *)((u8 *)(tlv + 1) + tlv->len);
		tlv->type = ATLV_OFF;
		put_ua_be32((void *)(tlv + 1), dp->next_off);
	}

	rc = serial_tx_buf_send(1, dp->req_id);
	if (!rc) {
		prop_dp_active = dp;
		dp->aborted = 0;
	}
	return rc;
}

/*
 * Send Datapoint Fetched operation to the module.
 */
static int prop_dp_fetched(struct prop_dp *dp)
{
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	int tot_len;
	int rc;

	tot_len = sizeof(*cmd) + sizeof(*tlv) + dp->loc_len;
	if (tot_len > ASPI_LEN_MAX) {
		prop_dp_fatal_err(dp);
		return 0;
	}

	cmd = serial_tx_buf_get(tot_len, 1, NULL);
	if (!cmd) {
		return -1;
	}

	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_DP_FETCHED;
	tlv_req_id++;
	dp->req_id = tlv_req_id;
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);

	tlv = (struct ayla_tlv *)(cmd + 1);
	tlv->type = ATLV_LOC;
	tlv->len = dp->loc_len;;
	memcpy(tlv + 1, dp->loc, dp->loc_len);

	rc = serial_tx_buf_send(1, dp->req_id);
	if (!rc) {
		prop_dp_done(dp);
	}
	return rc;
}

static int prop_dp_step(struct prop_dp *dp)
{
	switch (dp->state) {

	case DS_IDLE:
		prop_dp_done(dp);
		return 0;

	/*
	 * CREATE: send "datapoint create" command.
	 * This will callback to set the location for the datapoint.
 	 * Turn off send_mask until the location response is received.
	 */
	case DS_CREATE:
		return prop_dp_create_tx(dp);

	case DS_CREATE_RESP:
	case DS_RECV:
		return 0;

	case DS_SEND:
		return prop_dp_data_tx(dp);

	case DS_REQUEST:
		return prop_dp_request(dp);

	case DS_FETCHED:
		return prop_dp_fetched(dp);

	case DS_ABORTING:
		return prop_dp_abort();
	}

	return 0;
}

u8 prop_dp_last_err;		/* debug */

/*
 * Receive NAK (negative acknowlegement) for a datapoint operation.
 */
int prop_dp_nak(struct ayla_cmd *cmd, void *buf, size_t len)
{
	struct prop_dp *dp = prop_dp_active;
	struct ayla_tlv *tlv;
	u8 error;

	if (!dp) {
		return -2;
	}
	if (ntohs(cmd->req_id) != dp->req_id) {
		return -1;
	}
	tlv = buf;
	if (len < sizeof(*tlv) || len < sizeof(*tlv) + tlv->len) {
		return -1;		/* XXX invalid NAK */
	}
	if (tlv->type != ATLV_ERR) {
		return 0; 
	}
	error = *(u8 *)(tlv + 1);
	prop_dp_last_err = error;
	prop_dp_fatal_err(dp);

	return 0;
}


/*
 * Receive response from datapoint create request.
 * Caller has verified our state and the request ID.
 */
static void prop_dp_create_resp(struct prop_dp *dp, void *buf, size_t len)
{
	struct ayla_tlv *loc;

	loc = buf;
	if (len < sizeof(*loc) || len < sizeof(*loc) + loc->len) {
		goto bad_response;
	}
	if (loc->type != ATLV_LOC) {
		goto bad_response;
	}
	if (loc->len > sizeof(dp->loc) - 1) {
bad_response:
		prop_dp_fatal_err(dp);
		return;
	}
	memcpy(dp->loc, loc + 1, loc->len);
	dp->loc[loc->len] = '\0';
	dp->loc_len = loc->len;
	dp->state = DS_SEND;
	dp->next_off = 0;
	dp->req_id = tlv_req_id++;		/* req ID for all sends */
	dp->prop->send_mask = ADS_BIT;
}

/*
 * Receive response for data point request.
 */
static void prop_dp_rx(struct prop_dp *dp, void *buf, size_t len)
{
	struct prop *prop;
	struct ayla_tlv *tlv = (struct ayla_tlv *)buf;
	struct ayla_tlv *loc = NULL;
	size_t offset = 0;
	u8 eof = 0;
	void *valp = NULL;
	size_t val_len = 0;
	size_t rlen = len;
	size_t tlen;

	prop = dp->prop;
	while (rlen > 0) {
		if (rlen < sizeof(*tlv)) {
			STATS(rx_len_err);
			return;
		}

		tlen = tlv->len;
		if (tlen + sizeof(*tlv) > rlen) {
			STATS(rx_tlv_len_err);
			return;
		}

		switch (tlv->type) {
		case ATLV_LOC:
			loc = tlv;
			break;
		case ATLV_BIN:
			valp = (void *)(tlv + 1);
			val_len = tlv->len;
			break;
		case ATLV_OFF:
			if (tlv->len != sizeof(u32)) {
				STATS(rx_tlv_len_err);
				return;
			}
			offset = get_ua_be32((be32 *)(tlv + 1));
			break;
		case ATLV_EOF:
			eof = 1;
			break;
		default:
			STATS(rx_unk_tlv);
			break;
		}
		tlv = (void *)((char *)(tlv + 1) + tlen);
		rlen -= sizeof(*tlv) + tlen;
	}

	if (!loc) {
		STATS(rx_no_loc);
		return;
	}
	if (loc->len != dp->loc_len || memcmp(loc + 1, dp->loc, dp->loc_len)) {
		STATS(rx_bad_loc);
		return;
	}
	if (!dp->prop_set) {
		STATS(rx_read_only);
		return;
	}
	if (eof) {
		dp->state = DS_FETCHED;
		prop->send_mask = ADS_BIT;
	}
	dp->prop_set(prop, offset, valp, val_len, eof);
}

/*
 * Receive response from datapoint create or status request.
 */
void prop_dp_resp(struct ayla_cmd *cmd, void *buf, size_t len)
{
	struct prop_dp *dp = prop_dp_active;

	STATS(rx_resp);
	if (!dp) {
		STATS(rx_not_active);
		return;
	}
	if (ntohs(cmd->req_id) != dp->req_id) {
		STATS(rx_bad_req_id);
		return;
	}
	switch (dp->state) {
	case DS_CREATE_RESP:
		prop_dp_create_resp(dp, buf, len);
		break;
	case DS_RECV:
		prop_dp_rx(dp, buf, len);
		break;
	default:
		break;
	}
}

/*
 * Initiate or continue send or receive of large data point.
 * Never return 0, so prop->send_mask doesn't get cleared by caller.
 * We clear it ourselves after finishing the send.
 */
int prop_dp_send(struct prop *prop, void *arg)
{
	struct prop_dp *dp = prop->arg;
	int rc;

	if (prop_dp_active && prop_dp_active != dp) {
		/*
		 * don't start sending property if in the middle of
		 * sending one already.
		 */
		return -1;
	}
	dp->prop = prop;
	rc = prop_dp_step(dp);
	if (!rc) {
		/*
		 * don't return 0 so prop->send_mask doesn't get cleared
		 * automatically. we'll clear it ourselves when done.
		 */
		return 1;
	}
	return rc;
}

/*
 * The set handler for large data points.
 * This receives the location and starts the get state machine.
 * Ignore setting with same location, since that is presumably already
 * fetched or in progress.
 */
void prop_dp_set(struct prop *prop, void *arg, void *val, size_t len)
{
	struct prop_dp *dp = arg;

	dp->prop = prop;
	if (len > sizeof(dp->loc) - 1) {
		return;
	}
	if (len == dp->loc_len && !memcmp(dp->loc, val, len)) {
		return;
	}
	memcpy(dp->loc, val, len);
	dp->loc[len] = '\0';
	dp->loc_len = len;
	dp->state = DS_REQUEST;
	dp->next_off = 0;
	dp->tot_len = 0;
	dp->prop->send_mask = ADS_BIT;
}

int prop_dp_start_send(struct prop *prop, struct prop_dp *dp, u32 len)
{
	dp->prop = prop;
	if (dp->state == DS_IDLE) {
		dp->state = DS_CREATE;
		dp->tot_len = len;
		dp->next_off = 0;
		prop->send_mask = ADS_BIT;
	}

	return 0;
}
