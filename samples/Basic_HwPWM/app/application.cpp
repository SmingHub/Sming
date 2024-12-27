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
const uint8_t pins[]
{
#if defined(ARCH_ESP32)
#define LED_PIN 3
	LED_PIN, 4, 5, 18, 19, 4,
#elif defined(ARCH_RP2040)
#define LED_PIN PICO_DEFAULT_LED_PIN
	LED_PIN, 2, 3, 4, 5, 6, 7, 8,
#else
#define LED_PIN 2
	LED_PIN, 4, 5, 0, 15, 13, 12, 14,
#endif
};

const uint8_t defaultDutyPercent[]{
	50, 95, 50, 85, 10, 30, 60, 80,
};

HardwarePWM pwm(pins, ARRAY_SIZE(pins));
uint32_t maxDuty;

SimpleTimer procTimer;

void doPWM()
{
	static bool countUp = true;
	static uint32_t duty;

	const uint32_t increment = maxDuty / 50;

	if(countUp) {
		duty += increment;
		if(duty >= maxDuty) {
			duty = maxDuty;
			countUp = false;
		}
	} else if(duty <= increment) {
		duty = 0;
		countUp = true;
	} else {
		duty -= increment;
	}

	pwm.analogWrite(LED_PIN, duty);
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

	// Change PWM frequency if required: period is in microseconds
	// pwm.setPeriod(100);

	maxDuty = pwm.getMaxDuty();
	auto period = pwm.getPeriod();
	auto freq = pwm.getFrequency(LED_PIN);

	Serial << _F("PWM period = ") << period << _F("us, freq = ") << freq << _F(", max. duty = ") << maxDuty << endl;

	// Set default PWM values
	for(unsigned i = 0; i < ARRAY_SIZE(pins); ++i) {
		pwm.analogWrite(pins[i], maxDuty * defaultDutyPercent[i] / 100);
	}

	Serial << _F("PWM output set on all ") << ARRAY_SIZE(pins) << _F(" Pins. Kindly check...") << endl
		   << _F("Now LED (pin ") << LED_PIN << _F(") will go from 0 to VCC to 0 in cycles.") << endl;
	procTimer.initializeMs<100>(doPWM).start();
}
