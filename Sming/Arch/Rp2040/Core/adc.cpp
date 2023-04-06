#include <Digital.h>
#include <hardware/adc.h>
#include <bitset>

#define PIN_ADC0 26
#define PIN_ADC1 27
#define PIN_ADC2 28
#define PIN_ADC3 29
#define PIN_TEMP 30 // Not a GPIO
#define ADC_TEMP 4

namespace
{
std::bitset<NUM_ADC_CHANNELS> initflags;
}

uint16_t analogRead(uint16_t pin)
{
	if(pin < PIN_ADC0 || pin > PIN_TEMP) {
		// Not an analogue pin
		return 0;
	}

	uint8_t channel = pin - PIN_ADC0;
	if((adc_hw->cs & ADC_CS_EN_BITS) == 0) {
		adc_init();
	}
	if(!initflags[channel]) {
		if(channel == ADC_TEMP) {
			adc_set_temp_sensor_enabled(true);
		} else {
			adc_gpio_init(pin);
		}
		initflags[channel] = true;
	}

	adc_select_input(channel);
	return adc_read();
}
