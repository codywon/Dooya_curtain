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
#include <string.h>
#include <ayla/mcu_platform.h>
#include <mcu_io.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/serial_msg.h>
#include <ayla/uart.h>
#include <ayla/byte_order.h>
#include <ayla/crc.h>

/* ppp frame size = ptype + seq # + 2-byte crc */
#define UART_PPP_SIZE	4

/* ack message size. max of 4 ack resps pending at a time */
#define UART_ACK_SIZE	(UART_PPP_SIZE * 4 + 1)

/* max receiving buffer length. recv interrupts will fill this */
#define UART_RX_SIZE	(ASPI_LEN_MAX *  2 + UART_PPP_SIZE + 1)

/* max tx buffer length */
#define UART_TX_SIZE	(ASPI_LEN_MAX + UART_PPP_SIZE + 1)

/* time to wait for an ack response for a packet before retransmitting (ms) */
#define UART_ACK_WAIT	5000

/* max # of retransmissions */
#define UART_MAX_RETRIES	2

/***** PPP Protocol *****/
#define	UART_PPP_FLAG_BYTE		0x7E
#define	UART_PPP_ESCAPE_BYTE		0x7D
#define	UART_PPP_XOR_BYTE		0x20
/************************/

#define UART_STATS

#ifndef UART_STATS
#define STATS(x)
#else
#define STATS(x)	do { (uart_stats.x++); } while (0)

/*
 * Optional debug counters for various conditions.
 * These can be deleted to save space but may help debugging.
 */
struct uart_stats {
	u16 tx_cbs;
	u16 tx_acks;
	u16 tx_data;
	u16 tx_reacks;

	u16 rx_acks;

	u16 tx_ovrflw_err;

	u16 rx_frame_err;
	u16 rx_len_err;
	u16 rx_crc_err;
	u16 rx_ptype_err;
	u16 rx_seq_err;
	u16 rx_cmd_err;
	u16 rx_unexp_nak_confirm_err;
} uart_stats;
#endif

enum uart_ptype {
	MP_NONE = 0,
	MP_DATA,
	MP_ACK
} PACKED;

struct uart_circular_buffer_def {
	u8 *buf;
	int start;
	int end;
	int size;
};

struct uart_state_def {
	struct uart_circular_buffer_def tx_buf;
	struct uart_circular_buffer_def ack_buf;
	struct uart_circular_buffer_def *cur_tx;
	int rx_len;
	int tx_len;
	u32 tick_ct_for_retry;
	void (*tx_callback)(void);	/* cb pending once tx buf is avail */
	u16 confirm_req_id;		/* req id that needs confirmation */
	u8 rx_buffer[UART_RX_SIZE];
	u8 recved_seq_no;
	u8 tx_seq_no;			/* seq # of the last trans packet */
	u8 num_retries;			/* # of times a pkt has been retried */
	u8 ack_msg_cur_tx;		/* len remaining of current tx ack */
	u8 escaped_byte;		/* byte that was escaped */
	unsigned int inited:1;
	unsigned int saw_ppp_flag:1;
	unsigned int first_tx:1;
	unsigned int wait_for_ack:1;
	unsigned int wait_for_confirm:1;
	unsigned int confirm_needed:1;
	unsigned int escape_sent:1;
	unsigned int head_flag_sent:1;
	unsigned int tail_flag_sent:1;
};

static u8 uart_tx_buffer[UART_TX_SIZE];
static u8 uart_ack_buffer[UART_ACK_SIZE]; 
static struct uart_state_def uart_state;
static u8 uart_flag_byte = UART_PPP_FLAG_BYTE;
static u8 uart_escape_byte = UART_PPP_ESCAPE_BYTE;

/*
 * Dequeue from buffer and return pointer to data. Return null if empty
 */
static u8 *uart_buf_deq(struct uart_circular_buffer_def *buffer)
{
	int start = buffer->start;
	u8 *data;

	if (start == buffer->end) {
		return NULL;
	}
	data = &buffer->buf[start];
	if (++start >= buffer->size) {
		start = 0;
	}
	buffer->start = start;
	return data;
}

/*
 * Enqueue to buffer. Return -1 if buffer is full.
 */
static int uart_buf_enq(struct uart_circular_buffer_def *buffer, u8 data)
{
	int end = buffer->end;

	if (++end >= buffer->size) {
		end = 0;
	}
	if (end == buffer->start) {
		return -1;
	}
	buffer->buf[buffer->end] = data;
	buffer->end = end;

	return 0;
}

/*
 * Helper function for mcu_uart_build_tx.
 * Enqueues the data, escaping where necessary.
 */
