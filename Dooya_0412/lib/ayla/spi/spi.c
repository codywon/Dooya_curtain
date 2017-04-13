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
 * Example SPI / TLV driver code.
 */
#include <string.h>

#include <ayla/mcu_platform.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/ayla_spi_mcu.h>
#include <ayla/props.h>
#include <ayla/serial_msg.h>
#include <ayla/spi.h>
#include <cm3_intr.h>
#include <ayla/spi_platform.h>
#include <stm32.h>

static u8 spi_rx_buf[ASPI_LEN_MAX];	/* for receive */
static u8 spi_tx_buf[ASPI_LEN_MAX];	/* for transmit */

#define MAX_ATTEMPTS 400
#define SPI_CRC_RETRIES	20	/* number of times to retry CRC error recv */

#define SPI_STATS

#ifndef SPI_STATS
#define STATS(x)
#else
#define STATS(x)	do { (spi_stats.x++); } while (0)

/*
 * Optional debug counters for various conditions.
 * These can be deleted to save space but may help debugging.
 */
static struct spi_stats {
	u16 tx_ok;
	u16 tx_err;
	u16 rx_ok;
	u16 rx_crc;

	u16 rx_retry;		/* retried receives */
	u16 rx_retry_limited;	/* abandoned retry due to limit */
	u16 rx_too_short;
	u16 rx_hung;
	u16 rx_len;

	u16 tx_stat_inval;
	u16 tx_still_busy;
	u16 tx_busy;
	u16 tx_hung;

	u16 rx_stat_inval;
	u16 rx_no_attn;
	u16 rx_no_start;
	u16 rx_too_long;

	u16 rx_unk_proto;
} spi_stats;
#endif /* SPI_STATS */

struct spi_dev {
	enum spi_state {
		SS_IDLE = 0,
		SS_RX_WAIT,
		SS_RX,
		SS_TX_WAIT,	/* wait for non-busy to transmit */
		SS_TX_START,	/* wait for start byte to be accepted */
		SS_TX_END
	} state;
	u8 tx_ready;		/* transmit buffer is ready to send */
	u8 rx_retry_ct;		/* retry count */
	u16 tx_len;		/* length of pending transmit */
};

static struct spi_dev spi_dev;

static void spi_tx_start(void);
static void spi_tx_end(void);
static void spi_rx(void);

static void spi_delay(void)
{
	int i = 50;

	while (i--)
		;
}

/*
 * Return non-zero if SPI is busy with ADS command.
 */
int spi_is_ads_busy(void)
{
	u8 status;
	u8 mask = ASPI_MSTAT_INVAL | ASPI_MSTAT_ADS_BUSY;

	spi_platform_slave_select();

	(void)spi_platform_io(0);

	status = spi_platform_io(0);

	/* xXXXx spi_platform_slave_deselect()? */
	return (status & mask);
}

static u8 spi_tx_get_mask_to_check(u8 protocol, u8 opcode)
{
	u8 mask = ASPI_MSTAT_INVAL | ASPI_MSTAT_BUSY;

	if (protocol == ASPI_PROTO_DATA && 
	    (opcode == AD_SEND_TLV || opcode == AD_DP_CREATE ||
	    opcode == AD_DP_REQ ||  opcode == AD_DP_FETCHED ||
	    opcode == AD_DP_SEND || opcode == AD_REQ_TLV ||
	    opcode == AD_SEND_PROP_RESP || opcode == AD_SEND_NEXT_PROP)) {
		mask |= ASPI_MSTAT_ADS_BUSY;
	}

	return mask;
}

/*
 * Polls for non-busy status
 */
static int spi_tx_check_busy(u8 mask)
{
	struct spi_dev *dev = &spi_dev;
	u8 limit = 16;
	u8 status;

	/*
	 * Poll for valid, non-busy status.
	 */
	do {
		status = spi_platform_io(0);
		if (!(status & mask)) {
			break;
		}
		spi_delay();
	} while (limit-- > 0);

	if (status & ASPI_MSTAT_INVAL) {
		STATS(tx_stat_inval);
		return 1;
	}

	if (status & mask) {
		if (status & ASPI_MSTAT_ATTN) {
			dev->state = SS_RX_WAIT;
			return 2;
		}
		STATS(tx_still_busy);
		return 3;
	}

	return 0;
}

/*
 * Transmit message.
 *
 * This implements flow control by waiting for the slave to be non-busy.
 */
void spi_tx_wait(void)
{
	struct spi_dev *dev = &spi_dev;
	struct ayla_cmd *cmd;
	u8 mask;

	cmd = (struct ayla_cmd *)spi_tx_buf;
	mask = spi_tx_get_mask_to_check(cmd->protocol, cmd->opcode);

	if (spi_tx_check_busy(mask)) {
		return;
	}
	dev->state = SS_TX_START;
	spi_tx_start();
}

/*
 * Send transmit command and wait until slave signals it is ready for data.
 * If it is ready, send data.
 */
