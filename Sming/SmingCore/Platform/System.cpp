/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "System.h"
#include "../Interrupts.h"
#include "../../Services/SpifFS/spiffs_sming.h"

SystemClass System;

SystemClass::SystemClass()
{
	state = eSS_None;
}

void SystemClass::initialize()
{
	if (state != eSS_None)
		return;
	state = eSS_Intializing;

	system_init_done_cb(staticReadyHandler);
}

void SystemClass::restart()
{
	system_restart();
}

bool SystemClass::isReady()
{
	return state == eSS_Ready;
}

void SystemClass::onReady(SystemReadyDelegate readyHandler)
{
	readyHandlers.add(readyHandler);
}

void SystemClass::onReady(ISystemReadyHandler* readyHandler)
{
	readyInterfaces.add(readyHandler);
}

void SystemClass::setCpuFrequency(CpuFrequency freq)
{
	if (freq == eCF_160MHz)
		REG_SET_BIT(0x3ff00014, BIT(0));
	else
		REG_CLR_BIT(0x3ff00014, BIT(0));

	ets_update_cpu_frequency(freq);
}

CpuFrequency SystemClass::getCpuFrequency()
{
	return (CpuFrequency)ets_get_cpu_frequency();
}

bool SystemClass::deepSleep(uint32 timeMilliseconds, DeepSleepOptions options /* = eDSO_RF_CAL_BY_INIT_DATA */)
{
	if (!system_deep_sleep_set_option((uint8)options))
		return false;
	system_deep_sleep(timeMilliseconds * 1000);
	return true;
}

void SystemClass::staticReadyHandler()
{
	System.readyHandler();
}

void SystemClass::readyHandler()
{
	state = eSS_Ready;
	for (int i = 0; i < readyHandlers.count(); i++)
		readyHandlers[i]();
	for (int i = 0; i < readyInterfaces.count(); i++)
		readyInterfaces[i]->onSystemReady();

	readyHandlers.clear();
	readyInterfaces.clear();
}
