/*
 * Copyright 2011-2012 Ayla Networks, Inc.  All rights reserved.
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

/*
 * Example TLV driver code.
 */
#include <string.h>

#include <ayla/mcu_platform.h>
#include <mcu_io.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/conf_token.h>
#include <ayla/conf_access.h>
#include <ayla/byte_order.h>
#include <ayla/serial_msg.h>

#ifdef AYLA_UART
#include <ayla/uart.h>
#else
#include <ayla/spi.h>
#include <ayla/spi_ping.h>
#include <cm3_intr.h>
#endif


#ifdef AYLA_FILE_PROP
#include <ayla/prop_dp.h>
#endif

u16 tlv_req_id = 0;	/* current transaction ID */
u8 feature_mask = 0;	/* ayla features that the mcu supports */
u8 features_sent;
static struct send_arg serial_send_arg;
static u8 serial_msg_pkt[TLV_MAX_STR_LEN + 1];
static u32 serial_msg_next_off;
static u32 serial_msg_tot_size;

#ifdef AYLA_UART
static u8 tx_err_op_cb;
static u32 tx_err_req_id_cb;
static u8 tx_err_cb;
static u8 tx_feat_mask_cb;
static int serial_tx_err(u8 op, u32 req_id, u8 err, u8 feat_mask);
#else
static int serial_pri;
#endif

#define SERIAL_STATS

#ifndef SERIAL_STATS
#define STATS(x)
#else
#define STATS(x)	do { (serial_msg_stats.x++); } while (0)

/*
 * Optional debug counters for various conditions.
 * These can be deleted to save space but may help debugging.
 */
struct serial_msg_stats {
	u16 rx_len_err;
	u16 rx_offset_err;
	u16 rx_op_err;
	u16 rx_tlv_len_err;
	u16 rx_no_name;

	u16 rx_wrong_type;
	u16 rx_read_only;
	u16 rx_unk_name;
	u16 rx_no_send;

	u16 rx_nak;
	u16 rx_range_err;

	u16 tx_len_err;
} serial_msg_stats;
#endif /* SERIAL_STATS */

/*
 * Handle received TLVs.
 */