static int uart_build_tx_helper(struct uart_circular_buffer_def *in, u8 *data,
				int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (uart_buf_enq(in, data[i])) {
			return -1;
		}
	}
	return 0;
}

/*
 * Build a packet with the PPP flag bytes + ptype + seq no + data + crc.
 * Starts from index 0.
 */
static int uart_build_tx(struct uart_state_def *muart,
	struct uart_circular_buffer_def *output, enum uart_ptype ptype,
	u8 seq_no, u8 *data, int len)
{
	u16 crc;

	if (uart_buf_enq(output, ptype)) {
		goto full;
	}
	if (uart_buf_enq(output, seq_no)) {
		goto full;
	}
	if (data != NULL && len) {
		if (uart_build_tx_helper(output, data, len)) {
			goto full;
		}
	}
	crc = crc16(&ptype, sizeof(ptype), CRC16_INIT);
	crc = crc16(&seq_no, sizeof(seq_no), crc);
	crc = crc16(data, len, crc);
	put_ua_be16(&crc, crc);
	if (uart_build_tx_helper(output, (u8 *)&crc, 2)) {
		goto full;
	}

	return 0;
full:
	/* if full, cancel building this message */
	STATS(tx_ovrflw_err);
	return -1;
}

/*
 * Check if cb needs to be called
 */
static void uart_check_cb(struct uart_state_def *muart)
{
	void (*tx_cb)(void);

	if (!muart->tx_callback || muart->wait_for_ack ||
	    muart->wait_for_confirm) {
		return;
	}
	tx_cb = muart->tx_callback;
	muart->tx_callback = NULL;
	STATS(tx_cbs);
	tx_cb();
}

/*
 * Process the received frame
 */
static void uart_process_rx(struct uart_state_def *muart)
{
	int i;
	int recv_len;
	u8 *rx_buf = muart->rx_buffer;
	u8 *data_ptr = rx_buf + 2;

	/* unescape the frame */
	for (i = 0, recv_len = 0; i < muart->rx_len; i++, recv_len++) {
		if (rx_buf[i] == UART_PPP_ESCAPE_BYTE) {
			if (i == muart->rx_len - 1) {
				/* bad frame */
				STATS(rx_frame_err);
				goto finish;
			}
			rx_buf[recv_len] = rx_buf[++i] ^ UART_PPP_XOR_BYTE;
		} else {
			rx_buf[recv_len] = rx_buf[i];
		}
	}
	if (recv_len < 4) {
		/* frame too small to be valid, drop it. */
		STATS(rx_len_err);
		goto finish;
	}
	/* check crc */
	if (crc16(rx_buf, recv_len, CRC16_INIT)) {
		/* bad crc, drop packet */
		STATS(rx_crc_err);
		goto finish;
	}
	/* check ptype */
	if (rx_buf[0] != MP_DATA && rx_buf[0] != MP_ACK) {
		/* bad ptype, skip packet */
		STATS(rx_ptype_err);
		return;
	}
	if (rx_buf[0] == MP_ACK) {
		if (muart->wait_for_ack &&
		    rx_buf[1] == muart->tx_seq_no) {
			muart->wait_for_ack = 0;
			if (muart->confirm_needed) {
				muart->confirm_needed = 0;
				muart->wait_for_confirm = 1;
			} else {
				uart_check_cb(muart);
			}
		}
		STATS(rx_acks);
		goto finish;
	}

	if (uart_build_tx(muart, &muart->ack_buf, MP_ACK, rx_buf[1],
	    NULL, 0)) {
		/* couldn't fit the ack in the buffer */
		/* drop the packet and the module will resend it */
		/* hopefully there will be space for an ack then */
		goto finish;
	}
	/* check seq #, make sure its not the lollip seq # */
	if (rx_buf[1] && rx_buf[1] == muart->recved_seq_no) {
		/* already received and processed packet with this seq no */
		STATS(rx_seq_err);
		goto finish;
	}
	muart->recved_seq_no = rx_buf[1];
	recv_len -= 4;	/* drop the crc, ptype, and seq # */
	if (serial_process_inc_pkt(data_ptr, recv_len)) {
		STATS(rx_cmd_err);
	}
finish:
	muart->rx_len = 0;

}

/*
 * Received confirm or nak packet from module. See if its the confirmation
 * we are looking for.
 */
void uart_confirm_or_nak_recvd(u16 req_id)
{
	struct uart_state_def *muart = &uart_state;

	if (muart->wait_for_confirm && muart->confirm_req_id == req_id) {
		muart->wait_for_confirm = 0;
		uart_check_cb(muart);
	} else {
		STATS(rx_unexp_nak_confirm_err);
	}
}

