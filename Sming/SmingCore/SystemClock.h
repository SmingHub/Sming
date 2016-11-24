/** @defgroup   systemclock System clock functions
 *  @ingroup    datetime
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
	eTZ_UTC		= 0, ///< Use universal time coordinate (UTC)
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
	DateTime now(TimeZone timeType = eTZ_Local);

    /** @brief  Set the system clock's time
     *  @param  time Unix time to set clock to (quantity of seconds since 00:00:00 1970-01-01)
     *  @param  timeType Time zone of Unix time, i.e. is time provided as local or UTC?
     *  @note   System time is always stored as local timezone time
     */
	void setTime(time_t time, TimeZone timeType = eTZ_Local);

    /** @brief  Get current time as a string
     *  @param  timeType Time zone to present time as, i.e. return local or UTC time
     *  @retval String Current time in format: dd.mm.yy hh:mm:ss
     *  @note   Date separator may be changed by adding #define DT_DATE_SEPARATOR "/" to source code
     */
	String getSystemTimeString(TimeZone timeType = eTZ_Local);

    /** @brief  Sets the local time zone offset
     *  @param  localTimezoneOffset Offset from UTC of local time zone in hours (-12..+12)
     *  @retval bool True on success
     *  @todo   Why does this need to be set to 2 for UK during winter?
     *  @note   Supports whole hour and fraction of hour offsets from -12 hours to +12 hours
     */
	bool setTimeZone(double localTimezoneOffset);


private:
	double timezoneDiff = 0.0;
	DateTime dateTime;
	SystemClockStatus status = eSCS_Initial;
};

/**	@brief	Global instance of system clock object
 *	@note	Use SystemClock.<i>function</i> to access system clock functions
 *	@note	Example:
 *  @code   SystemClock.setTimeZone(+9.5); //Darwin, Australia
 *	@endcode
*/
extern SystemClockClass SystemClock;

/** @} */
#endif /* APP_SYSTEMCLOCK_H_ */
