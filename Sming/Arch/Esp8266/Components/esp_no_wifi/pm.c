#include <stdint.h>
#include <espinc/gpio_register.h>
#include <espinc/i2c_bbpll.h>

void ets_delay_us(uint32_t);

// Crystal frequency: 0=40, 1=26, 2=24
extern uint8_t chip6_phy_init_ctrl;

void uart_tx_flush(void)
{
}

/*
 * Returns a calibration factor giving the ratio of system clock ticks to RTC time.
 * NB. The calibration code is from `pm_rtc_clock_cali` in the `phy_sleep` module.
 * To keep things simple we're just rolling it up into one function here.
 */
uint32_t pm_rtc_clock_cali_proc(void)
{
	static uint32_t calibration_value;

	rom_i2c_writeReg(106, 2, 8, 0);

	uint32_t value;
	do {
		value = GPIO_REG_READ(GPIO_RTC_CALIB_VALUE_ADDRESS);
	} while((value & RTC_CALIB_RDY) == 0);

	const uint32_t rtcCalibValue = 0x0101;
	GPIO_REG_WRITE(GPIO_RTC_CALIB_SYNC_ADDRESS, rtcCalibValue);
	GPIO_REG_WRITE(GPIO_RTC_CALIB_SYNC_ADDRESS, rtcCalibValue | RTC_CALIB_START);
	ets_delay_us(10);

	do {
		value = GPIO_REG_READ(GPIO_RTC_CALIB_VALUE_ADDRESS);
	} while((value & RTC_CALIB_RDY) == 0);
	value &= RTC_CALIB_VALUE;

	uint32_t xtal_freq;
	switch(chip6_phy_init_ctrl) {
	case 0:
	case 1:
		xtal_freq = 26;
		break;
	case 2:
		xtal_freq = 24;
		break;
	default:
		xtal_freq = 40;
		break;
	}

	value = value * 16 / xtal_freq;
	if(value < 512) {
		return value;
	}

	if(calibration_value == 0) {
		calibration_value = value;
		return value;
	}

	calibration_value = ((calibration_value * 3) + (value * 5)) / 8;
	return calibration_value;
}
