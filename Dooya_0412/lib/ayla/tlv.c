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

#include <string.h>
#include <ayla/mcu_platform.h>
#include <ayla/ayla_proto_mcu.h>

/*
 * Get the first TLV of the specified type from a received packet.
 */
struct ayla_tlv *tlv_get(enum ayla_tlv_type type, void *buf, size_t len)
{
	struct ayla_cmd *cmd;
	struct ayla_tlv *tlv;
	size_t rlen;
	size_t tlen;

	if (len < sizeof(*cmd)) {
		return NULL;
	}
	rlen = len - sizeof(*cmd);
	cmd = buf;
	tlv = (struct ayla_tlv *)(cmd + 1);

	while (rlen >= sizeof(*tlv)) {
		tlen = sizeof(*tlv) + tlv->len;
		if (tlen > rlen) {
			return NULL;
		}
		if (tlv->type == type) {
			return tlv;
		}
		rlen -= tlen;
		tlv = (struct ayla_tlv *)((char *)tlv + tlen);
	}
	return NULL;
}

/*
 * Place TLV of a given type to a buffer.
 */
size_t tlv_put(void *buf, enum ayla_tlv_type type, const void *val, size_t len)
{
	struct ayla_tlv *tlv;

	if ((len > 0xff && type != ATLV_FILE) || len > 0x7fff) {
		return 0;
	}
	tlv = buf;
	tlv->type = type | (len >> 8);
	tlv->len = len;
	memcpy(tlv + 1, val, len);
	return len + sizeof(*tlv);
}