static void serial_rx_tlv(struct ayla_cmd *cmd, void *buf, size_t len)
{
	struct prop *prop = NULL;
	struct ayla_tlv *tlv = (struct ayla_tlv *)buf;
	void *valp;
	size_t rlen = len;
	size_t tlen;
	u8 dests = 0;
	u32 req_id;
	u32 offset = 0;
	u8 offset_given = 0;
	u8 eof = 0;
	u8 reset_offset = 1;

	prop = NULL;
	while (rlen > 0) {
		if (rlen < sizeof(*tlv)) {
			STATS(rx_len_err);
			break;
		}

		tlen = tlv->len;
		if (tlen + sizeof(*tlv) > rlen) {
			STATS(rx_tlv_len_err);
			break;
		}

		switch (tlv->type) {
		case ATLV_NAME:
			prop = prop_lookup_len((char *)(tlv + 1), tlen);
			if (!prop) {
				STATS(rx_unk_name);
				break;
			}
			if (cmd->opcode == AD_ECHO_FAIL) {
				goto setup_echo;
			}
			break;

		case ATLV_INT:
			if (!prop) {
				STATS(rx_unk_name);
				break;
			}
			if (prop->type == ATLV_UINT) {
				if ((s32)get_ua_be32(tlv + 1) < 0) {
					STATS(rx_range_err);
					break;
				}
				tlv->type = ATLV_UINT;
			}
			/* fall-through */
		case ATLV_CENTS:
		case ATLV_BIN:
		case ATLV_UTF8:
		case ATLV_FLOAT:
		case ATLV_BOOL:
		case ATLV_LOC:
		case ATLV_SCHED:
			if (!prop) {
				STATS(rx_no_name);
				break;
			} else if (tlv->type != prop->type) {
				STATS(rx_wrong_type);
				break;
			} else if (!prop->set) {
				STATS(rx_read_only);
				break;
			} else if (serial_msg_next_off != offset ||
			    serial_msg_next_off > serial_msg_tot_size) {
				STATS(rx_len_err);
				break;
			}
			valp = (void *)(tlv + 1);
			if (tlv->type == ATLV_UTF8) {
				memcpy(serial_msg_pkt +
				    serial_msg_next_off, valp, tlen);
				valp = &serial_msg_pkt;
			} else {
				prop_swap(prop, valp);
			}
			serial_msg_next_off += tlen;
			if (serial_msg_next_off == serial_msg_tot_size ||
			    eof || (!offset_given && !serial_msg_tot_size)) {
				prop->set(prop, prop->arg, valp,
				    serial_msg_next_off);
				if (!(valid_dest_mask & ADS_BIT)) {
setup_echo:
					prop->echo = 1;
					prop->send_mask = ADS_BIT;
				}
			} else {
				reset_offset = 0;
			}
			prop = NULL;
			break;
		case ATLV_NODES:
			dests = *(u8 *)(tlv + 1);
			if (cmd->opcode == AD_CONNECT) {
				prop_update_connectivity(dests);
			}
			break;
		case ATLV_LEN:
			if (serial_msg_tot_size) {
				STATS(rx_len_err);
				break;
			}
			if (get_ua_with_len(tlv + 1, tlv->len,
			    &serial_msg_tot_size)) {
				STATS(rx_len_err);
				break;
			}
			if (serial_msg_tot_size >
			    sizeof(serial_msg_pkt) - 1) {
				STATS(rx_len_err);
			}
			break;
		case ATLV_OFF:
			if (get_ua_with_len(tlv + 1, tlv->len, &offset)) {
				STATS(rx_offset_err);
				break;
			}
			offset_given = 1;
			break;
		case ATLV_EOF:
			eof = 1;
			break;
		case ATLV_FORMAT:
		default:
			break;
		}
		tlv = (void *)((char *)(tlv + 1) + tlen);
		rlen -= sizeof(*tlv) + tlen;
	}
	req_id = ntohs(cmd->req_id);
#ifdef AYLA_UART
	if (cmd->opcode == AD_NAK || cmd->opcode == AD_CONFIRM) {
		uart_confirm_or_nak_recvd(req_id);
	}
#endif
	if (cmd->opcode == AD_NAK) {
		prop_notify_failure(req_id, prop, dests);
	}
	if (reset_offset) {
		serial_msg_next_off = 0;
		serial_msg_tot_size = 0;
	}
}

/*
 * Return a buffer for transmission if one is available. Return null if
 * not available. If check_ads_busy flag is given, the function will
 * only return a buffer if module has received confirmation from module
 * that its finished sending out the previous packet to the service.
 */
#ifdef AYLA_UART
/*
 * The caller can set a callback function to be called  by the driver
 * when its available if the driver's transmission buffer is currently
 * not available. This is useful for functions that don't get "polled".
 * For example, its useful for serial_tx_err since it isn't retried if
 * serial_tx_buf_get fails.
 */
#else
/*
 * The use for callback is currently TBD. It should always be set to NULL
 * for now.
 */
#endif
void *serial_tx_buf_get(size_t tot_len, int check_ads_busy,
		    void (*callback)(void))
{
	void *buf;

#ifdef AYLA_UART
	if (check_ads_busy) {
		buf = uart_tx_buf_get_for_props(tot_len, callback);
	} else {
		buf = uart_tx_buf_get(tot_len, callback);
	}
#else
	serial_pri = intr_disable_save();
	if (check_ads_busy) {
		buf = spi_tx_buf_get_for_props(tot_len);
	} else {
		buf = spi_tx_buf_get(tot_len);
	}
	if (buf == NULL) {
		intr_restore(serial_pri);
	}
#endif
	return buf;
}

/*
 * Cancel the send request
 */
int serial_tx_cancel(void)
{
#ifndef AYLA_UART
	intr_restore(serial_pri);
#endif
	return 0;
}

/*
 * Send the buffer to module
 */
