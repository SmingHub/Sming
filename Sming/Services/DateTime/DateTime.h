/*
  DateTime.h - Arduino library for date and time functions
  Copyright (c) Michael Margolis.  All right reserved.


  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

  ///
  Partially rewritten for Sming Framework
*/

/** @defgroup datetime Date and time functions
 *  @{
 */
#ifndef _DateTime_h
#define _DateTime_h

#include <time.h>
#include "WString.h"

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
/** Get just seconds part of given Unix time */
#define numberOfSeconds(_time_) (_time_ % SECS_PER_MIN)
/** Get just minutes part of given Unix time */
#define numberOfMinutes(_time_) ((_time_ / SECS_PER_MIN) % SECS_PER_MIN)
/** Get just hours part of given Unix time */
#define numberOfHours(_time_) (( _time_% SECS_PER_DAY) / SECS_PER_HOUR)
/** Get day of week from given Unix time */
#define dayOfWeek(_time_)  (( _time_ / SECS_PER_DAY + 4)  % DAYS_PER_WEEK) // 0 = Sunday
/** Get elapsed days since 1970-01-01 from given Unix time */
#define elapsedDays(_time_) ( _time_ / SECS_PER_DAY)  // this is number of days since Jan 1 1970
/** Get quantity of seconds since midnight from given Unix time */
#define elapsedSecsToday(_time_)  (_time_ % SECS_PER_DAY)   // the number of seconds since last midnight
/** Get Unix time of midnight at start of day from given Unix time */
#define previousMidnight(_time_) (( _time_ / SECS_PER_DAY) * SECS_PER_DAY)  // time at the start of the given day
/** Get Unix time of midnight at end of day from given just Unix time */
#define nextMidnight(_time_) ( previousMidnight(_time_)  + SECS_PER_DAY ) // time at the end of the given day
/** Get quantity of seconds since midnight at start of previous Sunday from given Unix time */
#define elapsedSecsThisWeek(_time_)  (elapsedSecsToday(_time_) +  (dayOfWeek(_time_) * SECS_PER_DAY) )

// todo add date math macros
/*============================================================================*/

/** @brief  Days of week
*/
typedef enum {
	  dtSunday, ///< Sunday
	  dtMonday, ///< Monday
	  dtTuesday, ///< Tuesday
	  dtWednesday, ///< Wednesday
	  dtThursday, ///< Thursday
	  dtFriday, ///< Friday
	  dtSaturday ///< Saturday
} dtDays_t;

/** @brief  Date and time class
 *
 *          Date and time functions mostly work with Unix time, the quantity of seconds since 00:00:00 1970-01-01.
 *          There is no support for leap seconds which are added (and in theory, removed) occasionally to compensate for earth rotation variation.
 *          This means that timespan calculation and free-running clocks may be inaccurate if they span leap seconds.
 *          To facilitate leap seconds, reference must be made to leap second table. This will not be done within the Sming framework and must be handled by application code if required.
 *  @note   Sming uses 32-bit signed integer for its time_t data type which supports a range of +/-68 years. This means Sming is susceptible to Year 2038 problem.
 */
class DateTime
{
public:
    /** @brief  Instantiate an uninitialised date and time object
     */
	DateTime()
	{
	}

    /** @brief  Instantiate a date and time object
     *  @param  time Unix time to assign to object
     */
	DateTime(time_t time);

    /** @brief  Get current Unix time
     *  @retval time_t Quantity of seconds since 00:00:00 1970-01-01
    */
	operator time_t() { return toUnixTime(); }

    /** @brief  Set time using Unix time
     *  @param  time Unix time to set object time to
     */
	void setTime(time_t time);

