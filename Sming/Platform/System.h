/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * System.h
 *
 * @author: 14/8/2018 - mikee47 <mike@sillyhouse.net>
 *
 * 	When SystemClass::onReady method called, the callback is only queued if the
 * 	system is not actually ready; there is otherwise no point in queueing the
 * 	callback as it would never get invoked. To avoid unpredictable behaviour and
 * 	simplify application code, the callback is invoked immediately in this situation.
 *
 * 	Global task queue added to class, initialised at system startup.
 *
 */

/**	@defgroup system System
 *	@brief	Access to the ESP8266 system
 *	@note   Provides system control and monitoring of the ESP8266.
*/
#pragma once

#include "Delegate.h"

/** @brief default number of tasks in global queue
 *  @note tasks are usually short-lived and executed very promptly. If necessary this
 *  value can be overridden in makefile or user_config.h.
 */
#ifndef TASK_QUEUE_LENGTH
#define TASK_QUEUE_LENGTH 10
#endif

/** @brief Task callback function type
 * 	@ingroup event_handlers
 * 	@note Callback code does not need to be in IRAM
 *  @todo Integrate delegation into callbacks
 */
typedef void (*TaskCallback)(uint32_t param);

/// @ingroup event_handlers
typedef Delegate<void()> SystemReadyDelegate; ///< Handler function for system ready

class ISystemReadyHandler
{
public:
	virtual ~ISystemReadyHandler()
	{
	}

	/** @brief  Handle <i>system ready</i> events
    */
	virtual void onSystemReady() = 0;
};

/** @ingroup constants
 *  @{
 */

/// CPU Frequency
enum CpuFrequency {
	eCF_80MHz = 80,   ///< CPU 80MHz
	eCF_160MHz = 160, ///< CPU 160MHz
};

/// Deep sleep options
enum DeepSleepOptions {
	eDSO_RF_CAL_BY_INIT_DATA = 0, ///<  RF_CAL  or  not  after  deep-sleep  wake  up, depends on init data byte 108.
	eDSO_RF_CAL_ALWAYS = 1,		  ///< RF_CAL after deep-sleep wake up, there will be large current.
	eDSO_RF_CAL_NEVER = 2,		  ///<  no RF_CAL after deep-sleep wake up, there will only be small current.
	eDSO_DISABLE_RF =
		4, ///< disable RF after deep-sleep wake up, just like modem sleep, there will be the smallest current.
};

/// @brief  System state
enum SystemState {
	eSS_None,		 ///< System state unknown
	eSS_Intializing, ///< System initialising
	eSS_Ready		 ///< System ready
};
/** @} */

class SystemClass
{
public:
	/** @brief  System class
     *  @addtogroup system
     *  @{
     */
	SystemClass()
	{
	}

	/** @brief System initialisation
	 *  @retval bool true on success
	 *  @note Called by user_main: applications should not call this function or the task queue
	 *  will be re-initialised and any currently queued tasks won't be called.
	 */
	static bool initialize();

	/** @brief  Check if system ready
     *  @retval bool True if system initialisation is complete and system is now ready
     */
	bool isReady()
	{
		return state == eSS_Ready;
	}

	/** @brief Request a restart of the system
	 *  @param deferMillis defer restart request by a number of milliseconds
	 *  @note A delay is often required to allow network callback code to complete correctly.
	 *  The restart is always deferred, either using the task queue (if deferMillis == 0)
	 *  or using a timer. This method always returns immediately.
     */
	void restart(unsigned deferMillis = 0);

	/** @brief  Set the CPU frequency
     *  @param  freq Frequency to set CPU
     */
	void setCpuFrequency(CpuFrequency freq);

	/** @brief  Get the CPU frequency
     *  @retval CpuFrequency The frequency of the CPU
     */
	CpuFrequency getCpuFrequency()
	{
		return static_cast<CpuFrequency>(system_get_cpu_freq());
	}

	/** @brief  Enter deep sleep mode
     *  @param  timeMilliseconds Quantity of milliseconds to remain in deep sleep mode
     *  @param  options Deep sleep options
     */
	bool deepSleep(uint32_t timeMilliseconds, DeepSleepOptions options = eDSO_RF_CAL_BY_INIT_DATA);

	/** @brief  Set handler for <i>system ready</i> event
     *  @param  readyHandler Function to handle event
     *  @note if system is ready, callback is executed immediately without deferral
     */
	void onReady(SystemReadyDelegate readyHandler);

	/** @brief  Set handler for <i>system ready</i> event
     *  @param  readyHandler Function to handle event
     *  @note if system is ready, callback is executed immediately without deferral
     */
	void onReady(ISystemReadyHandler* readyHandler);

	/**
	 * @brief Queue a deferred callback.
	 * @param callback The function to be called
	 * @param param Parameter passed to the callback
	 * @retval bool false if callback could not be queued
	 * @note It is important to check the return value to avoid memory leaks and other issues,
	 * for example if memory is allocated and relies on the callback to free it again.
	 * Note also that this method is typically called from interrupt context so must avoid things
	 * like heap allocation, etc.
	 */
	static bool IRAM_ATTR queueCallback(TaskCallback callback, uint32_t param = 0);

	/** @brief Get number of tasks currently on queue
	 *  @retval unsigned
	 */
	static unsigned getTaskCount()
	{
		return taskCount;
	}

	/** @brief Get maximum number of tasks seen on queue at any one time
	 *  @retval unsigned
	 *  @note If return value is higher than maximum task queue TASK_QUEUE_LENGTH then
	 *  the queue has overflowed at some point and tasks have been left un-executed.
	 */
	static unsigned getMaxTaskCount()
	{
		return maxTaskCount;
	}

private:
	static void taskHandler(os_event_t* event);

private:
	static SystemState state;
	static os_event_t taskQueue[];		  ///< OS task queue
	static volatile uint8_t taskCount;	///< Number of tasks on queue
	static volatile uint8_t maxTaskCount; ///< Profiling to establish appropriate queue size
};

/**	@brief	Global instance of system object
 *	@note	Use system.<i>function</i> to access system functions
 *	@note	Example:
 *	@code	system.reset();
 *	@endcode
 */
extern SystemClass System;

/** @} */
