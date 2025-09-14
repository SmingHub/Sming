#include <driver/adc.h>
#include <debug_progmem.h>

uint16_t analogRead(uint16_t pin)
{
	adc_oneshot_unit_init_cfg_t init_config{};
	adc_channel_t channel;
	esp_err_t err = adc_oneshot_io_to_channel(pin, &init_config.unit_id, &channel);
	if(err != ESP_OK) {
		debug_e("Pin %u is not ADC pin!", pin);
		return 0;
	}

	// Initialise unit
	adc_oneshot_unit_handle_t adc_handle;
	ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

	// Initialise channel
	adc_oneshot_chan_cfg_t channel_config{
		.atten = ADC_ATTEN_DB_0,
		.bitwidth = ADC_BITWIDTH_DEFAULT,
	};
	ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, channel, &channel_config));

	int rawSampleValue{0};
	ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, channel, &rawSampleValue));

	ESP_ERROR_CHECK(adc_oneshot_del_unit(adc_handle));

	return rawSampleValue;
}
