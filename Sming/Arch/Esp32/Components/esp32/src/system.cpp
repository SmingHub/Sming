#include <esp_system.h>
#include <sys/time.h>
#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <sming_attr.h>
#include <string.h>
#if ESP_IDF_VERSION_MAJOR >= 5
#include <esp_mac.h>
#endif

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
	/*
	 * Internally, `esp_restart` calls `esp_wifi_stop`.
	 * This must not be called in the context of the task which handles the main event queue.
	 * i.e. The Sming main thread.
	 * Doing so causes a deadlock and a task watchdog timeout.
	 *
	 * Delegating this call to any other task fixes the issue.
	 *
	 * We can use the timer task to handle the call.
	 * This method does not free the allocated timer resources but as the system
	 * is restarting this doesn't matter.
	 */
	const esp_timer_create_args_t create_args = {
		.callback = esp_timer_cb_t(esp_restart),
		.dispatch_method = ESP_TIMER_TASK,
	};
	esp_timer_handle_t handle{nullptr};
	esp_timer_create(&create_args, &handle);
	esp_timer_start_once(handle, 100);
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