int serial_tx_buf_send(int check_ads_busy, u16 req_id)
{
#ifdef AYLA_UART
	return uart_tx_buf_send(check_ads_busy, req_id);
#else
	spi_tx_wait();
	intr_restore(serial_pri);
	return 0;
#endif
}

#ifdef AYLA_UART
/*
 * Callback to try to resend the error.
 */
static void serial_tx_err_cb(void)
{
	serial_tx_err(tx_err_op_cb, tx_err_req_id_cb, tx_err_cb,
	    tx_feat_mask_cb);
}
#endif

/*
 * Send error response.
 */
static int serial_tx_err(u8 op, u32 req_id, u8 err, u8 feat_mask)
{
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	int tot_len;
	int rc;

	tot_len = sizeof(*cmd) + sizeof(*tlv) + sizeof(u8);
	if (feat_mask) {
		tot_len += sizeof(*tlv) + sizeof(u8);
	}
#ifdef AYLA_UART
	tx_err_op_cb = op;
	tx_err_req_id_cb = req_id;
	tx_err_cb = err;
	tx_feat_mask_cb = feat_mask;
	cmd = serial_tx_buf_get(tot_len, 0, &serial_tx_err_cb);
#else
	cmd = serial_tx_buf_get(tot_len, 0, NULL);
#endif
	if (!cmd) {
		return -1;
	}

	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = (op == AD_SEND_PROP_RESP) ? op : AD_NAK;
	put_ua_be16((void *)&cmd->req_id, req_id);

	tlv = (struct ayla_tlv *)(cmd + 1);
	tlv->type = ATLV_ERR;
	tlv->len = sizeof(u8);
	*(u8 *)(tlv + 1) = err;

	if (feat_mask) {
		tlv = (void *)((char *)(tlv + 1) + sizeof(u8));
		tlv->type = ATLV_FEATURES;
		tlv->len = sizeof(feature_mask);
		*(u8 *)(tlv + 1) = feature_mask;
	}
	rc = serial_tx_buf_send(0, req_id);
	if (!rc) {
		features_sent = 1;
	}
	return rc;
}

/*
 * Handle request to send a TLVs.
 */
static void serial_rx_send_tlv(struct ayla_cmd *cmd, void *buf, size_t len)
{
	struct prop *prop;
	struct ayla_tlv *tlv = (struct ayla_tlv *)buf;
	struct send_arg send_arg;
	size_t tlen;
	u8 err;

	memset(&send_arg, 0, sizeof(send_arg));
	send_arg.resp = 1;
	send_arg.req_id = ntohs(cmd->req_id);
	if (len == 0) {
		/* AD_SEND_PROP cmd without a name TLV */
		err = AERR_INVAL_NAME;
		serial_tx_err(AD_SEND_PROP_RESP, send_arg.req_id, err, 1);
		prop_send_req_to_ads_only("version");
		prop_request_value(NULL);
		return;
	} else if (len < sizeof(*tlv)) {
		STATS(rx_len_err);
		err = AERR_LEN_ERR;
		goto reject;
	}

	tlen = tlv->len;
	if (tlen + sizeof(*tlv) > len) {
		STATS(rx_tlv_len_err);
		err = AERR_LEN_ERR;
		goto reject;
	}
	if (tlv->type != ATLV_NAME) {
		STATS(rx_no_name);
		err = AERR_INVAL_TLV;
		goto reject;
	}
	if (prop_is_busy()) {
		/* don't reply to prop requests if prop state machine is busy */
		return;
	}
	prop = prop_lookup_len((char *)(tlv + 1), tlen);
	if (!prop || !prop->send) {
		STATS(rx_unk_name);
		err = AERR_UNK_VAR;
		goto reject;
	}
	prop->send(prop, &send_arg);
	return;
reject:
	serial_tx_err(AD_SEND_PROP_RESP, send_arg.req_id, err, 0);
}

/*
 * Handle request to send the next TLV after a specified token.
 */
