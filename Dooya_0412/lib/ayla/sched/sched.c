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
#include <mcu_io.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/conf_token.h>
#include <ayla/props.h>
#include <ayla/schedeval.h>
#include <ayla/clock.h>
#include <ayla/byte_order.h>
#include <ayla/sched.h>


u32 clock_boot_rel_time;	/* time of startup according to RTC */
u32 sched_next_event_tick;	/* tick count when next event will happen */

/*
 * Converts from network byte order to host byte order
 */
int sched_int_get(struct ayla_tlv *atlv, long *value)
{
	switch (atlv->len) {
	case 1:
		*value = *(u8 *)(atlv + 1);
		break;
	case 2:
		*value = get_ua_be16(atlv + 1);
		break;
	case 4:
		*value = get_ua_be32(atlv + 1);
		break;
	default:
		return -1;
	}
	return 0;
}

static void sched_clock_set(u32 new_time)
{
	clock_boot_rel_time = new_time - tick / SYSTICK_HZ;
}

#ifdef AYLA_TIME
/*
 * Time information update from module
 */
void sched_update_time_info(enum conf_token token, struct ayla_tlv *tlv)
{
	long value;

	sched_int_get(tlv, &value);
	switch(token) {
	case CT_time:
		sched_clock_set(value);
		break;
	case CT_timezone_valid:
		timezone_info.valid = (value != 0);
		if (!timezone_info.valid) {
			timezone_info.mins = 0;
		}
		break;
	case CT_timezone:
		timezone_info.mins = (s32)value;
		break;
	case CT_dst_valid:
		daylight_info.valid = (value != 0);
		if (!daylight_info.valid) {
			daylight_info.active = 0;
			daylight_info.change = 0;
		}
		break;
	case CT_dst_active:
		daylight_info.active = (value != 0);
		break;
	case CT_dst_change:
		daylight_info.change = (u32)value;
		break;
	default:
		break;
	}
}
#endif

#ifdef AYLA_SCHED
void sched_run_all(u32 *tick_ct_to_use)
{
	struct prop *prop;
	u32 curtime;
	u32 curtick;
	u32 next_event;
	u32 earliest_event = MAX_U32;

	if (!clock_boot_rel_time) {
		return;
	}
	if (tick_ct_to_use) {
		curtick = *tick_ct_to_use;
	} else {
		curtick = tick;
	}
	sched_next_event_tick = 0;
	curtime = clock_boot_rel_time + curtick / SYSTICK_HZ;
	for (prop = prop_table; prop->name != NULL; prop++) {
		/* find the next schedule to fire out of the table */
		if (prop->type == ATLV_SCHED && prop->arg) {
			next_event = sched_evaluate(prop->arg, curtime);
			if (!next_event || next_event == MAX_U32) {
				/* no more events to fire for this schedule */
				continue;
			}
			if (next_event < earliest_event) {
				earliest_event = next_event;
			}
		}
	}
	if (earliest_event == MAX_U32) {
		/* no events left for any of the schedules */
		return;
	}
	sched_next_event_tick =
	    (earliest_event - clock_boot_rel_time) * SYSTICK_HZ;
}

/*
 * Reads the schedule action and fires it.
 */
void sched_set_prop(struct ayla_tlv *atlv, u8 tot_len)
{
	struct prop *prop;
	struct ayla_tlv *tlv;
	char name[PROP_NAME_LEN];
	long val;
	int name_tlv_len;

	tlv = (struct ayla_tlv *)(atlv);
	if (tlv->type != ATLV_NAME || tlv->len >= sizeof(name) ||
	    tlv->len + sizeof(struct ayla_tlv) > tot_len) {
		return;
	}
	memcpy(name, tlv + 1, tlv->len);
	name[tlv->len] = '\0';
	name_tlv_len = tlv->len;

	prop = prop_lookup(name);
	if (!prop) {
		return;
	}
	if (prop->type != ATLV_BOOL &&
	    prop->type != ATLV_INT) {
		/* only bool and int sched actions
		 * are supported for now.
		 */
		return;
	}

	tlv = (struct ayla_tlv *)((u8 *)tlv + tlv->len +
	    sizeof(struct ayla_tlv));

	if (tlv->len + name_tlv_len + 2 * sizeof(struct ayla_tlv) > tot_len) {
		return;
	}
	if (sched_int_get(tlv, &val)) {
		return;
	}
	prop->set(prop, prop->arg, &val, (prop->type == ATLV_BOOL) ? sizeof(u8) : sizeof(u32)/* tlv->len */);
	prop->echo = 1;
/*	prop->send_mask = ALL_DESTS_BIT; Remarked by Paul Wang */
}

/*
 * Logging for sched
 */
void sched_log(const char *fmt, ...)
{
}

#endif /* AYLA_SCHED */
