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

/*
 * Schedule analyzer and interpreter.
 */
#include <string.h>
#ifdef AYLA_SCHED		/* for host mcu build */
#include <ayla/mcu_platform.h>
#include <mcu_io.h>
#include <ayla/ayla_proto_mcu.h>
#include <ayla/props.h>
#include <ayla/clock.h>
#include <ayla/conf_token.h>
#include <ayla/schedeval.h>
#include <ayla/sched.h>
#else
#include <stdio.h>
#include <ayla/utypes.h>
#include <ayla/tlv.h>
#include <ayla/clock.h>
#include <ayla/log.h>
#include "schedeval.h"
#include "sched.h"
#endif


#define SCHED_LIB_VERSION 1
#define SCHED_LAST_OCCUR 0x80

/*
 * Initializes the schedule with defaults
 */
static void sched_init_schedule(struct schedule *sched)
{
	memset(sched, 0, sizeof(*sched));
	sched->days_of_month = ~0;
	sched->months_of_year = ~0;
	sched->days_of_week = ~0;
	sched->day_occur_in_month = ~0;
	sched->end_time_each_day = ~0;
	sched->start_time_each_day = ~0;
}

/*
 * Checks and sets the value of the TLV into the schedule object
 */
static int sched_set_sched_value(void *dest_ptr, u8 dest_len,
			struct ayla_tlv *atlv)
{
	long value;

	if (atlv->len > dest_len) {
#ifndef DEMO_SCHED_LIB
		SCHED_LOGF(LOG_WARN, "not enough space");
#endif
		return -1;
	}
	if (sched_int_get(atlv, &value)) {
#ifndef DEMO_SCHED_LIB
		SCHED_LOGF(LOG_WARN, "len/val err");
#endif
		return -1;
	}
	memcpy(dest_ptr, &value, dest_len);
	return 0;
}

/*
 * Checks if the mask_to_check bit has been set in valid_mask
 */
static int sched_check_mask(u16 valid_mask, u8 mask_to_check)
{
	return valid_mask & (1 << (mask_to_check - 1));
}

/*
 * Checks if the day in clk is valid according to the requirements
 * of the schedule. Returns 1 if clk satisfies the day reqs.
 */
static int sched_check_day(const struct schedule *schedule,
		const struct clock_info *clk)
{
	u8 dayofwk = clk->day_of_week;
	u8 day_occur = 1 << (clk->day_occur_in_month - 1);
	u32 day_of_month = 1 << (clk->days - 1);

	if (dayofwk == 7) {
		/* dayofwk = 7 means a sunday */
		dayofwk = 0;
	}
	dayofwk = 1 << dayofwk;

	if ((schedule->days_of_month >> 31) && !clk->days_left_in_month) {
		/* special case: 0x80000000 means last day of the month */
		day_of_month = MAX_U32;
	}

	if ((schedule->day_occur_in_month & SCHED_LAST_OCCUR) &&
	    clk->days_left_in_month < 7) {
		/* special case: 0x80 means last occurence in month */
		day_occur = SCHED_LAST_OCCUR;
	}

	return (schedule->days_of_week & dayofwk) &&
	    (schedule->day_occur_in_month & day_occur) &&
	    (schedule->days_of_month & day_of_month);
}

static int sched_day_spec_is_given(const struct schedule *schedule)
{
	u8 daysofwk = schedule->days_of_week;
	u8 dayoccur = schedule->day_occur_in_month;
	u32 daysofmon = schedule->days_of_month;

	if ((u8)~daysofwk || (u8)~dayoccur || ~daysofmon) {
		return 1;
	}
	return 0;
}

/*
 * If clk is on a valid day, start will go back to when this range
 * began and end will go to when the range ends.
 * If clk is not on a valid day, start will be the first valid day
 * from min limit and end will be the end of that range.
 * Both start and end are bound by min_limit and max_limit.
 */