static void serial_rx_send_next_tlv(struct ayla_cmd *cmd, void *buf, size_t len)
{
	struct prop *prop;
	struct ayla_tlv *tlv = (struct ayla_tlv *)buf;
	struct send_arg send_arg;
	u32 index;
	u8 err;

	memset(&send_arg, 0, sizeof(send_arg));
	send_arg.resp = 1;
	send_arg.req_id = ntohs(cmd->req_id);
	if (len == 0) {
		index = 0;
	} else {
		if (len < sizeof(*tlv)) {
			STATS(rx_len_err);
			err = AERR_LEN_ERR;
			goto reject;
		}
		if (tlv->type != ATLV_CONT) {
			STATS(rx_no_name);
			err = AERR_INVAL_TLV;
			goto reject;
		}
		if (len < tlv->len + sizeof(*tlv) || tlv->len != sizeof(u32)) {
			STATS(rx_tlv_len_err);
			err = AERR_LEN_ERR;
			goto reject;
		}
		index = get_ua_be32((be32 *)(tlv + 1));
		if (index >= prop_count) {
			STATS(rx_unk_name);
			err = AERR_UNK_VAR;
			goto reject;
		}
	}
	if (prop_is_busy()) {
		/* don't reply to prop requests if prop state machine is busy */
		return;
	}
	prop = &prop_table[index];
	for (index++; index < prop_count; index++) {
#ifdef AYLA_FILE_PROP
		if (prop_table[index].send == prop_dp_send) {
			continue;
		}
#endif /* AYLA_FILE_PROP */
		if (prop_table[index].send) {
			break;
		}
	}
	send_arg.cont = index < prop_count ? index : 0;
	if (!prop->send) {
		STATS(rx_no_send);
		err = AERR_UNK_VAR;
		goto reject;
	}
	prop->send(prop, &send_arg);
	return;
reject:
	serial_tx_err(AD_SEND_PROP_RESP, send_arg.req_id, err, 0);
}

/*
 * Handle received data command.
 * The command may be a request for properties or a new value for a property.
 * It can also be connectivity information or a NAK.
 */
void serial_rx_data_cmd(u8 *buf, size_t len)
{
	struct ayla_cmd *cmd = (void *)buf;

	if (len < sizeof(*cmd)) {
		STATS(rx_len_err);
		return;
	}
	buf += sizeof(*cmd);
	len -= sizeof(*cmd);

	switch (cmd->opcode) {
	case AD_NAK:
		STATS(rx_nak);
		prop_nak(cmd, buf, len);
		serial_rx_tlv(cmd, buf, len);
		break;
	case AD_RECV_TLV:
	case AD_CONNECT:
	case AD_ECHO_FAIL:
	case AD_CONFIRM:
		serial_rx_tlv(cmd, buf, len);
		break;
	case AD_SEND_PROP:
		serial_rx_send_tlv(cmd, buf, len);
		break;
	case AD_SEND_NEXT_PROP:
		serial_rx_send_next_tlv(cmd, buf, len);
		break;
#ifdef AYLA_FILE_PROP
	case AD_DP_RESP:
		prop_dp_resp(cmd, buf, len);
		break;
#endif /* AYLA_FILE_PROP */
	case AD_ERROR:
		prop_request_value(NULL);
		break;
	case AD_PROP_NOTIFY:
		/*
		 * There is a pending update either from the cloud or from a
		 * LAN client. If the host is in the middle of a FILE operation,
		 * it can choose to abort the operation so it can receive the
		 * update. Note that if the host aborts a file upload, the
		 * entire upload will fail and will need to be started from the
		 * beginning. Also, it is the host's responsibility to restart
		 * the file op when its ready. The default behavior for the demo
		 * is to just ignore this opcode and let the file operation
		 * finish before getting the property update.
		 */
		/* prop_dp_abort(); */
		break;
	default:
		STATS(rx_op_err);
		serial_tx_err(cmd->opcode, cmd->req_id, AERR_INVAL_OP, 0);
		break;
	}
}

