/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WDT.h
 *
 *  Created on: 06 ���. 2015 �.
 *      Author: Anakod
 *
 ****/

/**	@defgroup wdt Watchdog Timer
 *	@brief	Access to the ESP8266 watchdog timer
 *	@note   Provides control of the ESP8266 watchdog timer.
 *  @note   The WDT can be used to restart the ESP8266 if the firmware becomes unresponsive.
 *          To use WDT, enable the WDT then poke it regularly with WDT.alive();
*/

#pragma once

#include <user_config.h>
#include "System.h"

/** @brief  Watchdog timer class
 *  @addtogroup wdt
 *  @{
 */

class WDTClass : protected ISystemReadyHandler
{
public:
	/** @brief  Enable or disable watchdog timer
     *  @param  enableWatchDog True to enable. False to disable.
     */
	void enable(bool enableWatchDog);

	/** @brief  Keep watchdog timer alive
     *  @note   Call this function regularly to stop WDT from activating
     *  @todo   Define the WDT period (how long before it triggers)
     */
	void alive();

protected:
	void onSystemReady() override;

	void internalApplyEnabled();

private:
	bool enabled = true;
};

/**	@brief	Global instance of watchdog timer object
 *	@note	Use WFT.<i>function</i> to access watchdog timer functions
 *	@note	Example:
 *	@code	WDT.alive();
 *	@endcode
 */
extern WDTClass WDT;

/** @} */
