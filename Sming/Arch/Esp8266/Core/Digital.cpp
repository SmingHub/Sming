/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Digital.cpp
 *
 ****/

#include <Digital.h>
#include <espinc/eagle_soc.h>
#include <espinc/gpio_register.h>
#include <espinc/pin_mux_register.h>
#include <Platform/Timers.h>

#define TOTAL_PINS 16

#define PINMUX_OFFSET(addr) uint8_t((addr)-PERIPHS_IO_MUX)

// Used for pullup/noPullup
extern const uint8_t esp8266_pinmuxOffset[] = {
	PINMUX_OFFSET(PERIPHS_IO_MUX_GPIO0_U),	// 0 FLASH
	PINMUX_OFFSET(PERIPHS_IO_MUX_U0TXD_U),	// 1 TXD0
	PINMUX_OFFSET(PERIPHS_IO_MUX_GPIO2_U),	// 2 TXD1
	PINMUX_OFFSET(PERIPHS_IO_MUX_U0RXD_U),	// 3 RXD0
	PINMUX_OFFSET(PERIPHS_IO_MUX_GPIO4_U),	// 4
	PINMUX_OFFSET(PERIPHS_IO_MUX_GPIO5_U),	// 5
	PINMUX_OFFSET(PERIPHS_IO_MUX_SD_CLK_U),   // 6 SD_CLK_U
	PINMUX_OFFSET(PERIPHS_IO_MUX_SD_DATA0_U), // 7 SD_DATA0_U
	PINMUX_OFFSET(PERIPHS_IO_MUX_SD_DATA1_U), // 8 SD_DATA1_U
	PINMUX_OFFSET(PERIPHS_IO_MUX_SD_DATA2_U), // 9
	PINMUX_OFFSET(PERIPHS_IO_MUX_SD_DATA3_U), // 10
	PINMUX_OFFSET(PERIPHS_IO_MUX_SD_CMD_U),   // 11 SD_CMD_U
	PINMUX_OFFSET(PERIPHS_IO_MUX_MTDI_U),	 // 12 HSPIQ
	PINMUX_OFFSET(PERIPHS_IO_MUX_MTCK_U),	 // 13 HSPID
	PINMUX_OFFSET(PERIPHS_IO_MUX_MTMS_U),	 // 14 HSPICLK
	PINMUX_OFFSET(PERIPHS_IO_MUX_MTDO_U),	 // 15 HSPICS
};

// Prototype declared in esp8266-peri.h
const uint8_t esp8266_gpioToFn[TOTAL_PINS] = {0x34, 0x18, 0x38, 0x14, 0x3C, 0x40, 0x1C, 0x20,
											  0x24, 0x28, 0x2C, 0x30, 0x04, 0x08, 0x0C, 0x10};