static void sched_day_find_range(const struct clock_info *clk,
	const struct schedule *schedule, u32 min_limit, u32 max_limit,
	u32 *start, u32 *end)
{
	struct clock_info bound = *clk; /* struct copy */

	if (!sched_day_spec_is_given(schedule)) {
		/* if neither of the day-related fields are set in schedule */
		*start = min_limit;
		*end = max_limit;
		return;
	}
	if (sched_check_day(schedule, &bound)) {
		if (min_limit >= bound.day_start) {
			*start = min_limit;
			goto find_end;
		}
		do {
			clock_decr_day(&bound);
		} while (min_limit <= bound.day_start &&
		    sched_check_day(schedule, &bound) &&
		    bound.day_start);
		*start = (min_limit >= (bound.day_end + 1)  ?
		    min_limit : (bound.day_end + 1));
		bound = *clk;
		goto find_end;
	}
	clock_fill_details(&bound, min_limit);
	while (!sched_check_day(schedule, &bound) &&
	    bound.day_end <= max_limit &&
	    bound.day_end != MAX_U32) {
		clock_incr_day(&bound);
	}
	if (!sched_check_day(schedule, &bound)) {
		/* found no valid future range */
		*start = 0;
		*end = 0;
		return;
	}
	*start = (min_limit > bound.day_start ? min_limit : bound.day_start);
find_end:
	while (bound.day_start <= max_limit &&
	    sched_check_day(schedule, &bound) &&
	    bound.day_end != MAX_U32) {
		clock_incr_day(&bound);
	}
	*end = (max_limit < bound.day_start) ?
	    max_limit : bound.day_start;
}

/*
 * If clk is on a valid month, start will go back to when this range
 * began and end will go to when the range ends.
 * If clk is not on a valid month, start will be the first valid month
 * from min limit and end will be the end of that range.
 * Both start and end are bound by min_limit and max_limit.
 */
static void sched_month_find_range(const struct clock_info *clk,
	const struct schedule *sched, u32 min_limit, u32 max_limit,
	u32 *start, u32 *end)
{
	struct clock_info bound = *clk; /* struct copy */
	u16 valid_months = sched->months_of_year;

	if (!(u16)~valid_months) {
		*start = min_limit;
		*end = max_limit;
		return;
	}
	if (sched_check_mask(valid_months, bound.month)) {
		if (min_limit >= bound.month_start) {
			*start = min_limit;
			goto find_end;
		}
		do {
			clock_decr_month(&bound);
		} while (bound.month_start >= min_limit &&
		    sched_check_mask(valid_months, bound.month) &&
		    bound.month_start);
		*start = (min_limit > (bound.month_end + 1)  ?
		    min_limit : (bound.month_end + 1));
		bound = *clk;
		goto find_end;
	}
	clock_fill_details(&bound, min_limit);
	while (!sched_check_mask(valid_months, bound.month) &&
	    bound.month_end <= max_limit &&
	    bound.month_end != MAX_U32) {
		clock_incr_month(&bound);
	}
	if (!sched_check_mask(valid_months, bound.month)) {
		/* found no valid future range */
		*start = 0;
		*end = 0;
		return;
	}
	*start = (min_limit > bound.month_start ?
	    min_limit : bound.month_start);
find_end:
	while (bound.month_start <= max_limit &&
	    sched_check_mask(valid_months, bound.month) &&
	    bound.month_end != MAX_U32) {
		clock_incr_month(&bound);
	}
	*end = (max_limit < bound.month_start - 1) ?
	    max_limit : bound.month_start - 1;
}

/*
 * Adjusts tmax if the interval lasts across a SF boundary.
 */
static void sched_adjust_boundary_if_crossover(u32 *tmin, u32 *tmax,
					u32 duration)
{
	u32 tmp1;
	u32 tmp2;

	tmp1 = clock_local(&daylight_info.change);
	tmp2 = tmp1 - DAYLIGHT_OFFSET;
	if (*tmin >= tmp2 && *tmin < tmp1) {
		*tmax = tmp1 + duration;
	} else if (*tmin <= tmp2 && *tmax >= tmp2) {
		if (daylight_info.active) {
			*tmax -= DAYLIGHT_OFFSET;
		} else {
			*tmax += DAYLIGHT_OFFSET;
		}
	}
}

/*
 * Given a schedule, and the current time, this function will:
 * Case A: Time falls within the schedule interval. range_start
 * is set to the begininng of the interval and range_end is the end.
 * Case B: Time is before the scheduled interval. range_start
 * will be set to the beginning of the next interval.
 * Case C: No more events in the schedule after time. range_start
 * and range_end will both be set to 0.
 * This function will give the overall range that the time falls in.
 * It does not take into account intervals.
 */
