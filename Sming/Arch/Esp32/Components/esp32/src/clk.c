#include "include/esp_clk.h"
#include <esp_pm.h>
#include <debug_progmem.h>

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 2, 0)
#define EXPAND(a) a
#define CONCAT3a(a, b, c) a##b##c
#define CONCAT3(a, b, c) CONCAT3a(a, b, c)
typedef CONCAT3(esp_pm_config_, SMING_SOC, _t) esp_pm_config_t;
#endif

int esp_clk_cpu_freq(void);

static esp_pm_lock_handle_t handle;

bool system_update_cpu_freq(uint32_t freq)
{
	esp_pm_config_t config = {};
	esp_err_t err = esp_pm_get_configuration(&config);
	if(err != ESP_OK) {
		debug_e("[PM] Failed to read PM config %u", err);
	} else if((unsigned)config.max_freq_mhz == freq) {
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
	return esp_clk_cpu_freq() / 1000000U;
}