/*
 * Transmit data over uart if tx buffer is available.
 */
int uart_tx_buf_send(int confirm_needed, u16 req_id)
{
	struct uart_state_def *muart = &uart_state;
	u8 data[ASPI_LEN_MAX];

	if (!muart->inited || muart->cur_tx != NULL ||
	    muart->tx_buf.start != muart->tx_buf.end ||
	    muart->wait_for_ack || !muart->tx_len ||
	    (confirm_needed && muart->wait_for_confirm)) {
		/*
		 * not init or transmitting previous data
		 * or still waiting for ack from previous transmission
		 * or the user didn't call uart_tx_buf_get first or
		 * the user asked for confirmation when uart is still
		 * waiting on confirmation/nak from a previous packet.
		 */
		return -1;
	}
	memcpy(data, muart->tx_buf.buf, muart->tx_len);
	muart->tx_buf.start = 0;
	muart->tx_buf.end = 0;
	muart->tx_seq_no++;
	if (muart->first_tx) {
		muart->tx_seq_no = 0;
		muart->first_tx = 0;
	} else if (!muart->tx_seq_no) {
		muart->tx_seq_no = 1;
	}
	uart_build_tx(muart, &muart->tx_buf, MP_DATA, muart->tx_seq_no,
	    data, muart->tx_len);
	muart->wait_for_ack = 1;
	muart->num_retries = 0;
	muart->tick_ct_for_retry = tick +
	    UART_ACK_WAIT / 1000 * SYSTICK_HZ;
	muart->tx_len = 0;
	if (confirm_needed) {
		muart->confirm_needed = 1;
		muart->confirm_req_id = req_id;
	}

	return 0;
}

/*
 * Returns 1 if a prop update hasn't finished sending/receiving confirmation
 */
int uart_is_tx_busy(void)
{
	struct uart_state_def *muart = &uart_state;

	if (muart->wait_for_ack || muart->wait_for_confirm ||
	    (muart->tx_buf.end != muart->tx_buf.start)) {
		return 1;
	}

	return 0;
}

/*
 * If tx_buf is ready for another packet, return a pointer to a 
 * buffer for sending. Otherwise, return null.
 */
static void *uart_tx_buf_get_helper(size_t len, int need_confirm, void (*cb)(void))
{
	struct uart_state_def *muart = &uart_state;

	if (!muart->inited || len > ASPI_LEN_MAX) {
		return NULL;
	}
	if (muart->tx_callback) {
		/* something else has already reserved the transmission */
		return NULL;
	}
	if (muart->wait_for_ack || (need_confirm && muart->wait_for_confirm)) {
		/* still waiting for ack from previous trans */
		/* or waiting for confirmation */
		goto set_cb;
	}
	if (muart->cur_tx == NULL && muart->tx_buf.start == muart->tx_buf.end) {
		muart->tx_len = len;
		return muart->tx_buf.buf;
	}
set_cb:
	if (cb) {
		muart->tx_callback = cb;
	}

	return NULL;
}

/*
 * Resize the length of the tx data packet
 */
void uart_tx_buf_trim(size_t len)
{
	uart_state.tx_len = len;
}

/*
 * Return a buffer if available. The data going into this packet
 * SHOULD NOT be a property update or any data thats being
 * transmitted out. This function can be used for transmits like
 * configuration changes and error messages.
 */
void *uart_tx_buf_get(size_t len, void (*cb)(void))
{
	return uart_tx_buf_get_helper(len, 0, cb);
}

/*
 * Return a buffer if available. Wait for confirmation from module
 * before making this buffer available for anything else that requires
 * confirmation.
 */
void *uart_tx_buf_get_for_props(size_t len, void (*cb)(void))
{
	return uart_tx_buf_get_helper(len, 1, cb);
}

/*
 * Transmit packet if needed
 */
