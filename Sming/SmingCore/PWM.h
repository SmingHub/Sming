/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Wiring/WVector.h"

#define PWM_DEPTH 255

#ifndef _SMING_CORE_PWM_H_
#define _SMING_CORE_PWM_H_

class ChannelPWM;

class DriverPWM
{
public:
	DriverPWM();

	void initialize();
	void analogWrite(uint8_t pin, int duty);
	void noAnalogWrite(uint8_t pin);

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

#endif /* _SMING_CORE_PWM_H_ */

// OOP style
extern DriverPWM EspPWM;

// Arduino like style
void analogWrite(uint8_t pin, int duty);
void noAnalogWrite(uint8_t pin);

uint16_t analogRead(uint16_t pin);
