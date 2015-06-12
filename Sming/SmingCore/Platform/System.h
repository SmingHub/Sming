/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_PLATFORM_SYSTEM_H_
#define SMINGCORE_PLATFORM_SYSTEM_H_

#include <user_config.h>
#include "../../Wiring/WString.h"
#include "../../Wiring/WVector.h"
#include "../SmingCore/Delegate.h"

class BssInfo;

typedef Delegate<void()> SystemReadyDelegate;

class ISystemReadyHandler
{
public:
	virtual ~ISystemReadyHandler() {}
	virtual void onSystemReady() = 0;
};

enum CpuFrequency
{
	eCF_80MHz = 80,
	eCF_160MHz = 160,
};

enum DeepSleepOptions
{
	eDSO_RF_CAL_BY_INIT_DATA = 0, //  RF_CAL  or  not  after  deep-sleep  wake  up, depends on init data byte 108.
	eDSO_RF_CAL_ALWAYS = 1, // RF_CAL after deep-sleep wake up, there will be large current.
	eDSO_RF_CAL_NEVER = 2, //  no RF_CAL after deep-sleep wake up, there will only be small current.
	eDSO_DISABLE_RF = 4, // disable RF after deep-sleep wake up, just like modem sleep, there will be the smallest current.
};

enum SystemState
{
	eSS_None,
	eSS_Intializing,
	eSS_Ready
};

class SystemClass
{
public:
	SystemClass();
	void initialize();

	bool isReady(); // System initialization was completed?
	void restart();

	void setCpuFrequency(CpuFrequency freq);
	CpuFrequency getCpuFrequency();
	bool deepSleep(uint32 timeMilliseconds, DeepSleepOptions options = eDSO_RF_CAL_BY_INIT_DATA);

	void onReady(SystemReadyDelegate readyHandler);
	void onReady(ISystemReadyHandler* readyHandler);

	void applyFirmwareUpdate(uint32_t readFlashOffset, uint32_t targetFlashOffset, int firmwareSize);

private:
	static void staticReadyHandler();
	void readyHandler();
	void IRAM_ATTR internalApplyFirmwareUpdate(uint32_t readFlashOffset, uint32_t targetFlashOffset, int firmwareSize, bool outputDebug);

private:
	Vector<SystemReadyDelegate> readyHandlers;
	Vector<ISystemReadyHandler*> readyInterfaces;
	SystemState state;
};

extern SystemClass System;

#endif /* SMINGCORE_PLATFORM_SYSTEM_H_ */
