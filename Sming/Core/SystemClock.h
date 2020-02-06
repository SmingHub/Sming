/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SystemClock.h
 *
 ****/

/** @defgroup   systemclock System clock functions
 *  @ingroup    datetime
 *  @brief      Provides system clock functions
*/
#pragma once

#include "DateTime.h"
#include "WString.h"

/** @addtogroup constants
 *  @{
 */
/// Time zones
enum TimeZone {
	eTZ_UTC = 0,  ///< Use universal time coordinate (UTC)
	eTZ_Local = 1 ///< Use local time
};
/** @} */

/** @brief  System clock class
 *  @addtogroup systemclock
 *  @{
 */
class SystemClockClass
{
public:
	/** @brief  Get the current date and time
     *  @param  timeType Time zone to use (UTC / local)
     *  @retval DateTime Current date and time
     */
	time_t now(TimeZone timeType = eTZ_Local) const;

	/** @brief  Set the system clock's time
     *  @param  time Unix time to set clock to (quantity of seconds since 00:00:00 1970-01-01)
     *  @param  timeType Time zone of Unix time, i.e. is time provided as local or UTC?
     *  @note   System time is always stored as UTC time.
     *  If setting using the value retrieved from a time server using NTP, specify eTZ_UTC.
     *  If passing a local value using eTZ_Local, ensure that the time zone has been set correctly
     *  as it will be converted to UTC before storing.
     */
	bool setTime(time_t time, TimeZone timeType);

	/** @brief  Get current time as a string
     *  @param  timeType Time zone to present time as, i.e. return local or UTC time
     *  @retval String Current time in format: `dd.mm.yy hh:mm:ss`
     *  @note   Date separator may be changed by adding `#define DT_DATE_SEPARATOR "/"` to source code
     */
	String getSystemTimeString(TimeZone timeType = eTZ_Local) const;

	/** @brief  Sets the local time zone offset
     *  @param  seconds Offset from UTC of local time zone in seconds (-720 < offset < +720)
     *  @retval bool true on success, false if value out of range
     */
	bool setTimeZoneOffset(int seconds);

	/** @brief Set the local time zone offset in hours
     *  @param localTimezoneOffset Offset from UTC of local time zone in hours (-12.0 < offset < +12.0)
     *  @retval bool true on success, false if value out of range
	 */
	bool setTimeZone(float localTimezoneOffset)
	{
		return setTimeZoneOffset(localTimezoneOffset * SECS_PER_HOUR);
	}

	/** @brief Get the current time zone offset
	 *  @retval int Offset in seconds from UTC
	 */
	int getTimeZoneOffset() const
	{
		return timeZoneOffsetSecs;
	}

	/** @brief Determine if `setTime()` has been called yet
	 *  @note Indicates whether time returned can be relied upon
	 */
	bool isSet() const
	{
		return timeSet;
	}

private:
	int timeZoneOffsetSecs = 0;
	bool timeSet = false;
};

/**	@brief	Global instance of system clock object
 *	@note	Use SystemClock.<i>function</i> to access system clock functions
 *	@note	Example:
 *  @code   SystemClock.setTimeZone(+9.5); //Darwin, Australia
 *	@endcode
*/
extern SystemClockClass SystemClock;

/** @} */
