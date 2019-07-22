/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * RTC.h
 *
 ****/

/**	@defgroup rtc Real Time Clock
 *	@brief	Access to the real time clock
 *	@note   Provides ability to set and read the ESP8266 RTC.
 *  @ingroup datetime
*/
#pragma once

#include "WiringFrameworkDependencies.h"

/** @brief  Real time clock class
 *  @addtogroup rtc
 *  @{
 */
class RtcClass
{
public:
	/** @brief  Instantiate real time clock object
     */
	RtcClass();

	/** @brief  Get nanoseconds from RTC
     *  @retval uint64_t Quantity of nanoseconds since last RTC reset or set
     */
	uint64_t getRtcNanoseconds();

	/** @brief  Get seconds from RTC
     *  @retval uint32_t Quantity of seconds since epoch
     *  @note   Also updates RTC NVRAM
     */
	uint32_t getRtcSeconds();

	/** @brief  Set RTC nanoseconds
	 *  @param  nanoseconds Value to set RTC to
	 *  @retval bool True on success
	 */
	bool setRtcNanoseconds(uint64_t nanoseconds);

	/** @brief  Set RTC
     *  @param  seconds Quantity of seconds since epoch
     *  @retval bool True on success
     *  @note   Updates RTC NVRAM
     */
	bool setRtcSeconds(uint32_t seconds);

	/** @} */
};

/**	@brief	Global instance of real time clock object
 *	@note	Use RTC.<i>function</i> to access real time clock functions
 *	@note	Example:
 *	@code	RTC.setRtcSeconds(1454225610); //set time to 07:33:30 2016-31-01
 *	@endcode
 *  @ingroup rtc
 */
extern RtcClass RTC;

/** @} */