int send_name_tlv(const char *var_name,
			 enum ayla_tlv_type type, size_t val_len,
			 void *val, u8 fmt_flags)
{
	struct ayla_cmd *cmd;
	struct ayla_tlv *name;
	struct ayla_tlv *val_tlv;
	struct ayla_tlv_fmt *ftlv;
	u16 name_len;
	int tot_len;

	name_len = strlen(var_name);
	tot_len = sizeof(*cmd) + sizeof(*name) + name_len +
	    sizeof(*val_tlv) + val_len;
	if (fmt_flags) {
		tot_len += sizeof(*ftlv);
	}
	if (tot_len > ASPI_LEN_MAX) {
		STATS(tx_len_err);
		return 0;
	}
	cmd = serial_tx_buf_get(tot_len, 1, NULL);
	if (!cmd) {
		return -1;
	}

	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_SEND_TLV;
	tlv_req_id++;
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);

	name = (struct ayla_tlv *)(cmd + 1);
	name->type = ATLV_NAME;
	name->len = name_len;
	memcpy(name + 1, var_name, name_len);

	val_tlv = (struct ayla_tlv *)((char *)(name + 1) + name_len);

	if (fmt_flags) {
		ftlv = (struct ayla_tlv_fmt *)val_tlv;
		ftlv->head.type = ATLV_FORMAT;
		ftlv->head.len = sizeof(ftlv->fmt_flags);
		ftlv->fmt_flags = fmt_flags;
		val_tlv = (struct ayla_tlv *)(ftlv + 1);
	}

	val_tlv->type = type;
	val_tlv->len = val_len;
	memcpy(val_tlv + 1, val, val_len);
	return serial_tx_buf_send(1, tlv_req_id);
}

/*
 * Send TLV string.
 */
int send_tlv_string(char *name, char *val, u8 fmt_flags)
{
	return send_name_tlv(name, ATLV_UTF8, strlen(val), val, fmt_flags);
}

/*
 * Send property if possible.
 * For echoes, also send out an echo and a nodes TLV.
 * If a nodes tlv isn't sent, the module sends the update to all valid
 * destinations. For echoes, we only need to update the destinations
 * that don't have the update.
 */
