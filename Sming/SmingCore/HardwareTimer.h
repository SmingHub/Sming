/*
 * HWTimer.h
 *
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 23.11.2015 by johndoe
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/**	@defgroup hwtimer Hardware timer
 *	@brief	Access Hardware timer
 *  @{
 */

#ifndef _SMING_CORE_HWTIMER_H_
#define _SMING_CORE_HWTIMER_H_

#include "../SmingCore/Interrupts.h"
#include "../SmingCore/Delegate.h"

#define MAX_HW_TIMER_INTERVAL_US 0x7fffff ///< Maximum timer interval in microseconds
#define MIN_HW_TIMER_INTERVAL_US 0x32	 ///< Minimum hardware interval in microseconds

/** @brief Convert microseconds into timer ticks.
 *  @note Replaces the previous US_TO_RTC_TIMER_TICKS macro to guarantee we use the correct timer prescale value.
 */
uint32_t IRAM_ATTR usToTimerTicks(uint32_t us);

/** @brief Convert timer ticks into microseconds
 *  @note accounts for current timer prescale setting
 */
uint32_t IRAM_ATTR timerTicksToUs(uint32_t ticks);

/** @brief  Delegate callback type for timer trigger
 */
typedef Delegate<void()> TimerDelegate;

/// Hardware timer class
class HardwareTimer
{
public:
	/** @brief  Hardware timer
    */
	HardwareTimer();
	virtual ~HardwareTimer();

	/** @brief  Initialise hardware timer
     *  @param  microseconds Timer interval in microseconds
     *  @param  callback Callback function to call when timer triggers (Default: none)
     *  @retval HardwareTimer& Reference to timer
     */
	HardwareTimer& IRAM_ATTR initializeUs(uint32_t microseconds,
										  InterruptCallback callback = NULL); // Init in Microseconds.

	/** @brief  Initialise hardware timer
     *  @param  milliseconds Timer interval in milliseconds
     *  @param  callback Callback function to call when timer triggers (Default: none)
     *  @retval HardwareTimer& Reference to timer
     */
	HardwareTimer& IRAM_ATTR initializeMs(uint32_t milliseconds,
										  InterruptCallback callback = NULL); // Init in Milliseconds.

	/** @brief  Start timer running
     *  @param  repeating True to restart timer when it triggers, false for one-shot (Default: true)
     *  @retval bool True if timer started
     */
	bool IRAM_ATTR start(bool repeating = true);

	/** @brief  Start one-shot timer
	 *  @retval bool True if timer started
	 *  @note   Timer starts and will run for configured period then stop
	 */
	bool __forceinline IRAM_ATTR startOnce()
	{
		return start(false);
	}

	/** @brief  Stops timer
	 *  @retval bool Always false
	 *  @todo   Why always return false from Hardware_timer::stop()?
	 */
	bool IRAM_ATTR stop();

	/** @brief  Restart timer
	 *  @retval bool True if timer started
	 *  @note   Timer is stopped then started with current configuration
	 */
	bool IRAM_ATTR restart();

	/** @brief  Check if timer is started
	 *  @retval bool True if started
	 */
	bool isStarted();

	/** @brief  Get timer interval
     *  @retval uint32_t Timer interval in microseconds
     */
	uint32_t getIntervalUs();

	/** @brief  Get timer interval
     *  @retval uint32_t Timer interval in milliseconds
     */
	uint32_t getIntervalMs();

	/** @brief  Set timer interval
     *  @param  microseconds Interval time in microseconds (Default: 1ms)
     */
	bool IRAM_ATTR setIntervalUs(uint32_t microseconds = 1000000);

	/** @brief  Set timer interval
     *  @param  milliseconds Interval time in milliseconds (Default: 1s)
     */
	bool IRAM_ATTR setIntervalMs(uint32_t milliseconds = 1000000);

	/** @brief  Set timer trigger callback
     *  @param  callback Function to call when timer triggers
     */
	void IRAM_ATTR setCallback(InterruptCallback callback);

	/** @brief  Call timer callback
     *  @note   Calls the timer callback function
     */
	void __forceinline IRAM_ATTR call()
	{
		if(callback) {
			callback();
		}
	}

private:
	uint32_t interval = 0;
	InterruptCallback callback = nullptr;
	bool repeating = false;
	bool started = false;
};

/**
 * @deprecated Use HardwareTimer class instead
 */
class Hardware_Timer : public HardwareTimer
{
};

/** @} */
#endif /* _SMING_CORE_HWTIMER_H_ */
