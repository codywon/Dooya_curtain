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
#include <string.h>
#ifdef AYLA_TIME	/* for host mcu build */
#include <ayla/mcu_platform.h>
#include <mcu_io.h>
#include <ayla/clock.h>
#else
#include <ayla/utypes.h>
#include <ayla/clock.h>
#endif

#define	ONE_DAY		(24 * 60 * 60)

struct timezone_settings timezone_info;
struct daylight_settings daylight_info;

int clock_is_leap(u32 year)
{
	/*
	if year is divisible by 400 then
		is_leap_year
	else if year is divisible by 100 then
		not_leap_year
	else if year is divisible by 4 then
		is_leap_year
	else
		not_leap_year
	*/

	return !((year % 4) || ((year % 100) == 0 && (year % 400)));
}

/*
 * Return which occurance for this day is in this month.
 * i.e. 1st Sunday, 3rd Thursday
 */
u8 clock_get_day_occur_in_month(u32 days)
{
	return (days + 6) / 7;
}

/*
 * Fill the clock_info structure for a particular time.
 */
void clock_fill_details(struct clock_info *clk, u32 time)
{
	u32 tmp;
	u32 year;
	u8 month;
	static u8 mdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31,
	    30, 31 };

	clk->time = time;
	clk->sec = time % 60;
	time /= 60;
	clk->min = time % 60;
	time /= 60;
	clk->hour = time % 24;
	time /= 24;

	clk->day_of_week = (time + CLOCK_EPOCH_DAY) % 7;
	if (!clk->day_of_week) {
		clk->day_of_week = 7;
	}
	clk->secs_since_midnight = clk->hour * 60 *  60 +
	    clk->min * 60 + clk->sec;
	clk->day_start = clk->time - clk->secs_since_midnight;
	if (ONE_DAY - 1 > MAX_U32 - clk->day_start) {
		clk->day_end = MAX_U32;
	} else {
		clk->day_end = clk->day_start + ONE_DAY - 1;
	}

	/*
	 * here time is days since epoch
	 * XXX I'm sure this can be more clever and avoid the loops.
	 */
	for (year = CLOCK_EPOCH;; year++) {
		clk->is_leap = clock_is_leap(year);
		tmp = 365 + clk->is_leap;
		if (time < tmp)
			break;
		time -= tmp;
	}
	for (month = 1; ; month++) {
		clk->days = mdays[month - 1] + (month == 2 ? clk->is_leap : 0);
		if (time < clk->days)
			break;
		time -= clk->days;
	}
	clk->month = month;
	clk->year = year;
	clk->days = time + 1;		/* first day of month isn't 0 */
	clk->days_left_in_month = mdays[month - 1] +
	    (month == 2 ? clk->is_leap : 0) - clk->days;
	clk->day_occur_in_month = clock_get_day_occur_in_month(clk->days);
	clk->month_start = clk->day_start - ((clk->days - 1) * ONE_DAY);
	if (clk->days_left_in_month * ONE_DAY > MAX_U32 - clk->day_end) {
		clk->month_end = MAX_U32;
	} else {
		clk->month_end = clk->day_end +
		    clk->days_left_in_month * ONE_DAY;
	}
}

/*
 * Increments the day represented by clk and updates the details
 */
void clock_incr_day(struct clock_info *clk)
{
	static u8 mdays[12] = { 31, 28, 31, 30, 31, 30,
	    31, 31, 30, 31, 30, 31 };
	u8 recalc_month_end = 0;

	if (MAX_U32 - clk->time < ONE_DAY) {
		clk->time = MAX_U32;
	} else {
		clk->time += ONE_DAY;
	}
	clk->day_of_week++;
	if (clk->day_of_week == 8) {
		clk->day_of_week = 1;
	}
	clk->day_start = clk->time - clk->secs_since_midnight;
	if (clk->time == MAX_U32 || ONE_DAY - 1 > MAX_U32 - clk->day_start) {
		clk->day_end = MAX_U32;
	} else {
		clk->day_end = clk->day_start + ONE_DAY - 1;
	}
	if (!clk->days_left_in_month) {
		if (clk->month == 12) {
			clk->month = 1;
			clk->year++;
			clk->is_leap = clock_is_leap(clk->year);
		}
		clk->days = 1;
		clk->month_start = clk->day_start;
		recalc_month_end = 1;
	} else {
		clk->days++;
	}
	clk->day_occur_in_month = clock_get_day_occur_in_month(clk->days);
	clk->days_left_in_month = mdays[clk->month - 1] +
	    (clk->month == 2 ? clk->is_leap : 0) - clk->days;
	if (!recalc_month_end) {
		return;
	}
	if (clk->days_left_in_month * ONE_DAY > MAX_U32 - clk->day_end) {
		clk->month_end = MAX_U32;
	} else {
		clk->month_end = clk->day_end +
		    clk->days_left_in_month * ONE_DAY;
	}
}

