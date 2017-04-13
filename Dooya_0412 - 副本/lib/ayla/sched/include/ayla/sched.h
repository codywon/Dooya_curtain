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
#ifndef __AYLA_SCHED_H__
#define __AYLA_SCHED_H__

/* tick count when next event will happen */
extern u32 sched_next_event_tick;

/*
 * Run through all schedules. Fire events as time progresses
 * to current utc time. Determine the next future event.
 *
 */
void sched_run_all(u32 *tick_ct_to_use);

/*
 * Handle a schedule property from service using the module library.
 */
int sched_prop_set(const char *name, const void *val_ptr, size_t val_len);

/*
 * Logging for sched
 */
void sched_log(const char *fmt, ...);

/*
 * Converts from network byte order to host byte order
 */
int sched_int_get(struct ayla_tlv *atlv, long *value);

/*
 * Reads the schedule action and fires it.
 */
void sched_set_prop(struct ayla_tlv *atlv, u8 tot_len);

/*
 * Update the latest time info
 */
void sched_update_time_info(enum conf_token token, struct ayla_tlv *tlv);

#endif /* __AYLA_SCHED_H__ */
