/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Timezone.cpp
 *
 * Original code (c) Jack Christensen Mar 2012
 * Arduino Timezone Library Copyright (C) 2018 by Jack Christensen and
 * licensed under GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html
 *
 * @author mikee47 <mike@sillyhouse.net>
 * July 2018 Ported to Sming
 *
 ****/

#include "include/Timezone.h"

static unsigned year(time_t t)
{
	return DateTime(t).Year;
}

time_t Timezone::toLocal(time_t utc, const TimeChangeRule** p_tcr)
{
	// recalculate the time change points if needed
	auto y = year(utc);
	if(y != year(dstStartUTC)) {
		calcTimeChanges(y);
	}

	const TimeChangeRule& tcr = utcIsDST(utc) ? dstRule : stdRule;

	if(p_tcr) {
		*p_tcr = &tcr;
	}

	return utc + (tcr.offset * SECS_PER_MIN);
}

time_t Timezone::toUTC(time_t local)
{
	// recalculate the time change points if needed
	auto y = year(local);
	if(y != year(dstStartLoc)) {
		calcTimeChanges(y);
	}

	return local - (locIsDST(local) ? dstRule.offset : stdRule.offset) * SECS_PER_MIN;
}

bool Timezone::utcIsDST(time_t utc)
{
	// recalculate the time change points if needed
	auto y = year(utc);
	if(y != year(dstStartUTC)) {
		calcTimeChanges(y);
	}

	// daylight time not observed in this tz
	if(stdStartUTC == dstStartUTC) {
		return false;
	}

	// northern hemisphere
	if(stdStartUTC > dstStartUTC) {
		return (utc >= dstStartUTC) && (utc < stdStartUTC);
	}

	// southern hemisphere
	return (utc >= dstStartUTC) || (utc < stdStartUTC);
}

bool Timezone::locIsDST(time_t local)
{
	// recalculate the time change points if needed
	auto y = year(local);
	if(y != year(dstStartLoc)) {
		calcTimeChanges(y);
	}

	// daylight time not observed in this tz
	if(stdStartUTC == dstStartUTC) {
		return false;
	}

	// northern hemisphere
	if(stdStartLoc > dstStartLoc) {
		return (local >= dstStartLoc && local < stdStartLoc);
	}

	// southern hemisphere
	return (local >= dstStartLoc) || (local < stdStartLoc);
}

void Timezone::calcTimeChanges(unsigned yr)
{
	dstStartLoc = toTime_t(dstRule, yr);
	stdStartLoc = toTime_t(stdRule, yr);
	dstStartUTC = dstStartLoc - stdRule.offset * SECS_PER_MIN;
	stdStartUTC = stdStartLoc - dstRule.offset * SECS_PER_MIN;
}

time_t Timezone::toTime_t(TimeChangeRule r, unsigned yr)
{
	// working copies of r.month and r.week which we may adjust
	uint8_t m = r.month;
	uint8_t w = r.week;

	// is this a "Last week" rule?
	if(w == week_t::Last) {
		// yes, for "Last", go to the next month
		if(++m > month_t::Dec) {
			m = month_t::Jan;
			++yr;
		}
		// and treat as first week of next month, subtract 7 days later
		w = week_t::First;
	}

	// calculate first day of the month, or for "Last" rules, first day of the next month
	DateTime dt;
	dt.Hour = r.hour;
	dt.Minute = 0;
	dt.Second = 0;
	dt.Day = 1;
	dt.Month = m - month_t::Jan; // Zero-based
	dt.Year = yr;
	time_t t = dt;

	// add offset from the first of the month to r.dow, and offset for the given week
	t += ((r.dow - dayOfWeek(t) + 7) % 7 + (w - 1) * 7) * SECS_PER_DAY;
	// back up a week if this is a "Last" rule
	if(r.week == 0) {
		t -= 7 * SECS_PER_DAY;
	}

	return t;
}

void Timezone::init(const TimeChangeRule& dstStart, const TimeChangeRule& stdStart)
{
	dstRule = dstStart;
	stdRule = stdStart;
	// force calcTimeChanges() at next conversion call
	dstStartUTC = 0;
	stdStartUTC = 0;
	dstStartLoc = 0;
	stdStartLoc = 0;
}
