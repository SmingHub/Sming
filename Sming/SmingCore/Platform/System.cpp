/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "System.h"
#include "Interrupts.h"

SystemClass System;

Vector<SystemReadyDelegate> SystemClass::readyHandlers;
Vector<ISystemReadyHandler*> SystemClass::readyInterfaces;
SystemState SystemClass::state = eSS_None;
os_event_t SystemClass::taskQueue[TASK_QUEUE_LENGTH];
volatile uint8_t SystemClass::taskCount;
volatile uint8_t SystemClass::maxTaskCount;

/** @brief OS calls this function which invokes user-defined callback
 *  @note callback function pointer is placed in event->sig, with parameter
 *  in event->par.
 */
void SystemClass::taskHandler(os_event_t* event)
{
	auto callback = reinterpret_cast<TaskCallback>(event->sig);
	if(callback) {
		// If we get interrupt during adjustment of the counter, do it again
		uint8_t oldCount = taskCount;
		--taskCount;
		if(taskCount != oldCount - 1)
			--taskCount;
		callback(event->par);
	}
}

void SystemClass::initialize()
{
	if(state != eSS_None)
		return;
	state = eSS_Intializing;

	// Initialise the global task queue
	system_os_task(taskHandler, USER_TASK_PRIO_1, taskQueue, TASK_QUEUE_LENGTH);

	system_init_done_cb(readyHandler);
}

bool SystemClass::queueCallback(TaskCallback callback, uint32_t param)
{
	if(callback == nullptr) {
		return false;
	}

	if(++taskCount > maxTaskCount) {
		maxTaskCount = taskCount;
	}

	return system_os_post(USER_TASK_PRIO_1, reinterpret_cast<os_signal_t>(callback), param);
}

void SystemClass::onReady(SystemReadyDelegate readyHandler)
{
	if(!readyHandler) {
		return;
	}

	if(isReady()) {
		readyHandler();
	} else {
		readyHandlers.add(readyHandler);
	}
}

void SystemClass::onReady(ISystemReadyHandler* readyHandler)
{
	if(readyHandler == nullptr) {
		return;
	}

	if(isReady()) {
		readyHandler->onSystemReady();
	} else {
		readyInterfaces.add(readyHandler);
	}
}

void SystemClass::setCpuFrequency(CpuFrequency freq)
{
	if(freq == eCF_160MHz)
		REG_SET_BIT(0x3ff00014, BIT(0));
	else
		REG_CLR_BIT(0x3ff00014, BIT(0));

	ets_update_cpu_frequency(freq);
}

bool SystemClass::deepSleep(uint32 timeMilliseconds, DeepSleepOptions options)
{
	if(!system_deep_sleep_set_option((uint8)options))
		return false;
	system_deep_sleep(timeMilliseconds * 1000);
	return true;
}

void SystemClass::readyHandler()
{
	state = eSS_Ready;
	for(unsigned i = 0; i < readyHandlers.count(); i++) {
		readyHandlers[i]();
	}

	for(unsigned i = 0; i < readyInterfaces.count(); i++) {
		readyInterfaces[i]->onSystemReady();
	}

	readyHandlers.clear();
	readyInterfaces.clear();
}
