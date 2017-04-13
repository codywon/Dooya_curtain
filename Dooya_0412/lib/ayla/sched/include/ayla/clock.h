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
#ifndef __AYLA_CLOCK_H__
#define __AYLA_CLOCK_H__

extern u32 clock_boot_rel_time;	/* time of startup according to RTC */

/*
 * Daylight settings
 */
struct daylight_settings {
	unsigned valid:1;	/* 1 if settings should be followed */
	unsigned active:1;	/* 1 if DST is active before change */
	u32 change;		/* when DST flips inactive/active */
};

/*
 * Timezone settings
 */
struct timezone_settings {
	unsigned valid:1;	/* 1 if settings should be followed */
	s16 mins;		/* mins west of UTC */
};

/*
 * Calendar information about a particular time
 */
struct clock_info {
	u32 time;		/* time that this struct represents */
	u32 month_start;	/* start time of the month */
	u32 month_end;		/* end time of the month */
	u32 day_start;		/* start time of the day */
	u32 day_end;		/* end time of the day */
	u32 secs_since_midnight;/* secs since midnight */
	u32 year;		/* current year */
	u8 month;		/* current month starting from 1 */
	u8 days;		/* current day of month */
	u8 hour;		/* current hour */
	u8 min;			/* current min */
	u8 sec;			/* current seconds */
	u8 days_left_in_month;	/* # days left in current month */
	u8 day_of_week;		/* day of the week. Mon = 1, Sun = 7 */
	u8 day_occur_in_month;	/* occurence of day in month. i.e. 2nd sun */
	unsigned is_leap:1;	/* flag to signify if year is leap year */
};

/*
 * Return the current UTC time
 */
static inline u32 clock_utc(void)
{
	return tick / SYSTICK_HZ + clock_boot_rel_time;
}

u32 clock_local(const u32 *utc);
int clock_is_leap(u32 year);
void clock_fill_details(struct clock_info *clk, u32 time);
void clock_incr_day(struct clock_info *clk);
void clock_decr_day(struct clock_info *clk);
void clock_incr_month(struct clock_info *clk);
void clock_decr_month(struct clock_info *clk);
u8 clock_get_day_occur_in_month(u32 days);
u32 clock_local_to_utc(u32 local, u8 skip_fb);

extern struct timezone_settings timezone_info;	/* timezone settings */
extern struct daylight_settings daylight_info;	/* daylight settings */

#define DAYLIGHT_OFFSET	3600	/* daylight offset (secs) */
#define	CLOCK_EPOCH	1970U	/* must match Unix Epoch for SSL */
#define	CLOCK_EPOCH_DAY	4	/* day of the week for Jan 1, CLOCK_EPOCH */

/*
 * default value to be set in clock after power loss.
 * This is for testing algorithms and certificates.
 */
#define CLOCK_START_YEAR 2012U
#define CLOCK_START (((CLOCK_START_YEAR - CLOCK_EPOCH) * \
			(365 * 4 + 1) / 4) * 24 * 60 * 60)

#endif /* __AYLA_CLOCK_H__ */