static void spi_tx_start(void)
{
	struct spi_dev *dev = &spi_dev;
	u8 *bp;
	u16 limit;
	u16 rem;
	u16 len;
	u8 cmd;
	u8 status;

	/*
	 * Send command until busy status seen.
	 */
	len = dev->tx_len + 1;		/* include CRC byte in length */
	rem = ASPI_LEN(len);
	cmd = ASPI_CMD_MO + rem;
	limit = ASPI_XTRA_CMDS + 1;
	do {
		status = spi_platform_io(cmd);
		if (status & ASPI_MSTAT_BUSY) {
			break;
		}

		/*
		 * spi_delay after a few times if still not accepted.
		 */
		if (limit < ASPI_XTRA_CMDS - 2) {
			spi_delay();
		}
	} while (limit-- > 0);

	if (status & ASPI_MSTAT_INVAL) {
		STATS(tx_stat_inval);
		return;
	}

	if (!(status & ASPI_MSTAT_BUSY)) {
		STATS(tx_busy);
		return;
	}

	/*
	 * Send length bytes.
	 */
	rem = len;
	spi_platform_io(rem >> 8);
	spi_platform_io(rem & 0xff);

	/*
	 * Send payload.
	 */
	spi_platform_crc_en();
	rem = len - 2;
	bp = spi_tx_buf;
	while (rem-- > 0) {
		spi_platform_io(*bp++);
	}
	spi_platform_io_crc(*bp++);		/* send last byte and CRC */
	(void)spi_platform_crc_err();		/* clear CRC status */

	dev->state = SS_TX_END;
	spi_tx_end();
}

static void spi_tx_end(void)
{
	struct spi_dev *dev = &spi_dev;
	u16 limit;
	u8 status = 0;

	/*
	 * Wait for not busy.  This also serves as padding.
	 */
	limit = ASPI_LEN_MAX + 1;
	do {
		status = spi_platform_io(0);
		if (!(status & ASPI_MSTAT_BUSY)) {
			break;
		}
	} while (limit-- > 0);

	spi_platform_slave_deselect();
	if (status & ASPI_MSTAT_BUSY) {
		spi_platform_slave_select();
		STATS(tx_hung);
		return;
	}
	if (status & ASPI_MSTAT_ERR) {
		spi_platform_slave_select();
		STATS(tx_err);
		dev->state = SS_TX_WAIT;
		return;
	}
	STATS(tx_ok);
	dev->state = SS_IDLE;
	dev->tx_ready = 0;
	return;
}

/*
 * Check to see if transmit is OK and we're not busy doing receive.
 * Set transmit length for the buffer, which is not yet filled in.
 * Zero the buffer and return a pointer to the command byte location.
 * Must be called with interrupts disabled.
 *
 * Returns NULL if not ready for transmit.
 */
void *spi_tx_buf_get(size_t len)
{
	struct spi_dev *dev = &spi_dev;

	if (dev->tx_ready) {
		return NULL;
	}
	if (dev->state != SS_IDLE) {
		return NULL;
	}
	if (spi_platform_rx_pending()) {
		return NULL;
	}
	spi_platform_slave_select();
	dev->state = SS_TX_WAIT;
	dev->tx_len = len;
	dev->tx_ready = 1;
	memset(spi_tx_buf, 0, len);
	return spi_tx_buf;
}

/*
 * Check to see if transmit is OK and we're not busy doing receive.
 * Also checks ADS busy to make sure it's ok to send.
 * Set transmit length for the buffer, which is not yet filled in.
 * Zero the buffer and return a pointer to the command byte location.
 * Must be called with interrupts disabled.
 *
 * Returns NULL if not ready for transmit.
 */
void *spi_tx_buf_get_for_props(size_t len)
{
	struct spi_dev *dev = &spi_dev;
	u8 mask = ASPI_MSTAT_INVAL | ASPI_MSTAT_BUSY | ASPI_MSTAT_ADS_BUSY;

	if (dev->tx_ready) {
		return NULL;
	}
	if (dev->state != SS_IDLE) {
		return NULL;
	}
	if (spi_platform_rx_pending()) {
		return NULL;
	}
	spi_platform_slave_select();
	if (spi_tx_check_busy(mask)) {
		if (dev->state == SS_IDLE) {
			spi_platform_slave_deselect();
		}
		return NULL;
	}
	dev->state = SS_TX_WAIT;
	dev->tx_len = len;
	dev->tx_ready = 1;
	memset(spi_tx_buf, 0, len);
	return spi_tx_buf;
}


void spi_tx_buf_trim(size_t len)
{
	struct spi_dev *dev = &spi_dev;

	dev->tx_len = len;
}

/*
 * Wait for non-busy with ATTN status before receive.
 */
