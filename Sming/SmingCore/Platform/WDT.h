/*
 * WDT.h
 *
 *  Created on: 06 апр. 2015 г.
 *      Author: Anakod
 */

#ifndef SMINGCORE_PLATFORM_WDT_H_
#define SMINGCORE_PLATFORM_WDT_H_

#include <user_config.h>
#include "System.h"

class WDTClass : protected ISystemReadyHandler
{
public:
	WDTClass();
	virtual ~WDTClass() {}

	void enable(bool enableWatchDog);
	void alive();

protected:
	virtual void onSystemReady();
	void internalApplyEnabled();

private:
	bool enabled;
};

extern WDTClass WDT;

#endif /* SMINGCORE_PLATFORM_WDT_H_ */
