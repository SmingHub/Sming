/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   pwm Pulse width modulation
 *  @brief      Provides pulse width modulation output functions
 *  @{
*/

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Wiring/WVector.h"

#define PWM_DEPTH 255 ///< Maximum PWM depth

#ifndef _SMING_CORE_PWM_H_
#define _SMING_CORE_PWM_H_

class ChannelPWM;

/// Pulse width modulator driver class
class DriverPWM
{
public:
    /** @brief  PWM driver class
     */
	DriverPWM();

    /** @brief  Initialise PWM driver
     *  @note   Call this function once before using other PWM driver functions
     */
	void initialize();

    /** @brief  Set PWM output
     *  @param  pin GPIO to set
     *  @param  duty PWM duty
     *  @todo   Describe how <i>duty</i> affects output
     */
	void analogWrite(uint8_t pin, int duty);

    /** @brief  Disable PWM on GPIO
     *  @param  pin GPIO on which to disable PWM
     */
	void noAnalogWrite(uint8_t pin);

    /** @} */
protected:
	static void IRAM_ATTR processingStatic(void *arg);

private:
	os_timer_t main;
	Vector<ChannelPWM> channels;
	uint32_t period;
	bool initialized;
};

/// Pulse width modulator channel class
class ChannelPWM
{
public:
    /** @brief  PWM channel
    */
	ChannelPWM();

	/** @brief  PWM channel
	 *  @param  DriverPWMPin GPIO for this PWM channel
	 */
	ChannelPWM(int DriverPWMPin);

    /** @brief  Initialise PWM channel
    */
	void initialize();

	/** @brief  Set GPIO high and reset PWM timer
	 *  @note   Extends high (on) period for this cycle
	*/
	void IRAM_ATTR high();

	/** @brief  Configure PWM channel
	 *  @param  duty Duty cycle
        @param  basePeriod Period of PWM cycle
    */
	void config(int duty, uint32_t basePeriod);

	/** @brief  Get the GPIO used by this PWM channel
	 *  @retval int GPIO in use
	 */
	__inline int id() { return pin; }

	/** @brief  Close PWM channel
	 *  @note   Turns PWM off
	 */
	void close();

protected:
	static void IRAM_ATTR processingStatic(void *arg);

private:
	os_timer_t item;
	int pin;
	uint32_t time;
};

// OOP style
/**	@brief	Global instance of PWM driver object
 *	@note	Use EspPWM.<i>function</i> to access PWM driver functions
 *	@note	Example:
 *	@code	EspPWM.analogWrite(5, 50);
 *	@endcode
 */
extern DriverPWM EspPWM;

/** @} */
#endif /* _SMING_CORE_PWM_H_ */
