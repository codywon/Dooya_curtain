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
#ifndef __AYLA_SCHEDEVAL_H__
#define __AYLA_SCHEDEVAL_H__

#define SCHED_NAME_LEN	28

#define SCHED_TLV_LEN	256		/* max length of schedule TLVs */

struct sched_prop {
//	char name[SCHED_NAME_LEN];	/* name of schedule */
	u8 len;						/* length of schedule */
	u8 tlvs[SCHED_TLV_LEN];		/* base64-decoded tlvs of schedule */
};

struct schedule {
	u32 start_date;		/* inclusive */
	u32 end_date;		/* inclusive */
	u32 days_of_month;	/* 32-bit mask. last bit = last day of month */
	u32 start_time_each_day;/* X secs since midnight. inclusive */
	u32 end_time_each_day;	/* X secs since midnight. inclusive */
	u32 duration;		/* superseded by end_time_each_day */
	u32 interval;		/* start every X secs since start time */
	u16 months_of_year;	/* 12-bit mask. bit 0 = jan, bit 11 = dec */
	u8 days_of_week;	/* 7-bit mask. bit 0 = sunday, bit 6 = sat */
	u8 day_occur_in_month;	/* 6-bit mask. 0x80 means last occur */
	unsigned is_utc:1;	/* 1 if the schedule is for UTC */
};

/*
 * Given a schedule, and the current time, this function will:
 * Case A: Time falls within the schedule interval. range_start
 * is set to the begininng of the interval and range_end is the end.
 * Case B: Time is before the scheduled interval. range_start
 * will be set to the beginning of the next interval.
 * Case C: No more events in the schedule after time. range_start
 * and range_end will both be set to 0.
 * This function will take into account intervals.
 */
int sched_determine_range(const struct schedule *schedule,
		u32 time, u32 *range_start, u32 *range_end, u8 toplevel);

/*
 * Takes a schedtlv structure and fills a schedule structure with
 * the information.
 * Uses the current UTC time to calculate when the next event will
 * occur in UTC time. Returns this value.
 * If it returns 0 or MAX_U32, no more events are set to occur for
 * this schedule.
 */
u32 sched_evaluate(struct sched_prop *schedtlv, u32 time);

#endif /* __AYLA_SCHEDEVAL_H__ */
