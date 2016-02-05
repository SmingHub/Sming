/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/
/** @defgroup   pwm Pulse width modulation output functions
 *  @brief      Provides pulse width modulation output functions
*/

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Wiring/WVector.h"

#define PWM_DEPTH 255

#ifndef _SMING_CORE_PWM_H_
#define _SMING_CORE_PWM_H_

class ChannelPWM;

class DriverPWM
{
public:
    /** @brief  PWM class
     *  @ingroup pwm
     *  @{
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

class ChannelPWM
{
public:
	ChannelPWM();
	ChannelPWM(int DriverPWMPin);

	void initialize();
	void IRAM_ATTR high();
	void config(int duty, uint32_t basePeriod);
	__inline int id() { return pin; }
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
 *  @ingroup pwm
 */
extern DriverPWM EspPWM;

#endif /* _SMING_CORE_PWM_H_ */
