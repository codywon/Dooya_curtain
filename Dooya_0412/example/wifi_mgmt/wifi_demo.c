/*
 * Copyright 2011-2013 Ayla Networks, Inc.  All rights reserved.
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
 * Sample code that walks through scan list of module, trying the same
 * password with different APs.
 * If there is a failure, it moves to next slot. If connection succeeds,
 * this stops.
 * Note that this is purely as a demonstration of module's wifi configuration
 * API.
 */

#include <string.h>

#include <ayla/mcu_platform.h>

#ifdef DEMO_CONF
#include <ayla/conf_token.h>
#include <ayla/conf_access.h>
#endif

#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/serial_msg.h>
#include <ayla/byte_order.h>

#ifdef AYLA_WIFI_DEMO
struct var_descr {
	int nlen;
	enum conf_token name[5];
	int type;
	int len;
	char val[64];
};

static struct var_descr scan_ready[] = {
	{ 3, { CT_wifi, CT_scan, CT_ready } }
};

static struct var_descr hist_info[] = {
	{ 5, { CT_wifi, CT_hist, CT_n, (enum conf_token)0, CT_error } },
	{ 5, { CT_wifi, CT_hist, CT_n, (enum conf_token)0, CT_time } },
	{ 5, { CT_wifi, CT_hist, CT_n, (enum conf_token)0, CT_ssid } },
};

static struct var_descr scan_results[] = {
	{ 3, { CT_wifi, CT_scan, CT_n } },
	{ 5, { CT_wifi, CT_scan, CT_n, (enum conf_token)0, CT_ssid } },
	{ 5, { CT_wifi, CT_scan, CT_n, (enum conf_token)0, CT_security } },
	{ 5, { CT_wifi, CT_scan, CT_n, (enum conf_token)0, CT_rssi } },
	{ 5, { CT_wifi, CT_scan, CT_n, (enum conf_token)0, CT_bars } },
};

static struct var_descr *cur_tbl;
static int cur_idx;
static int cur_cnt;
static int read_done;
static int reread_tbl;
static int prev_attempt_time;
static int scan_idx;
char *key = "test_password";

extern void stm32_delay_time(u32 ms);
static int conf_read_next_tbl(void);
static void conf_rsp_cb(void *buf, size_t len);

/*
 * Given a table of variables, fetch their contents from module.
 */
static int conf_read_next_tbl(void)
{
	/*
	 * Reads the next variable in the table.
	 */
	if (cur_idx >= cur_cnt) {
		read_done = 1;
		return 0;
	}
	reread_tbl = 0;
	if (conf_read(cur_tbl[cur_idx].name, cur_tbl[cur_idx].nlen,
		conf_rsp_cb)) {
		reread_tbl = 1;
		return -1;
	}
	return 0;
}

static void copy_val(void *data, int type, int len)
{
	cur_tbl[cur_idx].len = len;
	cur_tbl[cur_idx].type = type;
	if (len > 0) {
		if (len > sizeof(cur_tbl[cur_idx].val)) {
			len = sizeof(cur_tbl[cur_idx].val);
		}
		memcpy(cur_tbl[cur_idx].val, data, len);
	}
}

/*
 * Callback with config info. Assume that only one TLV comes in, carrying
 * the response to earlier conf read request. Copy in all the data to
 * current table being read. You might also get an error TLV as response
 * in a NAK.
 */
static void conf_rsp_cb(void *buf, size_t rlen)
{
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	size_t tlen;

	cmd = buf;
	rlen -= sizeof(*cmd);
	tlv = (struct ayla_tlv *)(cmd + 1);

	while (rlen > sizeof(*tlv)) {
		tlen = sizeof(*tlv) + tlv->len;
		if (tlen > rlen) {
			copy_val(NULL, tlv->type, -1);
			break;
		}
		if (tlv->type != ATLV_CONF) {
			copy_val(tlv + 1, tlv->type, tlv->len);
			cur_idx++;
			conf_read_next_tbl();
			return;
		}
		rlen -= tlen;
		tlv = (struct ayla_tlv *)((char *)tlv + tlen);
	}
	copy_val(NULL, ATLV_ERR, 0);
	cur_idx++;
	conf_read_next_tbl();
}

/*
 * Function to start filling the table of variable values.
 * Uses cur_idx, cur_tbl, cur_cnt globals to keep track of the process.
 * And read_done to signal that the process is done.
 */
static int conf_fetch_tbl(struct var_descr *tbl, int items)
{
	read_done = 0;
	cur_idx = 0;
	cur_tbl = tbl;
	cur_cnt = items;

	return conf_read_next_tbl();
}

