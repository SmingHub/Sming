/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * System.cpp
 *
 ****/

#include "Platform/System.h"
#include "Timer.h"

#ifndef TASK_QUEUE_LENGTH
/** @brief default number of tasks in global queue
 *  @note tasks are usually short-lived and executed very promptly, so a large queue is
 *  normally un-necessry. If queue overrun is suspected, check `SystemClass::getMaxTaskCount()`.
 */
#define TASK_QUEUE_LENGTH 10
#endif

SystemClass System;

SystemState SystemClass::state = eSS_None;
os_event_t SystemClass::taskQueue[TASK_QUEUE_LENGTH];

static_assert(TASK_QUEUE_LENGTH >= 8, "Task queue too small");

#ifdef ENABLE_TASK_COUNT
volatile uint8_t SystemClass::taskCount;
volatile uint8_t SystemClass::maxTaskCount;
#endif

/** @brief OS calls this function which invokes user-defined callback
 *  @note callback function pointer is placed in event->sig, with parameter in event->par.
 */
void SystemClass::taskHandler(os_event_t* event)
{
#ifdef ENABLE_TASK_COUNT
	auto level = noInterrupts();
	--taskCount;
	restoreInterrupts(level);
#endif
	auto callback = reinterpret_cast<TaskCallback32>(event->sig);
	if(callback != nullptr) {
		callback(event->par);
	}
}

bool SystemClass::initialize()
{
	if(state != eSS_None) {
		return false;
	}

	state = eSS_Intializing;

	// Initialise the global task queue
	if(!system_os_task(taskHandler, USER_TASK_PRIO_1, taskQueue, TASK_QUEUE_LENGTH)) {
		return false;
	}

#ifdef ARCH_ESP8266
	system_init_done_cb([]() { state = eSS_Ready; });
#else
	state = eSS_Ready;
#endif

	return true;
}

bool SystemClass::queueCallback(TaskCallback32 callback, uint32_t param)
{
	if(callback == nullptr) {
		return false;
	}

#ifdef ENABLE_TASK_COUNT
	auto level = noInterrupts();
	++taskCount;
	if(taskCount > maxTaskCount) {
		maxTaskCount = taskCount;
	}
	restoreInterrupts(level);
#endif

	return system_os_post(USER_TASK_PRIO_1, reinterpret_cast<os_signal_t>(callback), param);
}

bool SystemClass::queueCallback(TaskDelegate callback)
{
	if(!callback) {
		return false;
	}

	// @todo consider failing immediately if called from interrupt context

	auto delegate = new TaskDelegate(callback);
	if(delegate == nullptr) {
		return false;
	}

	auto delegateHandler = [](void* param) {
		auto delegate = static_cast<TaskDelegate*>(param);
		(*delegate)();
		delete delegate;
	};

	if(!queueCallback(delegateHandler, delegate)) {
		delete delegate;
		return false;
	}

	return true;
}

void SystemClass::restart(unsigned deferMillis)
{
	if(deferMillis == 0) {
		queueCallback(system_restart);
	} else {
		auto timer = new AutoDeleteTimer;
		timer->initializeMs(deferMillis, system_restart).startOnce();
	}
}

bool SystemClass::deepSleep(uint32_t timeMilliseconds, DeepSleepOptions options)
{
	if(!system_deep_sleep_set_option(options)) {
		return false;
	}
	// Note: In SDK Version 3+ system_deep_sleep() returns bool but it's void before that
	system_deep_sleep(timeMilliseconds * 1000);
	return true;
}
