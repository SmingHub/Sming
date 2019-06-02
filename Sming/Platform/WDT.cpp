/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WDT.cpp
 *
 *  Created on: 06 àïð. 2015 ã.
 *      Author: Anakod
 *
 */

#include "Platform/WDT.h"

WDTClass WDT;

void WDTClass::enable(bool enableWatchDog)
{
	enabled = enableWatchDog;
	if(System.isReady())
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
	if(enabled)
		system_soft_wdt_restart();
	else
		system_soft_wdt_stop();
}