	/** @brief  Set time using time and date component values
	 *  @param  sec Seconds
	 *  @param  min Minutes
	 *  @param  day Day of month
	 *  @param  month Month (0=Jan, 11=Dec)
	 *  @param  year Year
	 */
	void setTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year);

    /** @brief  Parse a HTTP full date and set time and date
     *  @param  httpDate HTTP full date in RFC 1123 format, e.g. Sun, 06 Nov 1994 08:49:37 GMT
     *  @retval bool True on success
     *  @note   Also supports obsolete RFC 850 date format, e.g. Sunday, 06-Nov-94 08:49:37 GMT where 2 digit year represents range 1970-2069
     *  @note   GMT suffix is optional and is always assumed / ignored
     */
	bool parseHttpDate(const String& httpDate);

	/** @brief  Check if time date object is initialised
	 *  @retval True if object has no value. False if initialised.
	 */
	bool isNull();

    /** @brief  Get Unix time
     *  @retval time_t Unix time, quantity of seconds since 00:00:00 1970-01-01
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
     */
	time_t toUnixTime();

    /** @brief  Get human readable date
     *  @retval String Date in requested format, e.g. dd.mm.yyyy
     */
	String toShortDateString();

	/** @brief  Get human readable time
	 *  @param  includeSeconds True to include seconds (Default: false)
	 *  @retval String Time in format hh:mm or hh:mm:ss
	 */
	String toShortTimeString(bool includeSeconds = false);

    /** @brief  Get human readable date and time
     *  @retval String Date and time in format dd.mm.yyyy hh:mm:ss
     */
	String toFullDateTimeString();
	String toISO8601();

    /** @brief  Add time to date time object
     *  @param  add Quantity of milliseconds to add to object
     */
	void addMilliseconds(long add);

	// functions to convert to and from time components (hrs, secs, days, years etc) to time_t
	/** @brief  Convert from Unix time to individual time components
	 *  @param  timep Unix time date value to convert
	 *  @param  psec Pointer to integer to hold resulting seconds
	 *  @param  pmin Pointer to integer to hold resulting minutes
	 *  @param  phour Pointer to integer to hold resulting hour
	 *  @param  pday Pointer to integer to hold resulting day of month
	 *  @param  pwday Pointer to integer to hold resulting day of week
	 *  @param  pmonth Pointer to integer to hold resulting month
	 *  @param  pyear Pointer to integer to hold resulting year
	 *  @note   This is a more compact version of the C library localtime function
	 *  @note   Pass the Unix timedate value and pointers to existing integers. The integers are updated with the converted values
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. DateTime::convertFromUnixTime(...);
	 *  @note   32-bit Unix time has year 2036 issue.
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
	 *  @note   All of the return values are optional, specify nullptr if not required
	 */
	static void convertFromUnixTime(time_t timep, int8_t *psec, int8_t *pmin, int8_t *phour, int8_t *pday, int8_t *pwday, int8_t *pmonth, int16_t *pyear);

	/** @brief  Convert from individual time components to Unix time
	 *  @param  sec Seconds
	 *  @param  min Minutes
	 *  @param  hour Hours
	 *  @param  day Days
	 *  @param  month Month (0-11, Jan=0, Feb=1, ...Dec=11)
	 *  @param  year Year (1901-2036), either full 4 digit year or 2 digits for 1970-2036
	 *  @note   Seconds, minutes, hours and days may be any value, e.g. to calculate the value for 300 days since 1970 (epoch), set day=300
	 *  @note   This static function  may be used without instantiating a DateTime object, e.g. time_t unixTime = DateTime::convertToUnixTime(...);
	 *  @note   32-bit Unix time is valid between 1901-12-13 and 03:14:07 2038-01-19
	 *  @note   Unix time does not account for leap seconds. To convert Unix time to UTC requires reference to a leap second table.
	 */
	static time_t convertToUnixTime(int8_t sec, int8_t min, int8_t hour, int8_t day, int8_t month, int16_t year);

public:
	int8_t Hour = 0; ///< Hour (0-23)
	int8_t Minute = 0; ///< Minute (0-59)
	int8_t Second = 0; ///< Second (0-59)
	int16_t Milliseconds = 0; ///< Milliseconds (0-999)
	int8_t Day = 0; ///< Day of month (1-31)
	int8_t DayofWeek = 0; ///< Day of week (0-6 Sunday is day 0)
	int8_t Month = 0; ///< Month (0-11 Jan is month 0)
	int16_t Year = 0;  ///< Full Year number
};

/** @} */
#endif /* DateTime_h */