u8 *uart_tx(void)
{
	struct uart_state_def *muart = &uart_state;
	int cur_tx_is_empty;
	int tx_is_empty;
	int ack_is_empty;
	u8 *data;

recheck:
	if (muart->cur_tx == &muart->ack_buf) {
		cur_tx_is_empty = !muart->ack_msg_cur_tx;
	} else {
		cur_tx_is_empty = muart->cur_tx ?
		    (muart->cur_tx->end == muart->cur_tx->start) : 1;
	}
	tx_is_empty = muart->tx_buf.end == muart->tx_buf.start;
	ack_is_empty = muart->ack_buf.end == muart->ack_buf.start;
	if (muart->escape_sent) {
		muart->escape_sent = 0;
		if (muart->cur_tx == &muart->ack_buf) {
			muart->ack_msg_cur_tx--;
		}
		return &muart->escaped_byte;
	}
	if (cur_tx_is_empty) {
		if (muart->head_flag_sent && !muart->tail_flag_sent) {
			muart->tail_flag_sent = 1;
			muart->head_flag_sent = 0;
			return &uart_flag_byte;
		}
		if (!ack_is_empty) {
			STATS(tx_acks);
			muart->cur_tx = &muart->ack_buf;
			muart->ack_msg_cur_tx = UART_PPP_SIZE;
		} else if (!tx_is_empty) {
			STATS(tx_data);
			muart->cur_tx = &muart->tx_buf;
		} else if (muart->wait_for_ack &&
		    (signed)(tick - muart->tick_ct_for_retry) >= 0) {
			if (muart->num_retries++ < UART_MAX_RETRIES) {
				muart->tick_ct_for_retry = tick +
				    UART_ACK_WAIT / 1000 * SYSTICK_HZ;
				muart->tx_buf.start = 0;
				muart->head_flag_sent = 0;
				goto recheck;
			}
			muart->cur_tx = NULL;
			muart->wait_for_ack = 0;
			muart->confirm_needed = 0;
			uart_check_cb(muart);
		} else {
			muart->cur_tx = NULL;
		}
		muart->escape_sent = 0;
	}
	if (muart->cur_tx == NULL) {
		return NULL;
	}
	if (!muart->head_flag_sent) {
		muart->head_flag_sent = 1;
		muart->tail_flag_sent = 0;
		return &uart_flag_byte;
	}
	data = uart_buf_deq(muart->cur_tx);
	if (data == NULL) {
		muart->cur_tx = NULL;
		return NULL;
	}
	if (*data == UART_PPP_FLAG_BYTE ||
	    *data == UART_PPP_ESCAPE_BYTE) {
		muart->escaped_byte = *data ^ UART_PPP_XOR_BYTE;
		muart->escape_sent = 1;
		return &uart_escape_byte;
	}
	if (muart->cur_tx == &muart->ack_buf) {
		muart->ack_msg_cur_tx--;
	}
	return data;
}

/*
 * Put the received packets in a buffer. Once we see a whole frame,
 * call the process function to parse/interpret the frame.
 */
void uart_recv(u8 dr)
{
	struct uart_state_def *muart = &uart_state;

	if (!muart->saw_ppp_flag) {
		if (dr == UART_PPP_FLAG_BYTE) {
			muart->saw_ppp_flag = 1;
		}
		/* saw bytes without the first ppp flag, just drop */
		return;
	}
	if (dr != UART_PPP_FLAG_BYTE &&
	    muart->rx_len < sizeof(muart->rx_buffer)) {
		muart->rx_buffer[muart->rx_len++] = dr;
	} else if (dr == UART_PPP_FLAG_BYTE &&
	    muart->rx_len == 0) {
		return;
	} else {
		uart_process_rx(muart);
	}
}

/*
 * Setup the UART/GPIO Settings + rx and tx buffers
 */
void uart_init(void)
{
	struct uart_state_def *muart = &uart_state;

	uart_platform_init();
	memset(muart, 0, sizeof(uart_state));
	muart->tx_buf.buf = uart_tx_buffer;
	muart->tx_buf.size = sizeof(uart_tx_buffer);
	muart->ack_buf.size = sizeof(uart_ack_buffer);
	muart->ack_buf.buf = uart_ack_buffer;
	muart->first_tx = 1;
	muart->inited = 1;
}

#ifdef PLATFORM_LINUX
/*
 * Show UART statistics
 */
void uart_print_stats(void)
{
	printf("\nUART statistics:\n"
		"tx_cbs %u tx_acks %u tx_data %u tx_reacks %u tx_ovrflw_err %u\n",
		uart_stats.tx_cbs, uart_stats.tx_acks,
		uart_stats.tx_data, uart_stats.tx_reacks,
		uart_stats.tx_ovrflw_err);

	printf("rx_acks %u rx_frame_err %u "
		"rx_len_err %u rx_crc_err %u rx_ptype_err %u "
		"rx_seq_err %u rx_cmd_err %u\n",
		uart_stats.rx_acks,
		uart_stats.rx_frame_err, uart_stats.rx_len_err,
		uart_stats.rx_crc_err, uart_stats.rx_ptype_err,
		uart_stats.rx_seq_err, uart_stats.rx_cmd_err);
}
#endif /* PLATFORM_LINUX */
