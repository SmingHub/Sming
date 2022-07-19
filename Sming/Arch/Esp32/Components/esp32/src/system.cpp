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

	auto translateReason = [](uint8_t reason) -> uint8_t {
		switch(reason) {
		case ESP_RST_EXT:
		case ESP_RST_BROWNOUT:
			return REASON_EXT_SYS_RST;
		case ESP_RST_PANIC:
			return REASON_EXCEPTION_RST;
		case ESP_RST_INT_WDT:
			return REASON_SOFT_WDT_RST;
		case ESP_RST_TASK_WDT:
		case ESP_RST_WDT:
			return REASON_WDT_RST;
		case ESP_RST_DEEPSLEEP:
			return REASON_DEEP_SLEEP_AWAKE;
		case ESP_RST_SW:
		case ESP_RST_SDIO:
			return REASON_SOFT_RESTART;
		case ESP_RST_UNKNOWN:
		case ESP_RST_POWERON:
		default:
			return REASON_DEFAULT_RST;
		}
	};

	info.reason = translateReason(esp_reset_reason());

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
