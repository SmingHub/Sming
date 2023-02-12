#include <driver/adc.h>
#include <soc/adc_periph.h>
#if ESP_IDF_VERSION_MAJOR < 5
#include <esp_adc_cal.h>
#endif
#include <debug_progmem.h>
#include <Digital.h>

#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate

namespace
{
struct AdcInfo {
	adc_unit_t adc;
	adc_channel_t channel;
};

bool lookupAdc(uint16_t pin, AdcInfo& info)
{
	for(unsigned adc = 0; adc < SOC_ADC_PERIPH_NUM; ++adc) {
		for(unsigned ch = 0; ch < SOC_ADC_MAX_CHANNEL_NUM; ++ch) {
			if(adc_channel_io_map[adc][ch] == pin) {
				info.adc = adc_unit_t(adc + 1);
				info.channel = adc_channel_t(ch);
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

#if ESP_IDF_VERSION_MAJOR < 5

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

#else

	// Initialise unit
	adc_oneshot_unit_init_cfg_t init_config{
		.unit_id = info.adc,
	};
	adc_oneshot_unit_handle_t adc_handle;
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

	// Initialise channel
	adc_oneshot_chan_cfg_t channel_config{
		.atten = ADC_ATTEN_DB_0,
		.bitwidth = ADC_BITWIDTH_DEFAULT,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, info.channel, &channel_config));

	// Calibration
	// adc_cali_handle_t adc1_cali_handle = NULL;
	// bool do_calibration1 = example_adc_calibration_init(ADC_UNIT_1, ADC_ATTEN_DB_11, &adc1_cali_handle);

	int rawSampleValue{0};
	ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, info.channel, &rawSampleValue));

	// ESP_ERROR_CHECK(adc_cali_raw_to_voltage(adc1_cali_handle, adc_raw[0][0], &voltage[0][0]));
	// ESP_LOGI(TAG, "ADC%d Channel[%d] Cali Voltage: %d mV", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, voltage[0][0]);

	ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));

	return rawSampleValue;

#endif
}