/*
 * Decrements the day represented by clk and updates the details
 */
void clock_decr_day(struct clock_info *clk)
{
	static u8 mdays[12] = { 31, 28, 31, 30, 31, 30,
	    31, 31, 30, 31, 30, 31 };
	u8 recalc_month_start = 0;

	if (clk->time < ONE_DAY) {
		clk->time = 0;
	} else {
		clk->time -= ONE_DAY;
	}
	clk->day_of_week--;
	if (!clk->day_of_week) {
		clk->day_of_week = 7;
	}
	clk->day_start = clk->time - clk->secs_since_midnight;
	clk->day_end = clk->day_start + ONE_DAY - 1;
	clk->days--;
	if (!clk->days) {
		clk->month--;
		if (!clk->month) {
			clk->month = 12;
			clk->year--;
			clk->is_leap = clock_is_leap(clk->year);
		}
		clk->days = mdays[clk->month - 1] +
		    (clk->month == 2 ? clk->is_leap : 0);
		clk->month_end = clk->day_end;
		recalc_month_start = 1;
	}
	clk->days_left_in_month = mdays[clk->month - 1] +
	    (clk->month == 2 ? clk->is_leap : 0) - clk->days;
	clk->day_occur_in_month = clock_get_day_occur_in_month(clk->days);
	if (!recalc_month_start) {
		return;
	}
	if (!clk->time || clk->days * ONE_DAY > clk->month_end) {
		clk->month_start = 0;
	} else {
		clk->month_start = clk->month_end - clk->days * ONE_DAY;
	}
}

/*
 * Increments the month represented by clk and updates the details.
 * Just sets the time to the start of the following month.
 */
void clock_incr_month(struct clock_info *clk)
{
	static u8 mdays[12] = { 31, 28, 31, 30, 31, 30,
	    31, 31, 30, 31, 30, 31 };

	if (clk->month_end == MAX_U32) {
		return;
	}
	clk->sec = 0;
	clk->min = 0;
	clk->hour = 0;
	clk->secs_since_midnight = 0;
	clk->month_start = clk->month_end + 1;
	clk->day_start = clk->month_start;
	clk->time = clk->month_start;
	clk->day_of_week = (clk->day_of_week + clk->days_left_in_month) % 7;
	if (!clk->day_of_week) {
		clk->day_of_week = 7;
	}
	if (ONE_DAY - 1 > MAX_U32 - clk->day_start) {
		clk->day_end = MAX_U32;
	} else {
		clk->day_end = clk->day_start + ONE_DAY - 1;
	}
	clk->month++;
	if (clk->month == 13) {
		clk->month = 1;
		clk->year++;
		clk->is_leap = clock_is_leap(clk->year);
	}
	clk->days = 1;
	clk->day_occur_in_month = 1;
	clk->days_left_in_month = mdays[clk->month - 1] +
	    (clk->month == 2 ? clk->is_leap : 0) - clk->days;
	if (clk->days_left_in_month * ONE_DAY > MAX_U32 - clk->day_end) {
		clk->month_end = MAX_U32;
	} else {
		clk->month_end = clk->day_end +
		    clk->days_left_in_month * ONE_DAY;
	}
}

/*
 * Decrements the month represented by clk and updates the details.
 * Just sets the time to the last day of the previous month.
 */
