/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * user_main.cpp
 *
 */

#include "Platform/System.h"
#include <driver/uart.h>
#include <driver/hw_timer.h>
#include "include/esp_tasks_ll.h"
#include <gdb/gdb_hooks.h>
#include <Storage.h>
#include <hardware/structs/ioqspi.h>
#include <pico/bootrom.h>

extern void init();
extern void hw_timer_init();
extern void system_init_timers();
extern void system_service_timers();
extern void rp2040_network_initialise();
extern void rp2040_network_service();

namespace
{
#ifdef PICO_DEFAULT_LED_PIN
#define PICO_HDD_ACTIVITY_LED_PINMASK BIT(PICO_DEFAULT_LED_PIN)
#else
#define PICO_HDD_ACTIVITY_LED_PINMASK 0
#endif

#ifdef ENABLE_BOOTSEL
uint32_t last_bootsel_check;

/*
 * Read state of BOOTSEL switch on GPIO 1 (QSPI CS #0).
 *
 * https://github.com/raspberrypi/pico-examples/blob/master/picoboard/button/button.c
 *
 * TODO: Synchronise with second core if running code from flash.
 */
bool __noinline IRAM_ATTR get_bootsel_button()
{
	const unsigned CS_PIN_INDEX{1};

	noInterrupts();

	// Set chip select to Hi-Z
	hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl, GPIO_OVERRIDE_LOW << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
					IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

	// Note we can't call into any sleep functions in flash right now
	for(volatile int i = 0; i < 1000; ++i) {
		//
	}

	// The HI GPIO registers in SIO can observe and control the 6 QSPI pins.
	// Note the button pulls the pin *low* when pressed.
#ifdef SOC_RP2040
#define CS_BIT (1u << 1)
#else
#define CS_BIT SIO_GPIO_HI_IN_QSPI_CSN_BITS
#endif
	bool button_state = !(sio_hw->gpio_hi_in & CS_BIT);

	// Re-enable chip select
	hw_write_masked(&ioqspi_hw->io[CS_PIN_INDEX].ctrl, GPIO_OVERRIDE_NORMAL << IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_LSB,
					IO_QSPI_GPIO_QSPI_SS_CTRL_OEOVER_BITS);

	interrupts();

	return button_state;
}

void check_bootsel()
{
	constexpr uint32_t sampleInterval{HW_TIMER2_CLK / 2};
	auto ticks = hw_timer2_read();
	auto elapsed = ticks - last_bootsel_check;
	if(elapsed < sampleInterval) {
		return;
	}
	last_bootsel_check = ticks;

	if(get_bootsel_button()) {
		reset_usb_boot(PICO_HDD_ACTIVITY_LED_PINMASK, 0);
	}
}

#endif // ENABLE_BOOTSEL

} // namespace

extern "C" int main(void)
{
	extern void system_init_clocks();
	system_init_clocks();

	system_soft_wdt_restart();

	// Initialise hardware timers
	hw_timer_init();

	system_init_tasks();
	system_init_timers();

	// Initialise UARTs to a known state
	smg_uart_detach_all();

	/* Note: System is a static class so it's safe to call initialize() before cpp_core_initialize()
	 * We need to do this so that class constructors can use the task queue or onReady()
	 */
	System.initialize();

#ifdef SMING_RELEASE
	// disable all debug output for release builds
	smg_uart_set_debug(UART_NO);
#endif

	// gdb_init();

	Storage::initialize();

#ifndef DISABLE_NETWORK
	rp2040_network_initialise();
#endif

	init(); // User code init

	while(true) {
		system_soft_wdt_feed();
		system_service_tasks();
		system_service_timers();
#ifndef DISABLE_NETWORK
		rp2040_network_service();
#endif
#ifdef ENABLE_BOOTSEL
		check_bootsel();
#endif
	}

	return 0;
}