static int sched_determine_big_range(const struct schedule *schedule,
		u32 time, u32 *range_start, u32 *range_end, u8 toplevel)
{
	struct clock_info clk;
	struct clock_info bound;
	u32 tmin = 1;
	u32 tmax = MAX_U32;
	u32 start;
	u32 end;
	u32 tmp1;
	u32 tmp2;
	u8 time_carryover = ~schedule->end_time_each_day &&
	    ~schedule->start_time_each_day &&
	    schedule->end_time_each_day < schedule->start_time_each_day;
	u32 duration = schedule->duration;
	u32 max_tmax = MAX_U32;
	u8 in_range = 1;
	u8 duration_no_interval = schedule->duration && !schedule->interval;

	/*
	 * If start date is specified, set tmin to the start of that day.
	 * Same with end_date.
	 */
	if (schedule->start_date) {
		clock_fill_details(&bound, schedule->start_date);
		if (bound.day_start > tmin) {
			tmin = bound.day_start;
		}
	}
	if (schedule->end_date) {
		clock_fill_details(&bound, schedule->end_date);
		if (bound.day_end < tmax) {
			tmax = bound.day_end;
		}
		max_tmax = tmax;
	}

	/*
	 * If the range is empty, no more to do.
	 */
	if (tmin >= tmax) {
		goto no_more_events;
	}

	/*
	 * If we're at the end time, determine start of the range.
	 */
	if (time == tmax) {
		/*
		 * right on end date, we need to go through and figure
		 * out the start of this interval. can't just goto done.
		 * I believe finding the range of time-1 should take care
		 * of this.
		 */
		time--;
	} else if (time > tmax) {
		if (toplevel) {
			clock_fill_details(&clk, time);
			goto check_carryover;
		}
		goto no_more_events;
	} else if (time < tmin) {
		in_range = 0;
	}

	/*
	 * If not in range, before schedule start, find next range start.
	 */
	if (!in_range) {
find_next_event:
		if (tmin >= tmax) {
			goto no_more_events;
		}

		/*
		 * Find next valid month after tmin.
		 */
		clock_fill_details(&clk, tmin);
		sched_month_find_range(&clk, schedule, tmin, tmax, &start,
		    &end);
		if (!start) {
			goto no_more_events;
		}

		/*
		 * Find next valid day after start.
		 */
		tmin = start;
		tmax = end;
		clock_fill_details(&clk, tmin);
		sched_day_find_range(&clk, schedule, tmin, tmax,
		    &start, &end);
		if (!start) {
			tmin = tmax + 1;
			tmax = max_tmax;
			goto find_next_event;
		}
		tmin = start;
		tmax = end;

		/*
		 * If start time each day is specified, set tmin to that.
		 * But don't reduce tmin.
		 */
set_start_time_each_day:
		if (~schedule->start_time_each_day) {
			clock_fill_details(&clk, tmin);
			tmp1 = clk.day_start + schedule->start_time_each_day;
			if (tmp1 > tmin) {
				tmin = tmp1;
			}
		}

		/*
		 * If a duration is given without an interval, that's the
		 * big range duration.  Set tmax with that.
		 * end_time_each_day may not also be specified.
		 */
		if (duration_no_interval) {
			if (~schedule->end_time_each_day) {
#ifndef DEMO_SCHED_LIB
				SCHED_LOGF(LOG_WARN, "sched duration error");
#endif
				goto no_more_events;
			}
			tmax = tmin + duration;

			/*
			 * If tmax is in the past, the whole range is in past.
			 * Advance tmin to next day.
			 */
			if (time > tmax) {
				goto next_day;
			}
			/* Check durations that cross SF daylight boundaries */
			if (!schedule->is_utc && daylight_info.valid &&
			    !daylight_info.active) {
				sched_adjust_boundary_if_crossover(&tmin, &tmax,
				    duration);
			}
		}
		goto done;
	}

	/*
	 * In range of dates covered by schedule.  Find next day range.
	 */
	clock_fill_details(&clk, time);
	if (!sched_check_mask(schedule->months_of_year, clk.month)) {
		if (toplevel) {
			goto check_carryover;
		}
		goto no_more_events;
	}

	/*
	 * Get the start/end values for the current interval based on
	 * month range.
	 */
	sched_month_find_range(&clk, schedule, tmin, tmax, &start, &end);
	if (!start) {
		goto no_more_events;
	}
	tmin = start;
	tmax = end;

	if (!sched_day_spec_is_given(schedule) && schedule->interval) {
		/* if neither of the day-related fields are set in an interval
		 * schedule. i.e. Every other day in the month of June 2013 */
		if (~schedule->end_time_each_day) {
			/*
			 * end time specified.
			 */
			goto check_time;
		}
		clock_fill_details(&clk, tmin);
		if (~schedule->start_time_each_day) {
			tmin = clk.day_start + schedule->start_time_each_day;
		}
		if (time > tmax) {
			goto next_day;
		}
		if (time < tmin) {
			in_range = 0;
		}
		goto check_duration;
	}

	/*
	 * Day is specified, check it.
	 */
	if (!sched_check_day(schedule, &clk)) {
		if (toplevel) {
			goto check_carryover;
		}
		goto no_more_events;
	}
	/*
	 * Get the start/end values for the current interval based on
	 * day range.
	 */
	sched_day_find_range(&clk, schedule, tmin, tmax, &start, &end);
	if (!start) {
		in_range = 0;
		tmin = tmax + 1;
		tmax = max_tmax;
		goto find_next_event;
	}
	tmin = start;
	tmax = end;
	if (tmin > time) {
		in_range = 0;
		goto set_start_time_each_day;
	}
check_time:
	if (~schedule->start_time_each_day || ~schedule->end_time_each_day) {
		tmp1 = tmin;
		tmp2 = tmax;
		if (~schedule->start_time_each_day) {
			if (~schedule->end_time_each_day ||
			    duration_no_interval) {
				tmp1 = clk.day_start +
				    schedule->start_time_each_day;
			} else {
				clock_fill_details(&bound, tmin);
				tmp1 = bound.day_start +
				    schedule->start_time_each_day;
			}
		}
		if (~schedule->end_time_each_day) {
			if (~schedule->start_time_each_day) {
				tmp2 = clk.day_start +
				    schedule->end_time_each_day;
			} else {
				clock_fill_details(&bound, tmax);
				tmp2 = bound.day_start +
				    schedule->end_time_each_day;
			}
		}
		if (!time_carryover) {
			tmin = tmp1;
			if (tmin > time) {
				in_range = 0;
			}
			if (~schedule->end_time_each_day) {
				if (time <= tmp2) {
					tmax = tmp2;
					goto done;
				}
				goto next_day;
			}
			goto check_duration;
		}
		if (time <= tmp2 && (toplevel || in_range)) {
			goto check_carryover;
		}
		tmin = tmp1;
		tmax = clk.day_end + 1 + schedule->end_time_each_day;
		if (time < tmin) {
			in_range = 0;
		}
		goto done;
	}
check_duration:
	if (duration_no_interval) {
		if (~schedule->end_time_each_day) {
#ifndef DEMO_SCHED_LIB
			SCHED_LOGF(LOG_WARN, "sched duration error");
#endif
			goto no_more_events;
		}
		tmax = tmin + duration;
		/* Check durations that cross SF daylight boundaries */
		if (!schedule->is_utc && daylight_info.valid &&
		    !daylight_info.active) {
			sched_adjust_boundary_if_crossover(&tmin, &tmax,
			    duration);
		}
		if (time > tmax) {
			if (toplevel) {
				goto check_carryover;
			}
			goto next_day;
		}
	}

done:
	*range_start = tmin;
	*range_end = tmax;

	return in_range;

check_carryover:
	if (!time_carryover && !duration) {
		goto next_day;
	}
	clock_fill_details(&clk, time);
	if (time_carryover) {
		clock_decr_day(&clk);
		if (sched_check_mask(schedule->months_of_year, clk.month) &&
		    sched_check_day(schedule, &clk)) {
			if (time <= clk.day_end +
			    schedule->end_time_each_day + 1) {
				tmin = clk.day_start +
				    schedule->start_time_each_day;
				tmax = clk.day_end +
				    schedule->end_time_each_day + 1;
				goto done;
			}
		}
	} else if (duration) {
		if (time - duration >= clk.day_start) {
			/*
			 * check if subtracting duration will
			 * actually change the day. if not,
			 * then there's no way we're in_range.
			 */
			goto next_day;
		}
		clock_fill_details(&clk, time - duration);
		if (!sched_check_mask(schedule->months_of_year, clk.month) ||
		    !sched_check_day(schedule, &clk)) {
			goto next_day;
		}
		sched_determine_range(schedule, time - duration,
		    range_start, range_end, 0);
		if (time <= *range_end) {
			return 0;
		}
		clock_fill_details(&clk, time);
	}
	goto next_day;

no_more_events:
	*range_start = 0;
	*range_end = 0;

	return in_range;

next_day:
	in_range = 0;
	tmin = clk.day_end + 1;
	tmax = max_tmax;
	goto find_next_event;
}

