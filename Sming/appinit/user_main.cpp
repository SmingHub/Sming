#include <user_config.h>
#include "../SmingCore/SmingCore.h"

#ifndef SMING_RELEASE
extern "C" {
  void gdbstub_init();
}
#endif

extern void init();

extern "C" void  __attribute__((weak)) user_init(void)
{
	system_timer_reinit();
	uart_div_modify(UART_ID_0, UART_CLK_FREQ / SERIAL_BAUD_RATE);
	cpp_core_initialize();
	System.initialize();
#ifdef SMING_RELEASE
	// disable all debug output for release builds
	Serial.systemDebugOutput(false);
	system_set_os_print(0);
#else
	gdbstub_init();
#endif
	init(); // User code init
}

// For compatibility with SDK v1.1
extern "C" void __attribute__((weak)) user_rf_pre_init(void)
{
	uart_div_modify(UART_ID_0, UART_CLK_FREQ / SERIAL_BAUD_RATE);
	// RTC startup fix, author pvvx
    volatile uint32 * ptr_reg_rtc_ram = (volatile uint32 *)0x60001000;
    if((ptr_reg_rtc_ram[24] >> 16) > 4) {
        ptr_reg_rtc_ram[24] &= 0xFFFF;
        ptr_reg_rtc_ram[30] &= 0;
    }
}

extern "C" uint32 ICACHE_FLASH_ATTR  __attribute__((weak)) user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}
