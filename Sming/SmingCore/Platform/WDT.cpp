/*
 * WDT.cpp
 *
 *  Created on: 06 àïð. 2015 ã.
 *      Author: Anakod
 */

#include "WDT.h"

WDTClass WDT;

WDTClass::WDTClass()
{
	//enabled = true;
}

void WDTClass::enable(bool enableWatchDog)
{
	enabled = enableWatchDog;
	if (System.isReady())
		internalApplyEnabled();
	else
		System.onReady(this);
}

void WDTClass::alive()
{
	system_soft_wdt_restart();
}

void WDTClass::onSystemReady()
{
	internalApplyEnabled();
}

void WDTClass::internalApplyEnabled()
{
	if (enabled)
		system_soft_wdt_restart();
	else
		system_soft_wdt_stop();
}
