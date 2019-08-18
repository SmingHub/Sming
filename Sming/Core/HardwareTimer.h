/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HardwareTimer.h
 *
 * Created 23.11.2015 by johndoe
 *
 ****/

/**	@defgroup hwtimer Hardware timer
 *	@brief	Access Hardware timer
 *  @{
 */

#pragma once

#include "Interrupts.h"
#include <driver/hw_timer.h>
#include <muldiv.h>

// Hardware Timer operating mode
enum HardwareTimerMode {
	eHWT_Maskable,
	eHWT_NonMaskable,
};

/**
 * @brief Hardware timer class
 */
class HardwareTimer
{
public:
	/** @brief Convert microseconds into timer ticks.
	 */
	static uint32_t IRAM_ATTR usToTicks(uint32_t us)
	{
		return muldiv<frequency, 1000000>(us);
	}

	/** @brief Convert timer ticks into microseconds
	 */
	static uint32_t IRAM_ATTR ticksToUs(uint32_t ticks)
	{
		return muldiv<1000000, frequency>(ticks);
	}

	/** @brief  Hardware timer
	 *  @param mode
	 *  @note NMI has highest interrupt priority on system and can therefore occur within
	 *  any other interrupt service routine. Similarly, the NMI service routine cannot
	 *  itself be interrupted. This provides the most stable and reliable timing possible,
	 *  and is therefore the default behaviour.
    */
	HardwareTimer(HardwareTimerMode mode = eHWT_NonMaskable) : mode(mode)
	{
		assert(state == eTS_CallbackNotSet);
	}

	~HardwareTimer()
	{
		stop();
		hw_timer1_detach_interrupt();
		state = eTS_CallbackNotSet;
	}

	/** @brief  Initialise hardware timer
     *  @param  microseconds Timer interval in microseconds
     *  @param  callback Callback function to call when timer triggers (Default: none)
     *  @retval HardwareTimer& Reference to timer
     */
	HardwareTimer& IRAM_ATTR initializeUs(uint32_t microseconds, InterruptCallback callback = nullptr)
	{
		setCallback(callback);
		setIntervalUs(microseconds);
		return *this;
	}

	/** @brief  Initialise hardware timer
     *  @param  milliseconds Timer interval in milliseconds
     *  @param  callback Callback function to call when timer triggers (Default: none)
     *  @retval HardwareTimer& Reference to timer
     */
	HardwareTimer& IRAM_ATTR initializeMs(uint32_t milliseconds, InterruptCallback callback = nullptr)
	{
		setCallback(callback);
		setIntervalMs(milliseconds);
		return *this;
	}

	/** @brief  Start timer running
     *  @param  repeating True to restart timer when it triggers, false for one-shot (Default: true)
     *  @retval bool True if timer started
     */
	bool IRAM_ATTR start(bool repeating = true)
	{
		if(interval == 0 || state == eTS_CallbackNotSet) {
			stop();
			return false;
		}

		hw_timer1_enable(clkdiv, TIMER_EDGE_INT, repeating);
		this->repeating = repeating;
		hw_timer1_write(interval);
		return true;
	}

	/** @brief  Start one-shot timer
	 *  @retval bool True if timer started
	 *  @note   Timer starts and will run for configured period then stop
	 */
	__forceinline bool IRAM_ATTR startOnce()
	{
		return start(false);
	}

	/** @brief  Stops timer
	 */
	void IRAM_ATTR stop()
	{
		if(state == eTS_Armed) {
			hw_timer1_disable();
			state = eTS_Disarmed;
		}
	}

	/** @brief  Restart timer
	 *  @retval bool True if timer started
	 *  @note   Timer is stopped then started with current configuration
	 */
	bool IRAM_ATTR restart()
	{
		stop();
		return start(repeating);
	}

	/** @brief  Check if timer is started
	 *  @retval bool True if started
	 */
	__forceinline bool isStarted()
	{
		return state == eTS_Armed;
	}

	/** @brief  Get timer interval
     *  @retval uint32_t Timer interval in microseconds
     */
	__forceinline uint32_t getIntervalUs()
	{
		return ticksToUs(interval);
	}

	/** @brief  Get timer interval
     *  @retval uint32_t Timer interval in milliseconds
     */
	uint32_t getIntervalMs()
	{
		return getIntervalUs() / 1000;
	}

	/** @brief  Set timer interval in timer ticks
     *  @param  ticks Interval in ticks
     */
	bool IRAM_ATTR setInterval(uint32_t ticks)
	{
		constexpr uint32_t minTicks = MIN_HW_TIMER1_INTERVAL_US * frequency / 1000000;
		if(ticks < MAX_HW_TIMER1_INTERVAL && ticks > minTicks) {
			interval = ticks;
			if(state == eTS_Armed) {
				return restart();
			}
		} else {
			stop();
		}
		return false;
	}

	/** @brief  Set timer interval
     *  @param  microseconds Interval time in microseconds (Default: 1ms)
     */
	__forceinline bool IRAM_ATTR setIntervalUs(uint32_t microseconds)
	{
		return setInterval(usToTicks(microseconds));
	}

	/** @brief  Set timer interval
     *  @param  milliseconds Interval time in milliseconds
     */
	__forceinline bool IRAM_ATTR setIntervalMs(uint32_t milliseconds)
	{
		return setIntervalUs(milliseconds * 1000);
	}

	/** @brief  Set timer trigger callback
     *  @param  callback Function to call when timer triggers
     */
	void IRAM_ATTR setCallback(InterruptCallback callback)
	{
		if(callback == nullptr) {
			stop();
			hw_timer1_detach_interrupt();
			state = eTS_CallbackNotSet;
		} else {
			hw_timer1_attach_interrupt(mode == eHWT_NonMaskable ? TIMER_NMI_SOURCE : TIMER_FRC1_SOURCE,
									   reinterpret_cast<hw_timer_callback_t>(callback), nullptr);
			state = eTS_Disarmed;
		}
	}

private:
	HardwareTimerMode mode = eHWT_NonMaskable;

	/// Fixed prescale value
	static constexpr unsigned prescale = 16;
	static constexpr hw_timer_clkdiv_t clkdiv = TIMER_CLKDIV_16;
	static constexpr uint32_t frequency = HW_TIMER_BASE_CLK / prescale;

	uint32_t interval = 0; ///< Actual timer tick interval
	bool repeating = false;

	enum State {
		eTS_CallbackNotSet,
		eTS_Disarmed,
		eTS_Armed,
	};
	static uint8_t state;
};

/**
 * @deprecated Use HardwareTimer class instead
 */
class Hardware_Timer : public HardwareTimer
{
};

/** @} */