int serial_send_prop(struct prop *prop, const void *val, size_t val_len,
		    void *arg)
{
	struct send_arg *send_arg = (struct send_arg *)arg;
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	struct ayla_tlv_fmt *ftlv;
	void *valp;
	int tot_len;
	u16 req_id;
	u16 val16;
	u32 val32;
	u8 send_partials = 0;
	size_t curr_val_len;
	u32 offset;
	u8 eof = 0;
	int rc = 0;

	if (send_arg) {
		req_id = send_arg->req_id;
		offset = send_arg->offset;
	} else {
		offset = 0;
		tlv_req_id++;
		req_id = tlv_req_id;
	}
	/*
	 * Unsigned integers will be sent as ATLV_INT below, so
	 * if value would turn on the sign bit, increase to next larger size.
	 * Put integers in network byte order.
	 */
	switch (prop->type) {
	case ATLV_UINT:
		switch (val_len) {
		case sizeof(u8):
			if (*(u8 *)val > MAX_S8) {
				val16 = *(u8 *)val;
				val = &val16;
				val_len = sizeof(val16);
			}
			break;
		case sizeof(u16):
			if (*(u16 *)val > MAX_S16) {
				val32 = *(u16 *)val;
				val = &val32;
				val_len = sizeof(val32);
			}
			break;
		default:
			break;
		}
		/* fall through */
	case ATLV_INT:
	case ATLV_CENTS:
		switch (val_len) {
		case sizeof(s16):
			val16 = htons(*(s16 *)val);
			val = &val16;
			break;
		case sizeof(u32):
			val32 = get_ua_be32((be32 *)val);
			val = &val32;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	if (val_len > TLV_MAX_LEN) {
		if (prop->type != ATLV_UTF8 && prop->type != ATLV_BIN) {
			/* no support for > 255 size for non-strs and */
			/* non-binaries */
			return 0;
		}
		send_partials = 1;
		if (send_arg) {
			val_len -= send_arg->offset;
			val = (void *)((char *)val + send_arg->offset);
		}
	}
	while (!eof) {
		eof = val_len <= TLV_MAX_LEN;
		curr_val_len = eof ? val_len : TLV_MAX_LEN;
		tot_len = sizeof(*cmd) + sizeof(*tlv) + prop->name_len +
		    sizeof(*tlv) + curr_val_len;
		if (offset) {
			tot_len += sizeof(*tlv) + sizeof(offset);
		}
		if (eof && send_arg && send_arg->cont) {
			/* send the continuation in the final pkt only */
			tot_len += sizeof(*tlv) + sizeof(u32);
		}
		if (!send_arg && prop->echo) {
			tot_len += 2 * sizeof(*tlv) +
			    sizeof(prop->send_mask);
		}
		if (prop->fmt_flags) {
			tot_len += sizeof(*ftlv);
		}
		if (send_partials && !offset) {
			/* send total length in the first partial */
			tot_len += sizeof(*tlv) + sizeof(u32);
		}
		if (tot_len > ASPI_LEN_MAX) {
			STATS(tx_len_err);
			return 0;
		}
		cmd = serial_tx_buf_get(tot_len, 1, NULL);
		if (!cmd) {
setup_retry:
			if (send_arg) {
				/* retry sending later */
				memcpy(&serial_send_arg, send_arg,
				    sizeof(serial_send_arg));
				serial_send_arg.offset = offset;
				prop_setup_retry(prop, &serial_send_arg,
				    send_arg->resp);
			} else if (offset) {
				/* retry sending later */
				memset(&serial_send_arg, 0,
				    sizeof(serial_send_arg));
				serial_send_arg.offset = offset;
				serial_send_arg.req_id = req_id;
				prop_setup_retry(prop, &serial_send_arg, 0);
			}
			return -1;
		}

		cmd->protocol = ASPI_PROTO_DATA;
		prop->curr_op = POST;
		if (send_arg && send_arg->resp) {
			cmd->opcode = AD_SEND_PROP_RESP;
		} else {
			cmd->opcode = AD_SEND_TLV;
		}
		put_ua_be16((void *)&cmd->req_id, req_id);
		prop->req_id = req_id;

		tlv = (struct ayla_tlv *)(cmd + 1);
		if (eof && send_arg && send_arg->cont) {
			tlv->type = ATLV_CONT;
			tlv->len = sizeof(u32);
			put_ua_be32((be32 *)(tlv + 1), send_arg->cont);
			tlv = (struct ayla_tlv *)((char *)(tlv + 1) + sizeof(u32));
		}
		tlv->type = ATLV_NAME;
		tlv->len = prop->name_len;
		memcpy(tlv + 1, prop->name, prop->name_len);
		tlv = (struct ayla_tlv *)((char *)(tlv + 1) + prop->name_len);
		if (send_partials && !offset) {
			tlv->type = ATLV_LEN;
			tlv->len = sizeof(u32);
			put_ua_be32((be32 *)(tlv + 1), val_len);
			tlv = (struct ayla_tlv *)((char *)(tlv + 1) + sizeof(u32));
		}
		if (offset) {
			tlv->type = ATLV_OFF;
			tlv->len = sizeof(offset);
			put_ua_be32((be32 *)(tlv + 1), offset);
			tlv = (struct ayla_tlv *)((char *)(tlv + 1) + sizeof(offset));
		}
		if (prop->fmt_flags) {
			ftlv = (struct ayla_tlv_fmt *)tlv;
			ftlv->head.type = ATLV_FORMAT;
			ftlv->head.len = sizeof(ftlv->fmt_flags);
			ftlv->fmt_flags = prop->fmt_flags;
			tlv = (struct ayla_tlv *)(ftlv + 1);
		}
		tlv->type = prop->type;
		if (prop->type == ATLV_UINT) {
			tlv->type = ATLV_INT;
		}
		tlv->len = curr_val_len;
		valp = tlv + 1;
		memcpy(valp, (char *)val + offset, curr_val_len);
		tlv = (struct ayla_tlv *)((char *)(tlv + 1) + curr_val_len);
		if (!send_arg && prop->echo) {
			prop->curr_op = ECHO;
			tlv->type = ATLV_ECHO;
			tlv->len = 0;

			tlv = (struct ayla_tlv *)(tlv + 1);
			tlv->type = ATLV_NODES;
			tlv->len = sizeof(prop->send_mask);
			*(u8 *)(tlv + 1) = prop->send_mask;
			tlv = (struct ayla_tlv *)((u8 *)(tlv + 1) +
			    sizeof(u8));
		}
		rc = serial_tx_buf_send(1, req_id);
		if (rc) {
			goto setup_retry;
		}
		val_len -= curr_val_len;
		offset += curr_val_len;
	}

	return rc;
}

/*
 * Request property from cloud if possible. If prop == NULL, all to-device props
 * will be received.
 */
int serial_request_prop(struct prop *prop, u16 *req_id)
{
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	int tot_len;

	tot_len = sizeof(*cmd);
	if (prop) {
		tot_len += sizeof(*tlv) + prop->name_len;
	}
	if (tot_len > ASPI_LEN_MAX) {
		STATS(tx_len_err);
		return 0;
	}
	cmd = serial_tx_buf_get(tot_len, 1, NULL);
	if (!cmd) {
		return -1;
	}

	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_REQ_TLV;
	tlv_req_id++;
	*req_id = tlv_req_id;
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);

	if (prop) {
		prop->curr_op = GET;
		prop->req_id = tlv_req_id;
		tlv = (struct ayla_tlv *)(cmd + 1);
		tlv->type = ATLV_NAME;
		tlv->len = prop->name_len;
		memcpy(tlv + 1, prop->name, prop->name_len);
	}

	return serial_tx_buf_send(1, tlv_req_id);
}
/*
 * Send control command.
 */
int serial_tx_cmd(enum ayla_cmd_op op, void *args, size_t arg_len)
{
	struct ayla_cmd *cmd;
	size_t tot_len;

	tot_len = sizeof(*cmd) + arg_len;
	if (tot_len > ASPI_LEN_MAX) {
		STATS(tx_len_err);
		return 0;
	}

	cmd = serial_tx_buf_get(tot_len, 0, NULL);
	if (!cmd) {
		return -1;
	}
	cmd->protocol = ASPI_PROTO_CMD;
	cmd->opcode = op;
	tlv_req_id++;
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);

	if (arg_len) {
		memcpy(cmd + 1, args, arg_len);
	}
	return serial_tx_buf_send(0, tlv_req_id);
}

