#include <driver/adc.h>
#include <soc/adc_periph.h>
#include <esp_adc_cal.h>
#include <debug_progmem.h>
#include <Digital.h>

#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate

namespace
{
struct AdcInfo {
	adc_unit_t adc;
	uint8_t channel;
};

bool lookupAdc(uint16_t pin, AdcInfo& info)
{
	for(unsigned adc = 0; adc < SOC_ADC_PERIPH_NUM; ++adc) {
		for(unsigned ch = 0; ch < SOC_ADC_MAX_CHANNEL_NUM; ++ch) {
			if(adc_channel_io_map[adc][ch] == pin) {
				info.adc = adc_unit_t(adc + 1);
				info.channel = ch;
				return true;
			}
		}
	}

	return false;
}

} // namespace

uint16_t analogRead(uint16_t pin)
{
	AdcInfo info;
	if(!lookupAdc(pin, info)) {
		debug_e("Pin %u is not ADC pin!", pin);
		return 0;
	}

	pinMode(pin, ANALOG);

	constexpr adc_atten_t attenuation{ADC_ATTEN_DB_0};
	esp_adc_cal_characteristics_t adcChars{};

	auto adcWidth = adc_bits_width_t(ADC_WIDTH_MAX - 1);

	// Configure
	if(info.adc == ADC_UNIT_2) {
		adc2_config_channel_atten(adc2_channel_t(info.channel), attenuation);
	} else {
		adc1_config_width(adcWidth);
		adc1_config_channel_atten(adc1_channel_t(info.channel), attenuation);
	}

	// Characterize
	esp_adc_cal_characterize(info.adc, attenuation, adcWidth, DEFAULT_VREF, &adcChars);

	// Sample
	if(info.adc == ADC_UNIT_2) {
		int value{0};
		esp_err_t r = adc2_get_raw(adc2_channel_t(info.channel), adcWidth, &value);
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

	return adc1_get_raw(adc1_channel_t(info.channel));
}
