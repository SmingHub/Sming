/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "System.h"

SystemClass System;

SystemClass::SystemClass()
{
	state = eSS_None;
}

void SystemClass::initialize()
{
	if (state != eSS_None) return;
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

void SystemClass::onReady(SystemReadyCallback readyHandler)
{
	readyHandlers.add(readyHandler);
}

void SystemClass::onReady(ISystemReadyHandler* readyHandler)
{
	readyInterfaces.add(readyHandler);
}

void SystemClass::cpuUpdate(uint8 freq)
{

	REG_SET_BIT(0x3ff00014, BIT(0));
	os_update_cpu_frequency(freq);
}

uint8 SystemClass::getCpuFreq()
{
	return system_get_cpu_freq();
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
