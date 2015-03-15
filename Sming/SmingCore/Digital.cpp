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

		// Default Pull-up
		pullup(pin);

		// Switch to Input or Output
		if (mode == INPUT)
			GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, (1<<pin));
		else
			GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, (1<<pin));
	}
	else if (pin == 16)
	{
		if (mode == INPUT)
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
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	if (pin != 16)
		GPIO_REG_WRITE((((val != 0) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS)), (1<<pin));
	else
		WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xfffffffe) | (uint32)(val & 1));

	//GPIO_OUTPUT_SET(pin, (val ? 0xFF : 00));
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
	PIN_PULLDWN_DIS((EspDigitalPins[pin].mux));
	PIN_PULLUP_EN((EspDigitalPins[pin].mux));
}

void pulldown(uint16_t pin)
{
	if (pin >= 16) return;
	PIN_PULLUP_DIS((EspDigitalPins[pin].mux));
	PIN_PULLDWN_EN((EspDigitalPins[pin].mux));
}

void noPullup(uint16_t pin)
{
	if (pin >= 16) return;
	PIN_PULLDWN_DIS((EspDigitalPins[pin].mux));
	PIN_PULLUP_DIS((EspDigitalPins[pin].mux));
}