void clock_decr_month(struct clock_info *clk)
{
	static u8 mdays[12] = { 31, 28, 31, 30, 31, 30,
	    31, 31, 30, 31, 30, 31 };

	if (!clk->month_start) {
		return;
	}
	clk->month_end = clk->month_start - 1;
	clk->day_end = clk->month_end;
	clk->time = clk->month_end;
	clk->sec = 59;
	clk->min = 59;
	clk->hour = 23;
	clk->secs_since_midnight = ONE_DAY - 1;
	clk->month--;
	if (clk->day_end < ONE_DAY - 1) {
		clk->day_start = 0;
	} else {
		clk->day_start = clk->day_end - (ONE_DAY - 1);
	}
	if (!clk->month) {
		clk->month = 12;
		clk->year--;
		clk->is_leap = clock_is_leap(clk->year);
	}
	clk->days = mdays[clk->month - 1] +
	    (clk->month == 2 ? clk->is_leap : 0);
	clk->days_left_in_month = 0;
	clk->day_occur_in_month = clock_get_day_occur_in_month(clk->days);
	clk->day_of_week = (clk->time / ONE_DAY + CLOCK_EPOCH_DAY) % 7;
	if (clk->day_start < (clk->days - 1) * ONE_DAY) {
		clk->month_start = 0;
	} else {
		clk->month_start = clk->day_start - (clk->days - 1) * ONE_DAY;
	}
}

/*
 * Get local time. Takes into account timezone + DST
 * Optional pointer to current UTC time to calculate
 * local time for.
 */
u32 clock_local(const u32 *utc)
{
#if defined(SCHED_TEST) || defined(DEMO_TIME_LIB)
	u32 utc_time = *utc;
#else
	u32 utc_time = (utc) ? *utc : clock_utc();
#endif
	u32 local_time;

	if (!timezone_info.valid) {
		return utc_time;
	}
	local_time = utc_time - timezone_info.mins * 60;
	if (daylight_info.valid &&
	    daylight_info.active == (utc_time < daylight_info.change)) {
		local_time += DAYLIGHT_OFFSET;
	}

	return local_time;
}

/*
 * Convert local time to GMT time. Takes into account timezone + DST.
 * During fallback: Maps the 1 am to 2 am of fallback to the 2nd 1am to 2am's
 * UTC if skip_fb_or_spring_fwd = 1. Otherwise, it uses the 1st 1am to 2am's
 * UTC.
 * During spring forward: Maps the 2 am to 3 am to 1:59:59 if
 * skip_fb_or_spring_fwd = 0. If it equals 1, it maps to the new 3 am.
 * If anything else, it returns the normal utc time conversion of that time.
 */
u32 clock_local_to_utc(u32 local, u8 skip_fb_or_spring_fwd)
{
	u32 utc_time;

	if (!timezone_info.valid) {
		return local;
	}
	utc_time = local + timezone_info.mins * 60;

	if (local == MAX_U32 ||
	    (timezone_info.mins > 0 &&
		MAX_U32 - local < timezone_info.mins * 60)) {
		return MAX_U32;
	}
	if (!daylight_info.valid) {
		return utc_time;
	}
	if (daylight_info.active) {
		if (utc_time < daylight_info.change) {
			utc_time -= DAYLIGHT_OFFSET;
		}
		/* For fallback */
		if (!skip_fb_or_spring_fwd &&
		    (utc_time >= daylight_info.change &&
		    utc_time < daylight_info.change + DAYLIGHT_OFFSET)) {
			utc_time -= DAYLIGHT_OFFSET;
		}
		return utc_time;
	}
	/* For Spring Forward */
	if (utc_time >= daylight_info.change &&
	    utc_time < daylight_info.change + DAYLIGHT_OFFSET) {
		if (skip_fb_or_spring_fwd == 2) {
			return utc_time;
		} else if (skip_fb_or_spring_fwd == 1) {
			/* Anytime between 2pm to 3pm local, return 3 pm UTC */
			return daylight_info.change;
		} else if (!skip_fb_or_spring_fwd) {
			return daylight_info.change - 1;
		}
	}
	if (utc_time >= daylight_info.change) {
		utc_time -= DAYLIGHT_OFFSET;
	}

	return utc_time;
}

#ifndef AYLA_TIME
void clock_delay(u32 msecs)
{
	u32 now = clock_ms();

	while (clock_ms() < now + msecs) {
		;
	}
}
#endif
