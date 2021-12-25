#include <esp_system.h>
#include <sys/time.h>
#include <esp_task_wdt.h>
#include <sming_attr.h>

extern "C" int64_t esp_system_get_time();

uint32_t system_get_time(void)
{
	return esp_system_get_time();
}

struct rst_info* system_get_rst_info(void)
{
	static rst_info info{};
	auto reason = esp_reset_reason();
	switch(reason) {
	case ESP_RST_INT_WDT:
		info.reason = REASON_SOFT_WDT_RST;
		break;
	case ESP_RST_BROWNOUT:
		info.reason = REASON_WDT_RST;
		break;
	case ESP_RST_SDIO:
		info.reason = REASON_SOFT_RESTART;
		break;
	default:
		info.reason = reason;
	}

	return &info;
}

void system_restart(void)
{
	esp_restart();
}

/* Watchdog */

void system_soft_wdt_feed(void)
{
	esp_task_wdt_reset();
}

void system_soft_wdt_stop(void)
{
}

void system_soft_wdt_restart(void)
{
}

/* Misc */

const char* system_get_sdk_version(void)
{
	static const char version_string[] = "ESP-IDF v" IDF_VER;
	return version_string;
}

uint32_t system_get_chip_id()
{
	uint8_t baseMac[6];
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	uint32_t id;
	// get the last 4 bytes. they should be unique per device
	memcpy(&id, &baseMac[2], sizeof(id));
	return id;
}

/*
 * Building without WiFi sometimes leaves these functions undefined.
 */

unsigned long WEAK_ATTR os_random(void)
{
	return esp_random();
}

int WEAK_ATTR os_get_random(unsigned char* buf, size_t len)
{
	esp_fill_random(buf, len);
	return 0;
}
