/** @defgroup   system_clock System clock functions
 *  @brief      Provides system clock functions
*/

#ifndef APP_SYSTEMCLOCK_H_
#define APP_SYSTEMCLOCK_H_

#include "Clock.h"
#include "../../Services/DateTime/DateTime.h"
#include "../../Wiring/WString.h"
#include "../SmingCore/Network/NtpClient.h"
#include "../SmingCore/Platform/RTC.h"

/** @addtogroup constants
 *  @{
 */
/// Time zones
enum TimeZone
{
	eTZ_UTC		= 0, ///< Use universal time coordinate (UTC / GMT)
	eTZ_Local	= 1 ///< Use local time
};

/// System clock status
enum SystemClockStatus
{
	eSCS_Initial	= 0, ///< Clock not yet set
	eSCS_Set		= 1 ///< Clock set
};
/** @} */

class NtpClient;

class SystemClockClass
{
public:
    /** @addtogroup system_clock
     *  @{
     */

    /** @brief  Get the current date and time
     *  @param  timeType Time zone to use
     *  @retval DateTime Current date and time
     */
	DateTime now(TimeZone timeType = eTZ_Local);

    /** @brief  Set the system clock's time
     *  @param  time Time to set clock to
     *  @param  timeType Time zone to use
     */
	void setTime(time_t time, TimeZone timeType = eTZ_Local);

    /** @brief  Get current time as a string
     *  @param  timeType Time zone to use
     *  @retval String Current time in format: dd.mm.yy hh:mm:ss
     *  @todo   Allow time format to be defined
     */
	String getSystemTimeString(TimeZone timeType = eTZ_Local);

    /** @brief  Sets the local time zone offset
     *  @param  localTimezoneOffset Offset from UTC (GMT) of local time zone in hours
     *  @retval bool True on success
     *  @todo   Why does this need to be set to 2 for UK during winter?
     */
	bool setTimeZone(double localTimezoneOffset);

    /** @} */

private:
	double timezoneDiff = 0.0;
	DateTime dateTime;
	SystemClockStatus status = eSCS_Initial;
};

/**	@brief	Global instance of system clock object
 *	@note	Use SystemClock.<i>function</i> to access system clock functions
 *	@note	Example:
 *  @code   SystemClock.setTimeZone(-2);
 *	@endcode
 *	@ingroup system_clock
*/
extern SystemClockClass SystemClock;

#endif /* APP_SYSTEMCLOCK_H_ */
