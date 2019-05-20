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
 * PWM can be generated on upto 8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 *
 * See also ESP8266 Technical Reference, Chapter 12:
 * http://espressif.com/sites/default/files/documentation/esp8266-technical_reference_en.pdf
 */
#include <SmingCore.h>
#include <HardwarePWM.h>

uint8_t pins[8] = {4, 5, 0, 2, 15, 13, 12, 14}; // List of pins that you want to connect to pwm
HardwarePWM HW_pwm(pins, 8);

Timer procTimer;
int32 i = 0;
bool countUp = true;

int maxDuty = HW_pwm.getMaxDuty();
int32 inc = maxDuty / 50;

void doPWM()
{
	if(countUp == true) {
		i += inc;
		if(i >= maxDuty) {
			i = maxDuty;
			countUp = false;
		}
	} else {
		i -= inc;
		if(i <= 0) {
			i = 0;
			countUp = true;
		}
	}
	HW_pwm.analogWrite(2, i);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// WIFI not needed for demo. So disabling WIFI.
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	// Setting PWM values on 8 different pins
	HW_pwm.analogWrite(4, maxDuty);
	HW_pwm.analogWrite(5, maxDuty / 2);
	HW_pwm.analogWrite(0, maxDuty);
	HW_pwm.analogWrite(2, maxDuty / 2);
	HW_pwm.analogWrite(15, 0);
	HW_pwm.analogWrite(13, maxDuty / 3);
	HW_pwm.analogWrite(12, 2 * maxDuty / 3);
	HW_pwm.analogWrite(14, maxDuty);

	debugf("PWM output set on all 8 Pins. Kindly check...");
	debugf("Now Pin 2 will go from 0 to VCC to 0 in cycles.");
	procTimer.initializeMs(100, doPWM).start();
}