/*
 * Implement data command opcode 13: Enable Service Listener
 */
int serial_tx_service_listen(void)
{
	struct ayla_cmd *cmd;
	size_t tot_len;

	tot_len = sizeof(*cmd);
	if (tot_len > ASPI_LEN_MAX) {
		STATS(tx_len_err);
		return 0;
	}

	cmd = serial_tx_buf_get(tot_len, 0, NULL);
	if (!cmd) {
		return -1;
	}
	cmd->protocol = ASPI_PROTO_DATA;
	cmd->opcode = AD_LISTEN_ENB;
	tlv_req_id++;
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);

	return serial_tx_buf_send(0, tlv_req_id);
}

/*
 * Return non-zero if module is busy with ADS command.
 */
int serial_is_ads_busy(void)
{
#ifdef AYLA_UART
	return uart_is_tx_busy();
#else
	return spi_is_ads_busy();
#endif
}

/*
 * Poll for incoming messages.
 */
void serial_poll(void)
{
#ifdef AYLA_UART
	uart_platform_poll();
#else
	spi_poll();
#endif
}

/*
 * Handle incoming messages from module.
 * Route them as conf, data, or ping messages.
 */
int serial_process_inc_pkt(u8 *data_ptr, size_t recv_len)
{
	switch (data_ptr[0]) {
#ifdef AYLA_CONF
	case ASPI_PROTO_CMD:
		conf_rx(data_ptr, recv_len);
		break;
#endif
	case ASPI_PROTO_DATA:
		serial_rx_data_cmd(data_ptr, recv_len);
		break;
	default:
		return -1;
	}

	return 0;
}
