/**	@defgroup rtc Real Time Clock
 *	@brief	Access to the real time clock
 *	@note   Provides ability to set and read the ESP8266 RTC.
 *  @ingroup datetime
*/
#ifndef SMINGCORE_RTC_H_
#define SMINGCORE_RTC_H_

#include "../Wiring/WiringFrameworkDependencies.h"

#define RTC_MAGIC 0x55aaaa55
#define RTC_DES_ADDR 64
#define NS_PER_SECOND 1000000000

/** @brief  Structure to hold RTC data
 *  @addtogroup structures
 */
typedef struct {
	uint64_t time; ///< Quantity of nanoseconds since epoch
	uint32_t magic; ///< Magic ID used to identify that RTC has been initialised
	uint32_t cycles; ///< Quantity of RTC cycles since last update
} RtcData;

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

private:
	bool hardwareReset;
	bool saveTime(RtcData &data);
	void updateTime(RtcData &data);
	void loadTime(RtcData &data);
};

/**	@brief	Global instance of real time clock object
 *	@note	Use RTC.<i>function</i> to access real time clock functions
 *	@note	Example:
 *	@code	RTC.setRtcSeconds(1454225610); //set time to 07:33:30 2016-31-01
 *	@endcode
 *  @ingroup rtc
 */
extern RtcClass RTC;
#endif /* SMINGCORE_RTC_H_ */
