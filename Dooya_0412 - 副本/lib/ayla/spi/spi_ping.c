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
#include <ayla/ayla_spi_mcu.h>
#include <ayla/spi.h>
#include <cm3_intr.h>
#include <ayla/byte_order.h>

struct spi_ping_result {
	size_t	len;
	u16	start;
	u16	errors;
};
static struct spi_ping_result spi_ping_result;

static void spi_ping_pattern_fill(u8 *buf, size_t len, u16 data)
{
	be16 *bp;

	for (bp = (be16 *)buf; (u8 *)bp < buf + len - 1; bp++) {
		put_ua_be16(bp, data++);
	}
	if ((u8 *)bp < buf + len) {
		*(u8 *)bp = data >> 8;
	}
}

/*
 * Handle received ping packet
 * Fill in spi_ping_result with info from packet.
 */
void spi_ping_rx(void *buf, size_t len)
{
	struct spi_ping_result *result = &spi_ping_result;
	struct ayla_cmd *cmd;
	be16 *bp;
	u16 saw;
	u16 expect;

	cmd = buf;
	result->len = len;
	if (len < 3 || cmd->protocol != ASPI_PROTO_PING) {
		result->errors = 1;
		return;
	}
	bp = (u16 *)((char *)buf + 1);
	saw  = get_ua_be16(bp++);
	result->start = saw;
	for (expect = saw + 1; (char *)bp < (char *)buf + len - 1; expect++) {
		saw = get_ua_be16(bp++);
		if (saw != expect) {
			result->errors++;
		}
	}
	if ((u8 *)bp < (u8 *)buf + len) {
		saw = *(u8 *)bp; 
		if (saw != expect >> 8) {
			result->errors++;
		}
	}
}

/*
 * Send test-pattern packet over SPI and read it back.
 *
 * The test pattern is a ping with data with a 2-byte counting pattern.
 *
 * len is the length in the range of 3 to 384 bytes, inclusive.
 * pattern_start is the first 2-byte value of the pattern to be used.
 *
 * This function calles spi_poll() to let the SPI packet state machine send
 * and receive other packets that may be required by the module, so other
 * callbacks, for example to receive property settings, may be called during
 * this function.
 *
 * Returns 0 on success, positive error count otherwise,
 * -1 on invalid length, -2 if busy or not ready.
 */
int spi_ping_test(size_t len, u16 pattern_start)
{
	struct spi_ping_result *result = &spi_ping_result;
	u8 patt[ASPI_LEN_MAX];
	struct ayla_cmd *cmd;
	void *buf;
	int pri;

	if (len < 2 || len > ASPI_LEN_MAX) {
		return -1;
	}
	pri = intr_disable_save();
	while ((buf = spi_tx_buf_get(len)) == NULL) {
		intr_restore(pri);
		spi_poll();
		pri = intr_disable_save();
	}
	spi_ping_pattern_fill((u8 *)buf + 1, len - 1, pattern_start);
	cmd = buf;
	cmd->protocol = ASPI_PROTO_PING;

	memcpy(patt, cmd, len);
	memset(result, 0, sizeof(*result));

	spi_tx_wait();
	intr_restore(pri);

	/*
	 * Wait for packet to be received.
	 */
	while (!result->len) {
		spi_poll();
	}

	if (result->len != len) {
		result->errors++;
	}
	return result->errors;
}
