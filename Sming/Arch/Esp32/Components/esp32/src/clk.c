#include "include/esp_clk.h"
#include <esp_pm.h>
#include <debug_progmem.h>

#if CONFIG_IDF_TARGET_ESP32
typedef esp_pm_config_esp32_t pm_config_t;
#elif CONFIG_IDF_TARGET_ESP32C3
typedef esp_pm_config_esp32c3_t pm_config_t;
#elif CONFIG_IDF_TARGET_ESP32S2
typedef esp_pm_config_esp32s2_t pm_config_t;
#elif CONFIG_IDF_TARGET_ESP32S3
typedef esp_pm_config_esp32s3_t pm_config_t;
#endif

int esp_clk_cpu_freq(void);

static esp_pm_lock_handle_t handle;

bool system_update_cpu_freq(uint32_t freq)
{
	pm_config_t config = {};
	esp_err_t err = esp_pm_get_configuration(&config);
	if(err != ESP_OK) {
		debug_e("[PM] Failed to read PM config %u", err);
	} else if(config.max_freq_mhz == freq) {
		return true;
	}

	if(handle == NULL) {
		esp_err_t err = esp_pm_lock_create(ESP_PM_CPU_FREQ_MAX, 0, "sming", &handle);
		if(err != ESP_OK) {
			debug_e("[PM] Failed to create lock");
			return false;
		}
	} else {
		// Reverts PM to default settings
		esp_pm_lock_release(handle);
	}

	config.max_freq_mhz = config.min_freq_mhz = freq;
	err = esp_pm_configure(&config);
	if(err != ESP_OK) {
		debug_e("[PM] Failed to set CPU to %u MHz", freq);
		m_printHex("CFG", &config, sizeof(config), -1, 16);
		return false;
	}

	err = esp_pm_lock_acquire(handle);
	if(err != ESP_OK) {
		debug_e("[PM] Failed to lock CPU to %u MHz", freq);
		return false;
	}

	return true;
}

uint32_t system_get_cpu_freq(void)
{
	return esp_clk_cpu_freq() / MHZ;
}
