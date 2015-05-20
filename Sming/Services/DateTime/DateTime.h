/*
  DateTime.h - Arduino library for date and time functions
  Copyright (c) Michael Margolis.  All right reserved.


  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 

  ///
  Partially rewritten for Sming Framework
*/

#ifndef _DateTime_h
#define _DateTime_h

#include <user_config.h>
#include <time.h>
#include "../../Wiring/WString.h"

/*==============================================================================*/
/* Useful Constants */
#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)
#define DAYS_PER_WEEK (7L)
#define SECS_PER_WEEK (SECS_PER_DAY * DAYS_PER_WEEK)
#define SECS_PER_YEAR (SECS_PER_WEEK * 52L)
#define SECS_YR_2000  (946681200UL)
 
/* Useful Macros for getting elapsed time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)  
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN) 
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
#define dayOfWeek(_time_)  (( _time_ / SECS_PER_DAY + 4)  % DAYS_PER_WEEK) // 0 = Sunday
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  // this is number of days since Jan 1 1970
#define elapsedSecsToday(_time_)  (_time_ % SECS_PER_DAY)   // the number of seconds since last midnight 
#define previousMidnight(_time_) (( _time_ / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
#define nextMidnight(_time_) ( previousMidnight(_time_)  + SECS_PER_DAY ) // time at the end of the given day 
#define elapsedSecsThisWeek(_time_)  (elapsedSecsToday(_time_) +  (dayOfWeek(_time_) * SECS_PER_DAY) )   

// todo add date math macros
/*============================================================================*/

typedef enum {
	  dtSunday, dtMonday, dtTuesday, dtWednesday, dtThursday, dtFriday, dtSaturday
} dtDays_t;

class DateTime
{
public:
	DateTime();
	DateTime(time_t time);

	operator time_t() { return toUnixTime(); }

	void setTime(time_t time);
	void setTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year);
	bool parseHttpDate(String httpDate);
	bool isNull();

	time_t toUnixTime();
	String toShortDateString();
	String toShortTimeString(bool includeSeconds = false);
	String toFullDateTimeString();

	void addMilliseconds(long add);

	// functions to convert to and from time components (hrs, secs, days, years etc) to time_t
	static void convertFromUnixTime(time_t timep, int8_t *psec, int8_t *pmin, int8_t *phour, int8_t *pday, int8_t *pwday, int8_t *pmonth, int16_t *pyear);
	static time_t convertToUnixTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year); // returns time_t from components

public:
	int8_t Hour;
	int8_t Minute;
	int8_t Second;
	int16_t Milliseconds;
	int8_t Day;
	int8_t DayofWeek; // Sunday is day 0
	int8_t Month; // Jan is month 0
	int16_t Year;  // Full Year numer
};

#endif /* DateTime_h */