/*
 * Returns the minimum val that's greater than time.
 */
static u32 sched_get_minimum(u32 time, u32 val1, u32 val2)
{
	if (val1 >= time && val2 >= time) {
		return val1 < val2 ? val1 : val2;
	} else if (val1 >= time) {
		return val1;
	} else if (val2 >= time) {
		return val2;
	}

	return 0;
}

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
int sched_determine_range(const struct schedule *schedule, u32 time,
	u32 *range_start, u32 *range_end, u8 toplevel)
{
	struct clock_info clk;
	u8 in_range;
	u32 little_range_start;
	u32 little_range_end;
	u32 real_time = time;
	u32 utc_range_start1;
	u32 utc_range_start2;
	u32 utc_range_end1;
	u32 utc_range_end2;
	u32 orig_range_start;
	u32 orig_range_end;
	u32 tmp;

	/* Get the big picture range first */
	if (!time || time == MAX_U32) {
		goto no_more_events;
	}
	if (toplevel && !schedule->is_utc) {
		real_time = clock_local(&time);
	}
	in_range = sched_determine_big_range(schedule, real_time,
	    &orig_range_start, &orig_range_end, toplevel);

local_to_utc:
	if ((!orig_range_start && !orig_range_end) ||
	    orig_range_start == MAX_U32) {
		goto no_more_events;
	}

	if (toplevel && !schedule->is_utc) {
		if (daylight_info.valid && !daylight_info.active) {
			/* Accounts for spring-forward */
			*range_start = clock_local_to_utc(orig_range_start, 1);
			if (!schedule->interval && schedule->duration) {
				*range_end = clock_local_to_utc(orig_range_end,
				    2);
				if (*range_start >= *range_end) {
					goto find_next_spring_forward;
				}
				if (time < *range_start) {
					in_range = 0;
				}
				goto determine_little_range;
			}
			*range_end = clock_local_to_utc(orig_range_end, 0);
			utc_range_start2 =
			    clock_local_to_utc(orig_range_start, 0);
			if (*range_start > *range_end ||
			    (*range_start != utc_range_start2)) {
				in_range = 0;
				sched_determine_big_range(schedule,
				    *range_start, &orig_range_start,
				    &orig_range_end, toplevel);
				goto local_to_utc;
			} else if (*range_start == *range_end &&
			    orig_range_end - orig_range_start) {
find_next_spring_forward:
				in_range = 0;
				sched_determine_big_range(schedule,
				    *range_start + 1, &orig_range_start,
				    &orig_range_end, toplevel);
				goto local_to_utc;
			}
			goto determine_little_range;
		}
		/* Accounts for fallback */
		utc_range_start1 = clock_local_to_utc(orig_range_start, 0);
		utc_range_start2 = clock_local_to_utc(orig_range_start, 1);
		utc_range_end1 = clock_local_to_utc(orig_range_end, 0);
		utc_range_end2 = clock_local_to_utc(orig_range_end, 1);
		if (!in_range) {
			if (time >= utc_range_start1) {
				in_range = 1;
				goto in_range;
			}
			*range_start = sched_get_minimum(time,
			    utc_range_start1, utc_range_start2);
		} else {
in_range:
			*range_start = utc_range_start1;
		}
		if (!schedule->interval && schedule->duration) {
			/* Account for durations across FB */
			*range_end = *range_start +
			    orig_range_end - orig_range_start;
			if (in_range && time > *range_end) {
				in_range = 0;
				if (!daylight_info.valid ||
				    !daylight_info.active) {
					/* shouldn't happen */
					goto no_more_events;
				}
				real_time = time + DAYLIGHT_OFFSET;
				time = daylight_info.change + DAYLIGHT_OFFSET;
				sched_determine_big_range(schedule, real_time,
				    &orig_range_start, &orig_range_end,
				    toplevel);
				goto local_to_utc;
			}
		} else {
			*range_end = sched_get_minimum(time, utc_range_end1,
			    utc_range_end2);
		}
	} else {
		*range_start = orig_range_start;
		*range_end = orig_range_end;
	}
determine_little_range:
	if ((!*range_start && !*range_end) || *range_start == MAX_U32 ||
	    !in_range || !schedule->interval) {
		return 0;
	}
	/* Need to find the little interval from the big interval */
	tmp = ((time - *range_start) / schedule->interval) * schedule->interval;
	if (tmp >= MAX_U32 - *range_start) {
		goto no_more_events;
	}
	little_range_start = *range_start + tmp;
	if (little_range_start > *range_end) {
		/* should not happen, bad schedule */
		goto no_more_events;
	}
calc_end_and_return:
	if (little_range_start == *range_end && schedule->duration) {
		/*
		 * for instance, every hour on Monday for 20 mins,
		 * we don't want an event to occur at Tuesday midnight
		 */
		goto calc_next_interval;
	}
	/*
	 * its ok if little_range_end is > *range_end.
	 * we just care that the start time falls within the schedule.
	 * unless end_time_each_day is specified in which case the duration
	 * is chopped off.
	 */
	if (schedule->duration > MAX_U32 - little_range_start) {
		little_range_end = MAX_U32;
	} else {
		little_range_end = little_range_start + schedule->duration;
	}
	if (little_range_end > *range_end && ~schedule->end_time_each_day) {
		clock_fill_details(&clk, little_range_end);
		if (clk.secs_since_midnight > schedule->end_time_each_day) {
			little_range_end -= clk.secs_since_midnight -
			    schedule->end_time_each_day;
		}
	}
	if (little_range_end < little_range_start) {
		goto calc_next_interval;
	}
	if (time <= little_range_end) {
		*range_start = little_range_start;
		*range_end = little_range_end;
		return 0;
	}
calc_next_interval:
	if (schedule->interval >= MAX_U32 - little_range_start) {
		goto no_more_events;
	}
	little_range_start += schedule->interval;
	if (little_range_start > *range_end) {
		/* no valid little intervals left in this big interval */
		/* find the next big interval */
		if (!toplevel || orig_range_end == MAX_U32) {
			goto no_more_events;
		}
		in_range = 0;
		real_time = orig_range_end + 1;
		time = *range_end + 1;
		sched_determine_big_range(schedule, real_time,
		    &orig_range_start, &orig_range_end, toplevel);
		goto local_to_utc;
	}
	goto calc_end_and_return;

no_more_events:
	*range_start = 0;
	*range_end = 0;

	return 0;
}

