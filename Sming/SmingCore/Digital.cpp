/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "../SmingCore/Digital.h"
#include "../Wiring/WiringFrameworkIncludes.h"

void pinMode(uint16_t pin, uint8_t mode)
{
	if (pin < 16)
	{
		// Set as GPIO
		PIN_FUNC_SELECT((EspDigitalPins[pin].mux), (EspDigitalPins[pin].gpioFunc));

		// Switch to Input or Output
		if (mode == INPUT || mode == INPUT_PULLUP)
			GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, (1<<pin));
		else
			GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, (1<<pin));
	}
	else if (pin == 16)
	{
		if (mode == INPUT  || mode == INPUT_PULLUP)
		{
		    WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
		                   (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 		// mux configuration for XPD_DCDC and rtc_gpio0 connection

		    WRITE_PERI_REG(RTC_GPIO_CONF,
		                   (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

		    WRITE_PERI_REG(RTC_GPIO_ENABLE,
		                   READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe);				//out disable
		}
		else
		{
			WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
						   (READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xffffffbc) | (uint32)0x1); 		// mux configuration for XPD_DCDC to output rtc_gpio0

			WRITE_PERI_REG(RTC_GPIO_CONF,
						   (READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xfffffffe) | (uint32)0x0);	//mux configuration for out enable

			WRITE_PERI_REG(RTC_GPIO_ENABLE,
						   (READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xfffffffe) | (uint32)0x1); //out enable
		}
	} else
		SYSTEM_ERROR("No pin %d, can't set mode", pin); // NO PIN!

	//Decide to enable or disable the pullup
	if (mode == INPUT_PULLUP)
	{
		pullup(pin);
	}
}

//Detect if pin is input
bool isInputPin(uint16_t pin)
{
	bool result = false;

	if(pin != 16)
	{
		result =((GPIO_REG_READ(GPIO_ENABLE_ADDRESS)>>pin) & 1);
	}
	else
	{
		result = (READ_PERI_REG(RTC_GPIO_ENABLE) & 1);
	}
	return !result;
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	//make compatible with Arduino < version 100
	//enable pullup == setting a pin to input and writing 1 to it
	if (isInputPin(pin))
	{
		if(val == HIGH)
			pullup(pin);
		else
			noPullup(pin);
	}
	else
	{
		if (pin != 16)
			GPIO_REG_WRITE((((val != LOW) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS)), (1<<pin));
		else
			WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(val & 1));

		//GPIO_OUTPUT_SET(pin, (val ? 0xFF : 00));
	}
}

uint8_t digitalRead(uint16_t pin)
{
	if (pin != 16)
		return ((GPIO_REG_READ(GPIO_IN_ADDRESS)>>pin) & 1);
	else
		return (uint8)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);

	//return  GPIO_INPUT_GET(pin);
}

void pullup(uint16_t pin)
{
	if (pin >= 16) return;
	PIN_PULLUP_EN((EspDigitalPins[pin].mux));
}

void noPullup(uint16_t pin)
{
	if (pin >= 16) return;
	PIN_PULLUP_DIS((EspDigitalPins[pin].mux));
}

/* Measures the length (in microseconds) of a pulse on the pin; state is HIGH
 * or LOW, the type of pulse to measure.  Works on pulses from 2-3 microseconds
 * to 3 minutes in length, but must be called at least a few dozen microseconds
 * before the start of the pulse. */
unsigned long pulseIn(uint16_t pin, uint8_t state, unsigned long timeout)
{
	// cache the port and bit of the pin in order to speed up the
	// pulse width measuring loop and achieve finer resolution.  calling
	// digitalRead() instead yields much coarser resolution.
	uint8_t bit = digitalPinToBitMask(pin);
	uint8_t port = digitalPinToPort(pin);
	uint8_t stateMask = (state ? bit : 0);
	unsigned long width = 0; // keep initialization out of time critical area

	// convert the timeout from microseconds to a number of times through
	// the initial loop; it takes 16 clock cycles per iteration.
	unsigned long numloops = 0;
	unsigned long maxloops = microsecondsToClockCycles(timeout) / 16;

	// wait for any previous pulse to end
	 while ((*portInputRegister(port) & bit) == stateMask)
	 if (numloops++ == maxloops)
	 return 0;

	 // wait for the pulse to start
	 while ((*portInputRegister(port) & bit) != stateMask)
	 if (numloops++ == maxloops)
	 return 0;

	 // wait for the pulse to stop
	 while ((*portInputRegister(port) & bit) == stateMask) {
	 if (numloops++ == maxloops)
	 return 0;
	 width++;
	 }

	 // convert the reading to microseconds. The loop has been determined
	 // to be 20 clock cycles long and have about 16 clocks between the edge
	 // and the start of the loop. There will be some error introduced by
	 // the interrupt handlers.

	return clockCyclesToMicroseconds(width * 21 + 16);
}

