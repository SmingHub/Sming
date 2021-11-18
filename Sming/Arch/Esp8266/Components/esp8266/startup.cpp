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
#include <gdb/gdb_hooks.h>
#include <esp_cplusplus.h>
#include <Storage.h>
#include <spi_flash.h>

extern void init();

extern "C" void user_init(void)
{
	// Initialise hardware timers
	hw_timer_init();

	// Initialise UARTs to a known state
	smg_uart_detach_all();

	/* Note: System is a static class so it's safe to call initialize() before cpp_core_initialize()
	 * We need to do this so that class constructors can use the task queue or onReady()
	 */
	System.initialize();
	cpp_core_initialize();

#ifdef SMING_RELEASE
	// disable all debug output for release builds
	smg_uart_set_debug(UART_NO);
#endif

	gdb_init();

	Storage::initialize();

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
	auto rfCal = *Storage::findPartition(Storage::Partition::SubType::Data::rfCal);
	return rfCal.address();
}

#ifdef SDK_INTERNAL
#include <version.h>
#endif

#if defined(ESP_SDK_VERSION_MAJOR) and ESP_SDK_VERSION_MAJOR>=3

extern "C" void ICACHE_FLASH_ATTR WEAK_ATTR user_pre_init(void)
{
	Storage::initialize();

	auto sysParam = *Storage::findPartition(Storage::Partition::SubType::Data::sysParam);
	auto rfCal = *Storage::findPartition(Storage::Partition::SubType::Data::rfCal);
	auto phy = *Storage::findPartition(Storage::Partition::SubType::Data::phy);

	static const partition_item_t partitions[] = {
			{SYSTEM_PARTITION_BOOTLOADER,		0,						SPI_FLASH_SEC_SIZE},
			{SYSTEM_PARTITION_PHY_DATA,			phy.address(),			phy.size()},
			{SYSTEM_PARTITION_SYSTEM_PARAMETER,	sysParam.address(),		sysParam.size()},
			{SYSTEM_PARTITION_RF_CAL,			rfCal.address(),		rfCal.size()},
	};

	enum flash_size_map sizeMap = system_get_flash_size_map();
	if(!system_partition_table_regist(partitions, ARRAY_SIZE(partitions), sizeMap)) {
		os_printf("system_partition_table_regist: failed\n");
		os_printf("size_map = %u\n", sizeMap);
		for (unsigned i = 0; i < ARRAY_SIZE(partitions); ++i) {
			auto& part = partitions[i];
			os_printf("partition[%u]: %u, 0x%08x, 0x%08x\n", i, part.type, part.addr, part.size);
		}
		if(sizeMap < FLASH_SIZE_8M_MAP_512_512) {
			os_printf("** Note: SDK 3.0.1 requires spiFlash size >= 1M\n");
		}
		while(1) {
			// Cannot proceed
		};
	}
}

#endif /* defined(ESP_SDK_VERSION_MAJOR) and ESP_SDK_VERSION_MAJOR>=3 */
