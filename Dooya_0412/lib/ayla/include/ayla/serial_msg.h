/*
 * Copyright 2012-2013 Ayla Networks, Inc.  All rights reserved.
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
#ifndef __SERIAL_MSG_H__
#define __SERIAL_MSG_H__

/*
 * In serial_msg.c.
 */
void serial_rx_data_cmd(u8 *buf, size_t len);
int serial_tx_cmd(enum ayla_cmd_op, void *args, size_t arg_len);

/*
 * Implement data command opcode 13: Enable Service Listener
 */
int serial_tx_service_listen(void);

/*
 * Function provided by TLV driver to send property TLVs.
 */
int serial_send_prop(struct prop *, const void *val, size_t, void *arg);

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
			void (*callback)(void));

/*
 * Cancel the send request
 */
int serial_tx_cancel(void);

/*
 * Send the buffer to module
 */
int serial_tx_buf_send(int check_ads_busy, u16 req_id);

/*
 * Function provided by TLV driver to request prop value from cloud.
 * The request id of cmd is saved in req_id.
 */
int serial_request_prop(struct prop *prop, u16 *req_id);

/*
 * Poll for incoming messages.
 */
void serial_poll(void);

/*
 * Handle incoming messages from module.
 * Route them as conf, data, or ping messages.
 */
int serial_process_inc_pkt(u8 *data_ptr, size_t recv_len);

extern u16 tlv_req_id;	/* current transaction ID */
extern u8 feature_mask;	/* ayla features that the mcu supports */
extern u8 features_sent;

/*
 * Return non-zero if module is busy with ADS command.
 */
int serial_is_ads_busy(void);

#endif /* __SERIAL_MSG_H__ */
