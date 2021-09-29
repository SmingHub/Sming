#include "include/esp_system.h"
#include <driver/hw_timer.h>
#include <hardware/sync.h>
#include <hardware/watchdog.h>
#include <hardware/structs/watchdog.h>
#include <pico/unique_id.h>

namespace
{
constexpr uint32_t WATCHDOG_PERIOD_MS{8000};
constexpr uint32_t SYSTEM_RESTART_DELAY{250};

uint32_t savedIrqLevels[2];

}; // namespace

void system_soft_wdt_stop()
{
	hw_clear_bits(&watchdog_hw->ctrl, WATCHDOG_CTRL_ENABLE_BITS);
}

void system_soft_wdt_restart()
{
	constexpr uint32_t ticks = WATCHDOG_PERIOD_MS * 1000 * 2;
	static_assert(ticks <= 0xffffff);

	watchdog_enable(WATCHDOG_PERIOD_MS, true);
}

void system_soft_wdt_feed()
{
	watchdog_update();
}

void interrupts()
{
	restore_interrupts(savedIrqLevels[get_core_num()]);
}

uint32_t noInterrupts()
{
	auto level = save_and_disable_interrupts();
	savedIrqLevels[get_core_num()] = level;
	return level;
}

void restoreInterrupts(uint32_t level)
{
	restore_interrupts(level);
}

uint32_t system_get_time()
{
	return hw_timer2_read();
}

void system_restart()
{
	// Force a restart
	watchdog_hw->ctrl = WATCHDOG_CTRL_TRIGGER_BITS;
}

struct rst_info* system_get_rst_info()
{
	static struct rst_info info;
	auto reason = watchdog_hw->reason;
	if(reason & WATCHDOG_REASON_FORCE_BITS) {
		info.reason = REASON_SOFT_RESTART;
	} else if(reason != 0) {
		info.reason = REASON_SOFT_WDT_RST;
	} else {
		info.reason = REASON_DEFAULT_RST;
	}
	return &info;
}

void os_delay_us(uint32_t us)
{
	auto start = hw_timer2_read();
	while(hw_timer2_read() - start < us) {
		// Wait
	}
}

const char* system_get_sdk_version()
{
	return "PICO_SDK_" PICO_SDK_VERSION_STRING;
}

uint32_t system_get_chip_id()
{
	pico_unique_board_id_t id;
	pico_get_unique_board_id(&id);
	uint32_t res;
	memcpy(&res, &id, sizeof(res));
	return res;
}
