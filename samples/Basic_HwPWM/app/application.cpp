/*
 * File: Esp SDK Hardware PWM demo
 * Original Author: https://github.com/hrsavla
 *
 * This HardwarePWM library enables Sming framework user to use ESP SDK PWM API
 * Period of PWM is fixed to 1000us / Frequency = 1khz
 * Duty at 100% = 22222. Duty at 0% = 0
 * You can use function setPeriod() to change frequency/period.
 * Calculate the Duty as per the formulae give in ESP8266 SDK
 * Duty = (Period *1000)/45
 *
 * PWM can be generated on up to 8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 *
 * See also ESP8266 Technical Reference, Chapter 12:
 * http://espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf
 */
#include <SmingCore.h>
#include <HardwarePWM.h>

namespace
{
// List of pins that you want to connect to pwm
uint8_t pins[]
{
#if defined(ARCH_ESP32)
	3, 4, 5, 18, 19, 4,
#elif defined(ARCH_RP2040)
	2,
#else
	2, 4, 5, 0, 15, 13, 12, 14,
#endif
};

HardwarePWM HW_pwm(pins, ARRAY_SIZE(pins));

SimpleTimer procTimer;

const int maxDuty = HW_pwm.getMaxDuty();

void doPWM()
{
	static bool countUp = true;
	static int duty;

	const int increment = maxDuty / 50;

	if(countUp) {
		duty += increment;
		if(duty >= maxDuty) {
			duty = maxDuty;
			countUp = false;
		}
	} else {
		duty -= increment;
		if(duty <= 0) {
			duty = 0;
			countUp = true;
		}
	}

	HW_pwm.analogWrite(pins[0], duty);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// WIFI not needed for demo. So disabling WIFI.
#ifndef DISABLE_WIFI
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);
#endif

	Serial << "HW PWM maxDuty = " << maxDuty << endl;

	// Setting PWM values on 8 different pins
	HW_pwm.analogWrite(4, maxDuty);
	HW_pwm.analogWrite(5, maxDuty / 2);
	HW_pwm.analogWrite(0, maxDuty);
	HW_pwm.analogWrite(2, maxDuty / 2);
	HW_pwm.analogWrite(15, 0);
	HW_pwm.analogWrite(13, maxDuty / 3);
	HW_pwm.analogWrite(12, 2 * maxDuty / 3);
	HW_pwm.analogWrite(14, maxDuty);

	Serial << _F("PWM output set on all ") << ARRAY_SIZE(pins) << _F(" Pins. Kindly check...") << endl
		   << "Now LED_PIN will go from 0 to VCC to 0 in cycles." << endl;
	procTimer.initializeMs<100>(doPWM).start();
}
