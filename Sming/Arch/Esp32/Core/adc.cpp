#include <driver/driver_api.h>
#include <esp_adc_cal.h>

#define DEFAULT_VREF 1100 //Use adc2_vref_to_gpio() to obtain a better estimate

uint16_t analogRead(uint16_t pin)
{
	static const adc_atten_t attenuation = ADC_ATTEN_DB_0;
	int8_t channel = digitalPinToAnalogChannel(pin);
	int value = 0;
	esp_adc_cal_characteristics_t adcChars = {};
	adc_unit_t type = ADC_UNIT_1;

	if(channel < 0) {
		debug_e("Pin %u is not ADC pin!", pin);
		return value;
	}

	// Configure
	if(channel > 9) {
		// we have ADC2
		type = ADC_UNIT_2;
		channel -= 10;
		adc2_config_channel_atten((adc2_channel_t)channel, attenuation);
	} else {
		adc1_config_width(ADC_WIDTH_BIT_12);
		adc1_config_channel_atten((adc1_channel_t)channel, attenuation);
	}

	// Characterize
	esp_adc_cal_characterize(type, attenuation, ADC_WIDTH_BIT_12, DEFAULT_VREF, &adcChars);

	// Sample
	if(type == ADC_UNIT_2) {
		esp_err_t r = adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &value);
		if(r == ESP_OK) {
			return value;
		} else if(r == ESP_ERR_INVALID_STATE) {
			debug_e("GPIO%u: %s: ADC2 not initialized yet.", pin, esp_err_to_name(r));
		} else if(r == ESP_ERR_TIMEOUT) {
			debug_e("GPIO%u: %s: ADC2 is in use by Wi-Fi.", pin, esp_err_to_name(r));
		} else {
			debug_e("GPIO%u: %s", pin, esp_err_to_name(r));
		}

		return value;
	}

	return adc1_get_raw((adc1_channel_t)channel);
}
