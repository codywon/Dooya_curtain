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
#ifndef __AYLA_BYTE_ORDER_H__
#define __AYLA_BYTE_ORDER_H__

/*
 * Assuming little-endian (or unknown) for now.
 */
 
static inline void put_ua_be32(void *dest, u32 val)
{
	u8 *byte = dest;

	byte[0] = val >> 24;
	byte[1] = val >> 16;
	byte[2] = val >> 8;
	byte[3] = val;
}

static inline void put_ua_be16(void *dest, u16 val)
{
	u8 *byte = dest;

	byte[0] = val >> 8;
	byte[1] = val;
}

static inline u32 get_ua_be32(void *src)
{
	u8 *byte = src;

	return ((u32)byte[0] << 24) | ((u32)byte[1] << 16) |
	    ((u32)byte[2] << 8) | byte[3];
}

static inline u16 get_ua_be16(void *src)
{
	u8 *byte = src;

	return ((u16)byte[0] << 8) | byte[1];
}

int get_ua_with_len(void *src, u8 len, u32 *dest);

#endif /* __AYLA_BYTE_ORDER_H__ */
