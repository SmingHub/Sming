#include <user_config.h>
#include "Platform/System.h"
#include "HardwareSerial.h"

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

#ifdef SDK_INTERNAL
#include <version.h>
#endif

#if defined(ESP_SDK_VERSION_MAJOR) and ESP_SDK_VERSION_MAJOR>=3

extern "C" void ICACHE_FLASH_ATTR  __attribute__((weak)) user_pre_init(void)
{
	const uint32_t MAX_PROGRAM_SECTORS = 1024; // 1MB addressable

	// WARNING: Sming supports SDK 3.0 with rBoot enabled apps ONLY!
	const partition_type_t SYSTEM_PARTITION_RBOOT_CONFIG = static_cast<partition_type_t>(SYSTEM_PARTITION_CUSTOMER_BEGIN + 0);
	const partition_type_t SYSTEM_PARTITION_PROGRAM = static_cast<partition_type_t>(SYSTEM_PARTITION_CUSTOMER_BEGIN + 1);

	// Partitions offsets and sizes must be in sector multiples, so work in sectors
	#define PARTITION_ITEM(_type, _start, _length) \
		{_type, (_start) * SPI_FLASH_SEC_SIZE, (_length) * SPI_FLASH_SEC_SIZE}

	// Partitions in position order
	uint32_t rfCalSector = user_rf_cal_sector_set();
	static const partition_item_t partitions[] = {
			PARTITION_ITEM(SYSTEM_PARTITION_BOOTLOADER,			0,					1),
			PARTITION_ITEM(SYSTEM_PARTITION_RBOOT_CONFIG,		1,					1),
			PARTITION_ITEM(SYSTEM_PARTITION_PROGRAM,			2,					MAX_PROGRAM_SECTORS - 2),
			PARTITION_ITEM(SYSTEM_PARTITION_RF_CAL,				rfCalSector,		1),
			PARTITION_ITEM(SYSTEM_PARTITION_PHY_DATA,			rfCalSector + 1,	1),
			PARTITION_ITEM(SYSTEM_PARTITION_SYSTEM_PARAMETER,	rfCalSector + 2,	3),
	};

	enum flash_size_map sizeMap = system_get_flash_size_map();
	if(!system_partition_table_regist(partitions, ARRAY_SIZE(partitions), sizeMap)) {
		os_printf(_F("system_partition_table_regist: failed\n"));
		os_printf(_F("size_map = %u\n"), sizeMap);
		for (unsigned i = 0; i < ARRAY_SIZE(partitions); ++i) {
			auto& part = partitions[i];
			os_printf(_F("partition[%u]: %u, 0x%08x, 0x%08x\n"), i, part.type, part.addr, part.size);
		}
		while(1) {
			// Cannot proceed
		};
	}
}

#endif /* defined(ESP_SDK_VERSION_MAJOR) and ESP_SDK_VERSION_MAJOR>=3 */

namespace std {
    void __attribute__((weak)) __throw_bad_function_call()
    {
        while(1);
    };
}
