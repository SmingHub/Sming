/*
 * WDT.cpp
 *
 *  Created on: 06 àïð. 2015 ã.
 *      Author: Anakod
 */

#include "WDT.h"
#include <user_config.h>

WDTClass WDT;

void WDTClass::enable(bool enableWatchDog)
{
	_enabled = enableWatchDog;
	System.onReady(this);
}

void WDTClass::alive()
{
	system_soft_wdt_restart();
}

void WDTClass::internalApplyEnabled()
{
	if (_enabled)
		system_soft_wdt_restart();
	else
		system_soft_wdt_stop();
}
