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

#include "Interrupts.h"
#include "Delegate.h"

#define MAX_HW_TIMER_INTERVAL_US 0x7fffff ///< Maximum timer interval in microseconds
#define MIN_HW_TIMER_INTERVAL_US 0x32	 ///< Minimum hardware interval in microseconds

/** @brief  Delegate callback type for timer trigger
 */
typedef Delegate<void()> TimerDelegate;

/// Hardware timer class
class Hardware_Timer {
public:
	/** @brief  Hardware timer
    */
	Hardware_Timer();
	virtual ~Hardware_Timer();

	/** @brief  Initialise hardware timer
     *  @param  microseconds Timer interval in microseconds
     *  @param  callback Callback function to call when timer triggers (Default: none)
     *  @retval Hardware_Timer& Reference to timer
     */
	Hardware_Timer& IRAM_ATTR initializeUs(uint32_t microseconds,
										   InterruptCallback callback = nullptr); // Init in Microseconds.

	/** @brief  Initialise hardware timer
     *  @param  milliseconds Timer interval in milliseconds
     *  @param  callback Callback function to call when timer triggers (Default: none)
     *  @retval Hardware_Timer& Reference to timer
     */
	Hardware_Timer& IRAM_ATTR initializeMs(uint32_t milliseconds,
										   InterruptCallback callback = nullptr); // Init in Milliseconds.

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
	bool isStarted()
	{
		return _started;
	}

	/** @brief  Get timer interval
     *  @retval uint32_t Timer interval in microseconds
     */
	uint32_t getIntervalUs()
	{
		return _interval;
	}

	/** @brief  Get timer interval
     *  @retval uint32_t Timer interval in milliseconds
     */
	uint32_t getIntervalMs()
	{
		return getIntervalUs() / 1000;
	}

	/** @brief  Set timer interval
     *  @param  microseconds Interval time in microseconds (Default: 1ms)
     */
	bool IRAM_ATTR setIntervalUs(uint32_t microseconds = 1000000);

	/** @brief  Set timer interval
     *  @param  milliseconds Interval time in milliseconds (Default: 1s)
     */
	bool IRAM_ATTR setIntervalMs(uint32_t milliseconds = 1000000)
	{
		return setIntervalUs(milliseconds * 1000);
	}

	/** @brief  Set timer trigger callback
     *  @param  callback Function to call when timer triggers
     */
	void IRAM_ATTR setCallback(InterruptCallback callback);

	/** @brief  Call timer callback
     *  @note   Calls the timer callback function
     */
	void __forceinline IRAM_ATTR call()
	{
		if (_callback)
			_callback();
	}

private:
	uint32_t _interval = 0;
	InterruptCallback _callback = nullptr;
	bool _repeating = false;
	bool _started = false;
};

/** @} */
#endif /* _SMING_CORE_HWTIMER_H_ */
