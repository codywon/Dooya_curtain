/*
 * Copyright 2013 Ayla Networks, Inc.  All rights reserved.
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
#include <ayla/conf_token.h>

#ifdef AYLA_UART
#include <ayla/uart.h>
#else
#include <ayla/spi.h>
#endif

#include <ayla/props.h>
#include <ayla/serial_msg.h>
#include <ayla/internal/tlv_internal.h>
#include <ayla/byte_order.h>
#include <ayla/host_ota.h>

void (*host_ota_start_cb)(char *version, int ver_len, size_t len);
void (*host_ota_load_cb)(size_t off, void *buf, size_t len);
int (*host_ota_boot_cb)(char *version, int ver_len);

static u8 host_ota_cmd;	/* 1 if MCU OTA can go ahead */
static u8 host_ota_status; /* set to err + 1 to report status */

/*
 * Notification from module that there is an image available.
 */
void host_ota_start(void *buf, size_t len)
{
	struct ayla_tlv *ver, *sz;
	size_t ilen;

	ver = tlv_get(ATLV_UTF8, buf, len);
	sz = tlv_get(ATLV_LEN, buf, len);
	if (ver && sz) {
		if (sz->len == sizeof(u16)) {
			ilen = get_ua_be16(sz + 1);
		} else if (sz->len == sizeof(u32)) {
			ilen = get_ua_be32(sz + 1);
		} else {
			goto fail;
		}
		host_ota_start_cb((char *)(ver + 1), ver->len, ilen);
	} else {
		goto fail;
	}
	return;
fail:
	host_ota_stat(AERR_INVAL_TLV);
}

/*
 * Set the go ahead flag for mcu to tell module to start downloading image.
 */
void host_ota_go(void)
{
	host_ota_cmd = 1;
}

/*
 * Call the host_ota_cb with part of the image
 */
void host_ota_load(void *buf, size_t len)
{
	struct ayla_tlv *off_tlv, *data;
	size_t blen, off;

	off_tlv = tlv_get(ATLV_OFF, buf, len);
	data = tlv_get(ATLV_BIN, buf, len);

	if (off_tlv && data) {
		blen = data->len;
		off = get_ua_be32(off_tlv + 1);
		host_ota_load_cb(off, data + 1, blen);
	} else {
		/*
		 * report error
		 */
		host_ota_stat(AERR_INVAL_TLV);
	}
}

/*
 * Send MCU_OTA_STAT message
 */
void host_ota_stat(u8 err)
{
	host_ota_status = err + 1;
}

void host_ota_boot(void *buf, size_t len)
{
	struct ayla_tlv *ver;
	char *ver_str;
	int ver_len;

	ver = tlv_get(ATLV_UTF8, buf, len);
	if (ver) {
		ver_str = (char *)(ver + 1);
		ver_len = ver->len;
	} else {
		ver_str = NULL;
		ver_len = 0;
	}
	if (host_ota_boot_cb) {
		host_ota_boot_cb(ver_str, ver_len);
	}
}

/*
 * Send go-ahead message for MCU OTA.
 */
void host_ota_send(void)
{
	struct ayla_cmd *cmd;
	u8 tmp;

	if (!host_ota_cmd && !host_ota_status) {
		return;
	}

	if (host_ota_status) {
		tmp = sizeof(struct ayla_tlv) + sizeof(u8);
	} else {
		tmp = 0;
	}
	cmd = serial_tx_buf_get(sizeof(*cmd) + tmp, 0, NULL);
	if (!cmd) {
		return;
	}
	cmd->protocol = ASPI_PROTO_CMD;
	if (host_ota_cmd) {
		cmd->opcode = ACMD_MCU_OTA;
		host_ota_cmd = 0;
	} else {
		cmd->opcode = ACMD_MCU_OTA_STAT;
		tmp = host_ota_status - 1;
		tlv_put(cmd + 1, ATLV_ERR, &tmp, sizeof(tmp));
		host_ota_status = 0;
	}
	put_ua_be16((void *)&cmd->req_id, tlv_req_id);
	tlv_req_id++;

	serial_tx_buf_send(0, tlv_req_id - 1);
}
