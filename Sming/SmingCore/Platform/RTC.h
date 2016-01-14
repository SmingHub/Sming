/**	@defgroup rtc Real Time Clock
 *	@brief	Access to the real time clock
 *	@note   Provides ability to set and read the ESP8266 RTC.
*/
#ifndef SMINGCORE_RTC_H_
#define SMINGCORE_RTC_H_

#include "../Wiring/WiringFrameworkDependencies.h"

#define RTC_MAGIC 0x55aaaa55
#define RTC_DES_ADDR 64
#define NS_PER_SECOND 1000000000

/** @brief  Structure to hold RTC data
 *  @ingroup structures
 */
typedef struct {
	uint64_t time; ///< Timestamp
	uint32_t magic; ///< Magic ID @todo What is RtcData.magic?
	uint32_t cycles; ///@todo What is RtcData.cycles?
} RtcData;

class RtcClass
{
public:
    /** @brief  Real time clock class
     *  @addtogroup rtc
     *  @{
     */
	RtcClass();

    /** @brief  Get seconds from RTC
     *  @retval uint32_t Quantity of seconds since last RTC reset or set
     */
	uint32_t getRtcSeconds();

    /** @brief  Set RTC seconds
     *  @param  seconds Value to set RTC seconds to
     *  @retval bool True on success
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
 *	@code	RTC.setRtcSeconds(0);
 *	@endcode
 *  @ingroup rtc
 */
extern RtcClass RTC;
#endif /* SMINGCORE_RTC_H_ */
