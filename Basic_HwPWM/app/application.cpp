/*
 * File:   Esp SDK pwm demo
 * Author: https://github.com/hrsavla
 *
 * This ESP_PWM library enables Sming framework user to use ESP SDK PWM API
 * Period of PWM is fixed to 1000ms / Frequency = 1khz
 * Duty at 100% = 22222. Duty at 0% = 0
 * You can use function setPeriod() to change frequency/period.
 * Calculate the Duty as per the formulae give in ESP8266 SDK
 * Duty = (Period *1000)/45
 *
 * PWM can be generated on upto  8 pins (ie All pins except pin 16)
 * Created on August 17, 2015, 2:27 PM
 */
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <ESP_PWM.h>

uint8_t pins[8] = { 4, 5, 0, 2, 15, 13, 12, 14 }; // List of pins that you want to connect to pwm
ESPPWM Esp_pwm(pins, 8);

Timer procTimer;
uint32 i = 0;
uint32 inc = 100;
bool countUp = true;
void doPWM()
{
	if(countUp == true)
	{
		i = i+ inc;
		if (i >22222)
		{
			countUp = false;
		}
	}
	else
	{
		i = i- inc;
		if (i < inc)
		{
			countUp = true;
		}
	}
	Esp_pwm.analogWrite(2,i);
}
void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// WIFI not needed for demo. So disabling WIFI.
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	// Setting PWM values on 8 different pins
	Esp_pwm.analogWrite(4, 22222);
	Esp_pwm.analogWrite(5, 11111);
	Esp_pwm.analogWrite(0, 22222);
	Esp_pwm.analogWrite(2, 11111);
	Esp_pwm.analogWrite(15, 22222);
	Esp_pwm.analogWrite(13, 11111);
	Esp_pwm.analogWrite(12, 22222);
	Esp_pwm.analogWrite(14, 11111);

	debugf("PWM output set on 8 Pins. Kindly check ..");
	debugf("Now Pin 2 will go from 0 to VCC to 0 in cycles.");
	procTimer.initializeMs(100, doPWM).start();
}