/*
 * Fetch wifi connection history for the latest connection attempt.
 */
static int wifi_fetch_history(void)
{
	return conf_fetch_tbl(hist_info,
	    sizeof(hist_info) / sizeof(struct var_descr));
}

static int wifi_check_hist_info(void)
{
	int err;
	int time_stamp;

	switch (hist_info[1].len) {
	case 1:
		time_stamp = hist_info[1].val[0];
		break;
	case 2:
		time_stamp = get_ua_be16(hist_info[1].val);
		break;
	case 4:
		time_stamp = get_ua_be32(hist_info[1].val);
		break;
	}

	err = hist_info[0].val[0];
	if (err == 20 || prev_attempt_time == time_stamp) {
		/*
		 * Not finished yet. Or started yet.
		 */
		return -1;
	} else {
		prev_attempt_time = time_stamp;
		if (err) {
			return err;
		}
	}
	return 0;
}

/*
 * Fetches scan info results from index 'idx'.
 */
static int wifi_fetch_scan_result(int idx)
{
	int i;

	for (i = 0; i < sizeof(scan_results) / sizeof(scan_results[0]); i++) {
		scan_results[i].name[3] = (enum conf_token)idx;
	}
	memset(scan_results[1].val, 0, sizeof(scan_results[1].val));
	return conf_fetch_tbl(scan_results,
	    sizeof(scan_results) / sizeof(scan_results[0]));
}

static int wifi_check_scan_result(void)
{
	if (scan_results[0].type == ATLV_ERR) {
		/*
		 * retry fetch.
		 */
		return -1;
	}
	if (scan_results[1].val[0] == '\0') {
		/*
		 * No results for this idx. Try another one.
		 */
		return 1;
	}
	return 0;
}

/*
 * Initiate wifi scan.
 */
static void wifi_start_scan(void)
{
	const enum conf_token wifi_scan_start[] = {
		CT_wifi, CT_scan, CT_start
	};
	u8 val = 1;

	conf_write(1, wifi_scan_start, 3, ATLV_INT, &val, sizeof(val));
}

static int wifi_fetch_scan_ready(void)
{
	return conf_fetch_tbl(scan_ready,
	    sizeof(scan_ready) / sizeof(scan_ready[0]));
}

static int wifi_check_scan_done(void)
{
	if (scan_ready[0].val[0]) {
		return 0;
	}
	return -1;
}

/*
 * Demo wifi poller.
 *
 * scan - We issued wifi scan command to module, and are waiting for it
 *        to complete.
 * scan_info - Scan was done, fetch network info from module from scan info
 *             array slot 'scan_idx'. Waiting for results to come in, and
 *             then we try joining this network using our preconfigured
 *             password.
 * check_hist - Waiting for results for join attempt. If join fails, we
 *             fetch scan results from next slot.
 */
void wifi_demo_poll(int start)
{
	static enum my_state { NONE, SCAN, SCAN_INFO, CHECK_HIST } state = NONE;
	int rc;

	if (start) {
		state = SCAN;
		wifi_start_scan();
		stm32_delay_time(10);
		wifi_fetch_scan_ready();
	}
	if (reread_tbl) {
		conf_read_next_tbl();
		return;
	}
	if (read_done) {
		read_done = 0;
		switch (state) {
		case NONE:
			break;
		case SCAN:
			rc = wifi_check_scan_done();
			if (rc == 0) {
				state = SCAN_INFO;
				wifi_fetch_scan_result(scan_idx);
			} else {
				stm32_delay_time(100);
				wifi_fetch_scan_ready();
			}
			break;
		case SCAN_INFO:
			rc = wifi_check_scan_result();
			if (rc == 0) {
				conf_wifi_join(scan_results[1].val,
				    scan_results[1].len, key,
				    strlen(key),
				    scan_results[2].val[0]);
				state = CHECK_HIST;
				stm32_delay_time(100);
				wifi_fetch_history();
			} else if (rc == -2) {
				if (++scan_idx >= 19) {
					scan_idx = 0;
				}
				stm32_delay_time(100);
				wifi_fetch_scan_result(scan_idx);
			} else if (rc == -1) {
				stm32_delay_time(100);
				wifi_fetch_scan_result(scan_idx);
			}
			break;
		case CHECK_HIST:
			rc = wifi_check_hist_info();
			if (rc == 0) {
				state = NONE;
			} else if (rc > 0) {
				if (++scan_idx >= 19) {
					scan_idx = 0;
				}
				wifi_fetch_scan_result(scan_idx);
				state = SCAN_INFO;
			} else if (rc < 0) {
				stm32_delay_time(100);
				wifi_fetch_history();
			}
			break;
		}
	}
}
#endif
