/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Timer.h
 *
 ****/

/** @defgroup   timer Timer functions
 *  @brief      Provides timer functions
*/
#pragma once

#include "Interrupts.h"
#include "SimpleTimer.h"

typedef Delegate<void()> TimerDelegate;

/** @deprecated Use `TimerDelegate` */
typedef std::function<void()> TimerDelegateStdFunction SMING_DEPRECATED;

class Timer
{
public:
	/** @brief  Timer class
     *  @ingroup timer
     *  @{
     */
	Timer()
	{
	}

	~Timer()
	{
		stop();
	}

	// Methods return object reference for Method Chaining
	// http://en.wikipedia.org/wiki/Method_chaining
	// We provide both versions: Delegate and classic c-style callback function for performance reason (for high-frequency timers)
	// Usually only Delegate needed

	/** @brief  Initialise millisecond timer
     *  @param  milliseconds Duration of timer in milliseconds
     *  @param  callback Function to call when timer triggers
     *  @note   Classic c-style callback method
     */
	Timer& IRAM_ATTR initializeMs(uint32_t milliseconds, InterruptCallback callback = nullptr);

	/** @brief  Initialise microsecond timer
     *  @param  microseconds Duration of timer in milliseconds
     *  @param  callback Function to call when timer triggers
     *  @note   Classic c-style callback method
     */
	Timer& IRAM_ATTR initializeUs(uint32_t microseconds, InterruptCallback callback = nullptr);

	/** @brief  Initialise millisecond timer
     *  @param  milliseconds Duration of timer in milliseconds
     *  @param  delegateFunction Function to call when timer triggers
     *  @note   Delegate callback method
     */
	Timer& IRAM_ATTR initializeMs(uint32_t milliseconds, TimerDelegate delegateFunction = nullptr);

	/** @brief  Initialise microsecond timer
     *  @param  microseconds Duration of timer in milliseconds
     *  @param  delegateFunction Function to call when timer triggers
     *  @note   Delegate callback method
     */
	Timer& IRAM_ATTR initializeUs(uint32_t microseconds, TimerDelegate delegateFunction = nullptr);

	/** @brief  Start timer running
     *  @param  repeating Set to true for repeating timer. Set to false for one-shot.
     */
	void IRAM_ATTR start(bool repeating = true);

	/** @brief  Start one-shot timer running
     *  @note   Will start timer and trigger once after configured duration.
     */
	__forceinline void IRAM_ATTR startOnce()
	{
		start(false);
	}

	/** @brief  Stop timer
     *  @note   Stops a running timer. Has no effect on stopped timer.
     */
	void IRAM_ATTR stop();

	/** @brief  Restarts timer
     *  @note   Restarts the timer from zero, extending duration.
     */
	__forceinline void IRAM_ATTR restart()
	{
		stop();
		start();
	}

	/** @brief  Check if timer is started
     *  @retval bool True if timer is running
     */
	bool isStarted()
	{
		return started;
	}

	/** @brief  Get timer interval
     *  @retval uint64_t Timer interval in microseconds
     */
	uint64_t getIntervalUs()
	{
		return (interval * longIntervalCounterLimit) ?: 1;
	}

	/** @brief  Get timer interval
     *  @retval uint32_t Timer interval in milliseconds
     */
	uint32_t getIntervalMs()
	{
		return getIntervalUs() / 1000;
	}

	/** @brief  Set timer interval
     *  @param  microseconds Interval in microseconds. (Default: 1ms)
     */
	void IRAM_ATTR setIntervalUs(uint64_t microseconds = 1000000);

	/** @brief  Set timer interval
     *  @param  milliseconds Interval in milliseconds. (Default: 1s)
     */
	void IRAM_ATTR setIntervalMs(uint32_t milliseconds = 1000000);

	/** @brief  Set timer trigger function
     *  @param  interrupt Function to be called on timer trigger
     *  @note   Classic c-type callback method
     */
	void IRAM_ATTR setCallback(InterruptCallback interrupt = nullptr);

	/** @brief  Set timer trigger function
	*  @param  delegateFunction Function to be called on timer trigger
	*  @note   Delegate callback method
	*/
	void IRAM_ATTR setCallback(TimerDelegate delegateFunction);

protected:
	void IRAM_ATTR processing();

	/** @brief  virtual timer loop() method
     *  @note   Can be override in class derivations. If overwriten,
     *          no classic other callbacks are working.
     */
	void tick();
	/** @} */

private:
	SimpleTimer simpleTimer; ///< We use a SimpleTimer to access the hardware
	uint32_t interval = 0;
	InterruptCallback callback = nullptr;
	TimerDelegate delegateFunc;
	bool repeating = false;
	bool started = false;

	// Because of the limitation in Espressif SDK a workaround
	// was added to allow for longer timer intervals.
	uint16_t longIntervalCounter = 0;
	uint16_t longIntervalCounterLimit = 0;
};
