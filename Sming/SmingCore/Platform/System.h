/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/**	@defgroup system System
 *	@brief	Access to the ESP8266 system
 *	@note   Provides system control and monitoring of the ESP8266.
*/
#ifndef SMINGCORE_PLATFORM_SYSTEM_H_
#define SMINGCORE_PLATFORM_SYSTEM_H_

#include <user_config.h>
#include "../../Wiring/WString.h"
#include "../../Wiring/WVector.h"
#include "../SmingCore/Delegate.h"

class BssInfo;

/// @ingroup event_handlers
typedef Delegate<void()> SystemReadyDelegate; ///< Handler function for system ready

class ISystemReadyHandler
{
public:
	virtual ~ISystemReadyHandler() {}

    /** @brief  Handle <i>system ready</i> events
    */
	virtual void onSystemReady() = 0;
};

/** @ingroup constants
 *  @{
 */

/// CPU Frequency
enum CpuFrequency
{
	eCF_80MHz = 80, ///< CPU 80MHz
	eCF_160MHz = 160, ///< CPU 160MHz
};

/// Deep sleep options
enum DeepSleepOptions
{
	eDSO_RF_CAL_BY_INIT_DATA = 0, ///<  RF_CAL  or  not  after  deep-sleep  wake  up, depends on init data byte 108.
	eDSO_RF_CAL_ALWAYS = 1, ///< RF_CAL after deep-sleep wake up, there will be large current.
	eDSO_RF_CAL_NEVER = 2, ///<  no RF_CAL after deep-sleep wake up, there will only be small current.
	eDSO_DISABLE_RF = 4, ///< disable RF after deep-sleep wake up, just like modem sleep, there will be the smallest current.
};

/// @brief  System state
enum SystemState
{
	eSS_None, ///< System state unknown
	eSS_Intializing, ///< System initialising
	eSS_Ready ///< System ready
};
/** @} */

class SystemClass
{
public:
    /** @brief  System class
     *  @addtogroup system
     *  @{
     */
	SystemClass();

	/** @brief System initialisation
	 */
	void initialize();

    /** @brief  Check if system ready
     *  @retval bool True if system initialisation is complete and system is now ready
     */
	bool isReady();

    /** @brief  Restart system
     */
	void restart();

    /** @brief  Set the CPU frequency
     *  @param  freq Frequency to set CPU
     */
	void setCpuFrequency(CpuFrequency freq);

    /** @brief  Get the CPU frequency
     *  @retval CpuFrequency The frequency of the CPU
     */
	CpuFrequency getCpuFrequency();

    /** @brief  Enter deep sleep mode
     *  @param  timeMilliseconds Quantity of milliseconds to remain in deep sleep mode
     *  @param  options Deep sleep options
     */
	bool deepSleep(uint32 timeMilliseconds, DeepSleepOptions options = eDSO_RF_CAL_BY_INIT_DATA);

    /** @brief  Set handler for <i>system ready</i> event
     *  @param  readyHandler Function to handle event
     */
	void onReady(SystemReadyDelegate readyHandler);

    /** @brief  Set handler for <i>system ready</i> event
     *  @param  readyHandler Function to handle event
     */
	void onReady(ISystemReadyHandler* readyHandler);

    /** @brief  Apply a firmware update
     *  @param  readFlashOffset Address of the firmware to use for update
     *  @param  targetFlashOffset Address to write firmware to
     *  @param  firmwareSize Quantity of bytes to write
     */
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

/**	@brief	Global instance of system object
 *	@note	Use system.<i>function</i> to access system functions
 *	@note	Example:
 *	@code	system.reset();
 *	@endcode
 */
extern SystemClass System;

/** @} */
#endif /* SMINGCORE_PLATFORM_SYSTEM_H_ */