static void spi_rx_wait(void)
{
	struct spi_dev *dev = &spi_dev;
	int i;
	u8 status;

	/*
 	 * Discard first status to let slave update its data register.
	 */
	(void)spi_platform_io(0);

	/*
	 * Check for non-busy, valid status from module
	 * If no attention from module, just return.
	 */
	i = MAX_ATTEMPTS;
	do {
		status = spi_platform_io(0);
		if (!(status & (ASPI_MSTAT_INVAL | ASPI_MSTAT_BUSY))) {
			break;
		}
		spi_delay();
	} while (i-- > 0);
	if (status & ASPI_MSTAT_INVAL) {
		STATS(rx_stat_inval);
		spi_platform_slave_deselect();
		spi_platform_slave_select();
		return;
	}
	if (status & ASPI_MSTAT_BUSY) {
		STATS(rx_hung);
		spi_platform_slave_deselect();
		spi_platform_slave_select();
		return;
	}
	if (!(status & ASPI_MSTAT_ATTN) && !dev->rx_retry_ct) {
		STATS(rx_no_attn);
		dev->state = SS_IDLE;
		spi_platform_slave_deselect();
		return;
	}
	dev->state = SS_RX;
	spi_rx();
}

/*
 * Send receive command and wait for it to be echoed.
 */
static void spi_rx(void)
{
	struct spi_dev *dev = &spi_dev;
	u16 rx_len;
	int i;
	int j;
	u8 byte;

	/*
	 * Send CMD_MI and wait for reply of 0xf1 start.
	 */
	for (i = 0; i < MAX_ATTEMPTS; i++) {
		byte = spi_platform_io(dev->rx_retry_ct ? ASPI_CMD_MI_RETRY :
		    ASPI_CMD_MI);
		if (byte == ASPI_CMD_MI) {
			break;
		}
		spi_delay();
	}

	if (byte != ASPI_CMD_MI) {
		STATS(rx_no_start);
		return;
	}

	/*
	 * Read until non-start byte appears.
	 * That will be the most-significant length byte
	 */
	do {
		byte = spi_platform_io(0);
	} while (byte == ASPI_CMD_MI);

	/*
	 * Read low-order byte of length.
	 */
	rx_len = (byte << 8) | spi_platform_io(0);
	if (rx_len < 2) {
		STATS(rx_too_short);
		goto flush_input;
	}

	/*
	 * If length is excessive, don't trust it.
	 */
	if (rx_len > ASPI_LEN_MAX || rx_len > sizeof(spi_rx_buf)) {
		STATS(rx_too_long);
		goto flush_input;
	}

	spi_platform_crc_en();
	rx_len -= 2;		/* handle last byte and CRC after loop */
	for (j = 0; j < rx_len; j++) {
		spi_rx_buf[j] = spi_platform_io(j & 0x7f);	/* XXX */
	}
	spi_rx_buf[j] = spi_platform_io_crc(1);
	rx_len++;

	if (spi_platform_crc_err()) {
		STATS(rx_crc);
		goto do_retry;
	}

	if (rx_len > sizeof(spi_rx_buf)) {
		rx_len = sizeof(spi_rx_buf);
	}
	STATS(rx_ok);
	dev->rx_retry_ct = 0;
	dev->state = SS_IDLE;
	spi_platform_slave_deselect();
	spi_stats.rx_len = rx_len;   /* for debugging */
	if (serial_process_inc_pkt(spi_rx_buf, rx_len)) {
		STATS(rx_unk_proto);
	}
	return;

flush_input:
	for (i = 0; i < ASPI_LEN_MAX; i++) {
		(void)spi_platform_io(0);
	}
do_retry:
	spi_platform_slave_deselect();
	if (dev->rx_retry_ct++ < SPI_CRC_RETRIES) {
		STATS(rx_retry);
		dev->state = SS_RX_WAIT;
		spi_platform_slave_select();
	} else {
		STATS(rx_retry_limited);
		dev->rx_retry_ct = 0;
		dev->state = SS_IDLE;
	}
}


u8 spi_status(void)
{
	u8 status;

	if (!stm32_ready()) {
		return 0xff;
	}

	spi_platform_slave_select();
	status = spi_platform_io(0);
	spi_platform_slave_deselect();
	return status;
}

/*
 * Poll for incoming messages.
 */
void spi_poll(void)
{
	struct spi_dev *dev = &spi_dev;
	int pri;

	if (!stm32_ready()) {
		return;
	}
	pri = intr_disable_save();
	switch (dev->state) {
	case SS_IDLE:
		if (spi_platform_rx_pending()) {
			dev->state = SS_RX_WAIT;
			spi_platform_slave_select();
			spi_rx_wait();
		} else if (dev->tx_ready) {
			spi_platform_slave_select();
			dev->state = SS_TX_WAIT;
		}
		break;
	case SS_RX_WAIT:
		spi_rx_wait();
		break;
	case SS_RX:
		spi_rx();
		break;
	case SS_TX_WAIT:
		spi_tx_wait();
		break;
	case SS_TX_START:
		spi_tx_start();
		break;
	case SS_TX_END:
		spi_tx_end();
		break;
	}
	intr_restore(pri);
}
