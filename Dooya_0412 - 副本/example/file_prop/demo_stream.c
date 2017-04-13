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
#include <string.h>
#include <ayla/mcu_platform.h>
#include <ayla/byte_order.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/prop_dp.h>

struct prop_dp stream_up_state;
struct prop_dp stream_down_state;
static u8 stream_down_patt_match;
int stream_up_len;
u32 stream_down_patt_match_len;

#define TEST_PATT_BASE 0x11223344

/*
 * Return the test pattern byte value for a given offset.
 * This is a four-byte counting pattern, in big-endian byte order.
 * The starting bytes are in TEST_PATT_BASE.
 */
static u8 test_patt(size_t off)
{
	u32 patt;

	patt = off / 4;
	patt += TEST_PATT_BASE;
	off &= 3;
	off = 24 - off * 8;
	return (patt >> off) & 0xff;
}

/*
 * Return test_pattern for testing sending large datapoints to the service.
 * The offset and length is given. 
 * The data pattern is a 16-bit counting pattern for now.
 */
static int test_patt_get(struct prop *prop, size_t off, void *buf, size_t len)
{
	u8 *bp;
	int out_len = 0;
	
	bp = buf;
	for (out_len = 0; out_len < len; out_len++) {
		*bp++ = test_patt(off++);
	}
	return out_len;
}

/*
 * Accept new value for stream_down property.
 * This just accumulates the length and tests for matching the test pattern.
 */
static int test_patt_set(struct prop *prop, size_t off,
			void *buf, size_t len, u8 eof)
{
	struct prop_dp *dp;
	u8 *bp;

	dp = prop->arg;
	if (off == 0) {
		stream_down_patt_match = 1;
		stream_down_patt_match_len = 0;
	}
	if (dp->next_off != off) {
		/* XXX handle bad offset here */
		stream_down_patt_match = 0;
	}

	dp->next_off = off + len;

	if (stream_down_patt_match) {
		for (bp = buf; len > 0; len--, bp++, off++) {
			if (*bp != test_patt(off)) {
				stream_down_patt_match = 0;
				break;
			}
			stream_down_patt_match_len = off + 1;
		}
	}

	if (eof) {
		prop_send_req("stream_down_len");
		prop_send_req("stream_down_match_len");
	}
	return 0;
}

/*
 * Set length of test pattern stream to send up to module.
 * Setting this causes the transfer to start, unless it is already in progress.
 */
void set_length_up(struct prop *prop, void *arg, void *valp, size_t len)
{
	s32 i = *(s32 *)valp;
	struct prop *stream_prop;

	if (len != sizeof(s32) || i <= 0) {
		return;
	}
	stream_prop = prop_lookup("stream_up");	
	if (!stream_prop) {
		return;
	}
	prop_dp_start_send(stream_prop, &stream_up_state, i);
	prop->send_mask = valid_dest_mask; /* set stream_up_len back to 0 */
}

void demo_stream_init(void)
{
	prop_dp_init(&stream_up_state, test_patt_get, NULL);
	prop_dp_init(&stream_down_state, NULL, test_patt_set);
}