/*
 * Takes a schedtlv structure and fills a schedule structure with
 * the information.
 * Uses the current UTC time to calculate when the next event will
 * occur in UTC time. Returns this value.
 * If it returns 0 or MAX_U32, no more events are set to occur for
 * this schedule.
 */
u32 sched_evaluate(struct sched_prop *schedtlv, u32 time)
{
	u8 *tlvs = &schedtlv->tlvs[0];
	struct ayla_tlv *atlv = (struct ayla_tlv *)(tlvs);
	long value;
	u8 consumed = 0;
	struct schedule sched;
	u8 end_needed = 0;
	u32 range_start = 0;
	u32 range_end = 0;
	u8 range_calculated = 0;
	int err = 0;
	enum ayla_tlv_type type = ATLV_INVALID;

	if (atlv->type != ATLV_VERSION ||
	    sched_int_get(atlv, &value) ||
	    value > SCHED_LIB_VERSION) {
#ifndef DEMO_SCHED_LIB
		SCHED_LOGF(LOG_WARN, "bad sched ver");
#endif
		return 0;
	}
	consumed += atlv->len + sizeof(struct ayla_tlv);
	sched_init_schedule(&sched);
	while (consumed < schedtlv->len) {
		atlv = (struct ayla_tlv *)(tlvs + consumed);
		switch (atlv->type) {
		case ATLV_AND:
			break;
		case ATLV_DISABLE:
			return 0;
		case ATLV_UTC:
			sched.is_utc = 1;
			break;
		case ATLV_STARTDATE:
			err = sched_set_sched_value(&sched.start_date,
			    sizeof(sched.start_date), atlv);
			break;
		case ATLV_ENDDATE:
			err = sched_set_sched_value(&sched.end_date,
			    sizeof(sched.end_date), atlv);
			break;
		case ATLV_DAYSOFMON:
			err = sched_set_sched_value(&sched.days_of_month,
			    sizeof(sched.days_of_month), atlv);
			break;
		case ATLV_DAYSOFWK:
			err = sched_set_sched_value(&sched.days_of_week,
			    sizeof(sched.days_of_week), atlv);
			break;
		case ATLV_DAYOCOFMO:
			err = sched_set_sched_value(&sched.day_occur_in_month,
			    sizeof(sched.day_occur_in_month), atlv);
			break;
		case ATLV_MOOFYR:
			err = sched_set_sched_value(&sched.months_of_year,
			    sizeof(sched.months_of_year), atlv);
			break;
		case ATLV_STTIMEEACHDAY:
			err = sched_set_sched_value(&sched.start_time_each_day,
			    sizeof(sched.start_time_each_day), atlv);
			break;
		case ATLV_ENDTIMEEACHDAY:
			err = sched_set_sched_value(&sched.end_time_each_day,
			    sizeof(sched.end_time_each_day), atlv);
			break;
		case ATLV_DURATION:
			err = sched_set_sched_value(&sched.duration,
			    sizeof(sched.duration), atlv);
			break;
		case ATLV_INTERVAL:
			err = sched_set_sched_value(&sched.interval,
			    sizeof(sched.interval), atlv);
			break;
		case ATLV_ATEND:
			end_needed = 1;
		case ATLV_ATSTART:
		case ATLV_INRANGE:
			type = (enum ayla_tlv_type)atlv->type;
			if (!time || (!sched.is_utc && !timezone_info.valid)) {
#ifndef DEMO_SCHED_LIB
				SCHED_LOGF(LOG_WARN, "curr time not known");
#endif
				return 0;
			}
			if (range_calculated) {
				/* ranges only need to be calculated once */
				break;
			}
			if (sched.interval && sched.duration &&
			    sched.interval <= sched.duration) {
				/* i.e. do something for 15 mins every 5 mins */
				sched.interval += sched.duration;
			}
			if (sched_determine_range(&sched, time, &range_start,
			    &range_end, 1)) {
#ifndef DEMO_SCHED_LIB
				SCHED_LOGF(LOG_WARN, "range calc err");
#endif
				return 0;
			}
			range_calculated = 1;
			break;
		case ATLV_SETPROP:
			if ((type == ATLV_ATSTART && time == range_start) ||
			    (type == ATLV_ATEND && time == range_end) ||
			    (type == ATLV_INRANGE && time >= range_start &&
			    time < range_end)) {
				sched_set_prop(atlv + 1, atlv->len);
			}
			break;
		default:
#ifndef DEMO_SCHED_LIB
			SCHED_LOGF(LOG_WARN, "unknown sched tlv = %x",
			    atlv->type);
#endif
			return 0;
		}
		if (err) {
#ifndef DEMO_SCHED_LIB
			SCHED_LOGF(LOG_WARN, "sched value err");
#endif
			return 0;
		}
		consumed += atlv->len + sizeof(struct ayla_tlv);
	}
	if (!range_start && !range_end) {
		return 0;
	} else if (time < range_start) {
		return range_start;
	}
	if (!end_needed && range_end != MAX_U32) {
		time = range_end + 1;
		goto determine_next_event;
	}
	if (time < range_end) {
		return range_end;
	} else if (time == range_end) {
		/* find the next interval */
		time = range_end + 1;
determine_next_event:
		sched_determine_range(&sched, time,
		    &range_start, &range_end, 1);
		if (range_start && range_start != MAX_U32 &&
		    range_start < time) {
#ifndef DEMO_SCHED_LIB
			SCHED_LOGF(LOG_ERR, "sched_eval err");
#endif
			return 0;
		}
		return range_start;
	}
	return 0;
}