void pinMode(uint16_t pin, uint8_t mode)
{
	if(pin < TOTAL_PINS) {
		if(mode == SPECIAL) {
			GPC(pin) = (GPC(pin) &
						(0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
			GPEC = (1 << pin);			//Disable
			GPF(pin) = GPFFS(GPFFS_BUS(pin)); //Set mode to BUS (RX0, TX0, TX1, SPI, HSPI or CLK depending in the pin)
			if(pin == 3)
				GPF(pin) |= (1 << GPFPU); //enable pullup on RX
		} else if(mode & FUNCTION_0) {
			GPC(pin) = (GPC(pin) &
						(0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
			GPEC = (1 << pin);			//Disable
			GPF(pin) = GPFFS((mode >> 4) & 0x07);
			if(pin == 13 && mode == FUNCTION_4)
				GPF(pin) |= (1 << GPFPU); //enable pullup on RX
		} else if(mode == OUTPUT || mode == OUTPUT_OPEN_DRAIN) {
			GPF(pin) = GPFFS(GPFFS_GPIO(pin)); //Set mode to GPIO
			GPC(pin) = (GPC(pin) &
						(0xF << GPCI)); //SOURCE(GPIO) | DRIVER(NORMAL) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
			if(mode == OUTPUT_OPEN_DRAIN)
				GPC(pin) |= (1 << GPCD);
			GPES = (1 << pin); //Enable
		} else if(mode == INPUT || mode == INPUT_PULLUP) {
			GPF(pin) = GPFFS(GPFFS_GPIO(pin)); //Set mode to GPIO
			GPEC = (1 << pin);				   //Disable
			GPC(pin) = (GPC(pin) & (0xF << GPCI)) |
					   (1 << GPCD); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(UNCHANGED) | WAKEUP_ENABLE(DISABLED)
			if(mode == INPUT_PULLUP) {
				GPF(pin) |= (1 << GPFPU); // Enable  Pullup
			}
		} else if(mode == WAKEUP_PULLUP || mode == WAKEUP_PULLDOWN) {
			GPF(pin) = GPFFS(GPFFS_GPIO(pin)); //Set mode to GPIO
			GPEC = (1 << pin);				   //Disable
			if(mode == WAKEUP_PULLUP) {
				GPF(pin) |= (1 << GPFPU); // Enable  Pullup
				GPC(pin) = (1 << GPCD) | (4 << GPCI) |
						   (1 << GPCWE); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(LOW) | WAKEUP_ENABLE(ENABLED)
			} else {
				GPF(pin) |= (1 << GPFPD); // Enable  Pulldown
				GPC(pin) = (1 << GPCD) | (5 << GPCI) |
						   (1 << GPCWE); //SOURCE(GPIO) | DRIVER(OPEN_DRAIN) | INT_TYPE(HIGH) | WAKEUP_ENABLE(ENABLED)
			}
		}
	} else if(pin == 16) {
		GPF16 = GP16FFS(GPFFS_GPIO(pin)); //Set mode to GPIO
		GPC16 = 0;
		if(mode == INPUT || mode == INPUT_PULLDOWN_16) {
			if(mode == INPUT_PULLDOWN_16) {
				GPF16 |= (1 << GP16FPD); //Enable Pulldown
			}
			GP16E &= ~1;
		} else if(mode == OUTPUT) {
			GP16E |= 1;
		}
	}
}

//Detect if pin is input
bool isInputPin(uint16_t pin)
{
	bool result = false;

	if(pin != 16) {
		result = ((GPIO_REG_READ(GPIO_ENABLE_ADDRESS) >> pin) & 1);
	} else {
		result = (READ_PERI_REG(RTC_GPIO_ENABLE) & 1);
	}
	return !result;
}

void digitalWrite(uint16_t pin, uint8_t val)
{
	//make compatible with Arduino < version 100
	//enable pullup == setting a pin to input and writing 1 to it
	if(isInputPin(pin)) {
		if(val == HIGH)
			pullup(pin);
		else
			noPullup(pin);
	} else if(pin != 16)
		GPIO_REG_WRITE((((val != LOW) ? GPIO_OUT_W1TS_ADDRESS : GPIO_OUT_W1TC_ADDRESS)), (1 << pin));
	else
		WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT) & (uint32_t)0xfffffffe) | (uint32_t)(val & 1));
}

uint8_t digitalRead(uint16_t pin)
{
	if(pin != 16) {
		return ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> pin) & 1);
	}
	return (uint8_t)(READ_PERI_REG(RTC_GPIO_IN_DATA) & 1);
}

void pullup(uint16_t pin)
{
	if(pin < TOTAL_PINS) {
		PIN_PULLUP_EN(PERIPHS_IO_MUX + esp8266_pinmuxOffset[pin]);
	}
}

void noPullup(uint16_t pin)
{
	if(pin < TOTAL_PINS) {
		PIN_PULLUP_DIS(PERIPHS_IO_MUX + esp8266_pinmuxOffset[pin]);
	}
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
	uint32_t bit = digitalPinToBitMask(pin);
	uint32_t stateMask = (state ? bit : 0);

	auto pinState = [&]() -> bool { return (*portInputRegister() & bit) == stateMask; };

	OneShotFastUs timeoutTimer(timeout);

	// wait for any previous pulse to end
	while(pinState()) {
		if(timeoutTimer.expired()) {
			return 0;
		}
	}

	// wait for the pulse to start
	while(!pinState()) {
		if(timeoutTimer.expired()) {
			return 0;
		}
	}

	CpuCycleTimer cycleTimer;

	// wait for the pulse to stop
	while(pinState()) {
		if(timeoutTimer.expired()) {
			return 0;
		}
	}

	return cycleTimer.elapsedTime().as<NanoTime::Microseconds>();
}
