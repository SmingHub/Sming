/*
 * WDT.cpp
 *
 *  Created on: 06 апр. 2015 г.
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
	WRITE_PERI_REG(0x60000914, 0x73);
}

void WDTClass::onSystemReady()
{
	internalApplyEnabled();
}

void WDTClass::internalApplyEnabled()
{
	if (enabled)
		ets_wdt_enable();
	else
		ets_wdt_disable();
}
