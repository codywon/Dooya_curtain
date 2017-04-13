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
#ifndef __AYLA_PROP_DP_H__
#define __AYLA_PROP_DP_H__


/*
 * prop_dp_init() - set callbacks on the prop_dp so it is ready to use.
 *                  If prop_set is null, prop is read-only.
 */
void prop_dp_init(struct prop_dp *dp,
	int (*prop_get)(struct prop *prop, size_t off, void *buf, size_t len),
	int (*prop_set)(struct prop *prop, size_t off, void *buf, size_t len, u8 eof));

/*
 * prop_dp_send() - send streaming data point for property.
 */
int prop_dp_send(struct prop *, void *);

/*
 * prop_dp_set() - receive streaming data point for property.
 */
void prop_dp_set(struct prop *, void *arg, void *, size_t len);

/*
 * prop_dp_start_send() - send streaming data point for property.
 */
int prop_dp_start_send(struct prop *, struct prop_dp *, u32 len);

/*
 * prop_dp_resp() - receive response from module to datapoint request.
 */
void prop_dp_resp(struct ayla_cmd *, void *, size_t);

/*
 * prop_dp_get_resp() - receive response from module to datapoint get request.
 */
void prop_dp_get_resp(struct ayla_cmd *, void *, size_t);

/*
 * prop_dp_nak() - receive NAK from module to datapoint request.
 * Returns zero if NAK is for the current data point request.
 */
int prop_dp_nak(struct ayla_cmd *, void *, size_t);

/*
 * prop_dp_abort() - abort any ongoing file operation
 */
int prop_dp_abort(void);

#endif /*  __AYLA_PROP_DP_H__ */
