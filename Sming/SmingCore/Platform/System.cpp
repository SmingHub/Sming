/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "System.h"
#include "Interrupts.h"

SystemClass System;

os_event_t SystemClass::_taskQueue[TASK_QUEUE_LENGTH];

/** @brief OS calls this function which invokes user-defined callback
 *  @note callback function pointer is placed in event->sig, with parameter
 *  in event->par.
 */
static void __taskHandler(os_event_t* event)
{
	task_callback_t callback = (task_callback_t)event->sig;
	if (callback)
		callback(event->par);
}

void SystemClass::initialize()
{
	if (_state != eSS_None)
		return;

	_state = eSS_Intializing;

	// Initialise the global task queue
	system_os_task(__taskHandler, USER_TASK_PRIO_1, _taskQueue, TASK_QUEUE_LENGTH);

	system_init_done_cb([]() { System.readyHandler(); });
}

void SystemClass::onReady(SystemReadyDelegate readyHandler)
{
	if (readyHandler) {
		if (isReady())
			readyHandler();
		else
			_readyHandlers.add(readyHandler);
	}
}

void SystemClass::onReady(ISystemReadyHandler* readyHandler)
{
	if (readyHandler) {
		if (isReady())
			readyHandler->onSystemReady();
		else
			_readyInterfaces.add(readyHandler);
	}
}

void SystemClass::setCpuFrequency(CpuFrequency freq)
{
	if (freq == eCF_160MHz)
		REG_SET_BIT(0x3ff00014, BIT(0));
	else
		REG_CLR_BIT(0x3ff00014, BIT(0));

	ets_update_cpu_frequency(freq);
}

bool SystemClass::deepSleep(uint32 timeMilliseconds, DeepSleepOptions options)
{
	if (!system_deep_sleep_set_option((uint8)options))
		return false;
	system_deep_sleep(timeMilliseconds * 1000);
	return true;
}

void SystemClass::readyHandler()
{
	_state = eSS_Ready;
	for (unsigned i = 0; i < _readyHandlers.count(); i++)
		_readyHandlers[i]();

	for (unsigned i = 0; i < _readyInterfaces.count(); i++)
		_readyInterfaces[i]->onSystemReady();

	_readyHandlers.clear();
	_readyInterfaces.clear();
}
