/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "System.h"
#include "Interrupts.h"

SystemClass System;

void SystemClass::initialize()
{
	if (_state != eSS_None)
		return;

	_state = eSS_Intializing;

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
