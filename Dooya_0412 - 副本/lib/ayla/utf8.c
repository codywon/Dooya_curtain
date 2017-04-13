/*
 * Copyright 2012-2014 Ayla Networks, Inc.  All rights reserved.
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

#include <stddef.h>

#include <ayla/mcu_platform.h>
#include <mcu_io.h>

/*
 * Put up to 4 UTF-8 bytes to a buffer from a int value up to 0x10ffff.
 *
 * 4-byte UTF-8 can handle up to 21-bit values, but Unicode further restricts
 * this to values up to 0x10ffff.
 */
static int utf8_put_wchar(u8 *bp, u32 val)
{
	if (val > 0x10ffff) {
		return -1;
	}
	if (val >= 0x10000) {
		/* 21 bits */
		bp[0] = 0xf0 + ((val >> 18) & 7);
		bp[1] = 0x80 + ((val >> 12) & 0x3f);
		bp[2] = 0x80 + ((val >> 6) & 0x3f);
		bp[3] = 0x80 + (val & 0x3f);
		return 4;
	}
	if (val >= 0x800) {
		/* 16 bits */
		bp[0] = 0xe0 + ((val >> 12) & 0xf);
		bp[1] = 0x80 + ((val >> 6) & 0x3f);
		bp[2] = 0x80 + (val & 0x3f);
		return 3;
	}
	if (val >= 0x80) {
		/* 11 bits */
		bp[0] = 0xc0 + ((val >> 6) & 0x1f);
		bp[1] = 0x80 + (val & 0x3f);
		return 2;
	}
	/* 7 bits */
	bp[0] = val;
	return 1;
}

/*
 * Get next UTF-8 token from a buffer.
 * Returns length, or zero if invalid UTF-8 code encountered.
 */
static size_t utf8_get(u32 *result, u8 *buf, size_t len)
{
	u32 val = 0;
	size_t rlen = 0;
	u8 test[4];
	int i;
	u8 c;

	if (len == 0) {
		return 0;
	}
	c = buf[0];
	if (c < 0x80) {
		*result = c;
		return 1;
	}
	if ((c & 0xf8) == 0xf0) {
		val = c & 7;
		rlen = 4;
	} else if ((c & 0xf0) == 0xe0) {
		val = c & 0xf;
		rlen = 3;
	} else if ((c & 0xe0) == 0xc0) {
		if (c == 0xc0 || c == 0xc1) {
			return 0;
		}
		val = c & 0x1f;
		rlen = 2;
	} else if ((c & 0xc0) == 0x80) {
		return 0;
	}
	if (len < rlen) {
		return 0;
	}
	for (i = 1; i < rlen; i++) {
		c = buf[i];
		if ((c & 0xc0) != 0x80) {
			return 0;
		}
		val = (val << 6) | (c & 0x3f);
	}

	/*
	 * Check for over-long coding, which is invalid.
	 */
	if (utf8_put_wchar(test, val) != rlen) {
		return 0;
	}
	*result = val;
	return rlen;
}

/*
 * Get multiple UTF-8 wide characters from a buffer.
 * Returns the number of characters put in result, or -1 on error.
 */
int utf8_gets(u32 *result, int rcount, u8 *buf, size_t len)
{
	int tlen;
	int count;

	for (count = 0; count < rcount && len > 0; count++) {
		tlen = utf8_get(&result[count], buf, len);
		if (tlen <= 0) {
			return -1;
		}
		len -= tlen;
		buf += tlen;
	}
	if (len > 0) {
		return -1;
	}
	return count;
}
