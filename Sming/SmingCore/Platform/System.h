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

class BssInfo;

typedef void (*SystemReadyCallback)(void);

class ISystemReadyHandler
{
public:
	virtual void onSystemReady() = 0;
};

enum CpuFrequency
{
	eCF_80MHz = 80,
	eCF_160MHz = 160,
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

	void onReady(SystemReadyCallback readyHandler);
	void onReady(ISystemReadyHandler* readyHandler);

private:
	static void staticReadyHandler();
	void readyHandler();

private:
	Vector<SystemReadyCallback> readyHandlers;
	Vector<ISystemReadyHandler*> readyInterfaces;
	SystemState state;
};

extern SystemClass System;

#endif /* SMINGCORE_PLATFORM_SYSTEM_H_ */
