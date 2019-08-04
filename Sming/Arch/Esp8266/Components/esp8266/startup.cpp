/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * user_main.cpp
 *
 */

#include <user_config.h>
#include "Platform/System.h"
#include <driver/uart.h>
#include <gdb/gdb_hooks.h>
#include <esp_cplusplus.h>

extern void init();

extern "C" void  WEAK_ATTR user_init(void)
{
	// We want high resolution timing - see HardwareTimer class
	system_timer_reinit();

	// Initialise UARTs to a known state
	uart_detach_all();

	/* Note: System is a static class so it's safe to call initialize() before cpp_core_initialize()
	 * We need to do this so that class constructors can use the task queue or onReady()
	 */
	System.initialize();
	cpp_core_initialize();

#ifdef SMING_RELEASE
	// disable all debug output for release builds
	uart_set_debug(UART_NO);
#endif

	gdb_init();

	init(); // User code init
}

// For compatibility with SDK v1.1
extern "C" void WEAK_ATTR user_rf_pre_init(void)
{
	// RTC startup fix, author pvvx
    volatile uint32 * ptr_reg_rtc_ram = (volatile uint32 *)0x60001000;
    if((ptr_reg_rtc_ram[24] >> 16) > 4) {
        ptr_reg_rtc_ram[24] &= 0xFFFF;
        ptr_reg_rtc_ram[30] &= 0;
    }
}

extern "C" uint32 ICACHE_FLASH_ATTR  WEAK_ATTR user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (int(size_map)) {
        case FLASH_SIZE_2M:
            rf_cal_sec = 64 - 5;
            break;

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

        case 8: // FLASH_SIZE_64M_MAP_1024_1024
            rf_cal_sec = 2048 - 5;
            break;

        case 9: // FLASH_SIZE_128M_MAP_1024_1024
            rf_cal_sec = 4096 - 5;
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

extern "C" void ICACHE_FLASH_ATTR WEAK_ATTR user_pre_init(void)
{
	const uint32_t MAX_PROGRAM_SECTORS = 0x100000 / SPI_FLASH_SEC_SIZE; // 1MB addressable

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
			PARTITION_ITEM(SYSTEM_PARTITION_PROGRAM,			2,					std::min(MAX_PROGRAM_SECTORS, rfCalSector) - 2),
			PARTITION_ITEM(SYSTEM_PARTITION_RF_CAL,				rfCalSector,		1),
			PARTITION_ITEM(SYSTEM_PARTITION_PHY_DATA,			rfCalSector + 1,	1),
			PARTITION_ITEM(SYSTEM_PARTITION_SYSTEM_PARAMETER,	rfCalSector + 2,	3),
	};

	enum flash_size_map sizeMap = system_get_flash_size_map();
	if(!system_partition_table_regist(partitions, ARRAY_SIZE(partitions), sizeMap)) {
		os_printf("system_partition_table_regist: failed\n");
		os_printf("size_map = %u\n", sizeMap);
		for (unsigned i = 0; i < ARRAY_SIZE(partitions); ++i) {
			auto& part = partitions[i];
			os_printf("partition[%u]: %u, 0x%08x, 0x%08x\n", i, part.type, part.addr, part.size);
		}
		while(1) {
			// Cannot proceed
		};
	}
}

#endif /* defined(ESP_SDK_VERSION_MAJOR) and ESP_SDK_VERSION_MAJOR>=3 */
