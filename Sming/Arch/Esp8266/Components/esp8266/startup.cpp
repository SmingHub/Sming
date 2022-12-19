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
#include <Storage.h>
#include <spi_flash.h>

extern void init();
extern void cpp_core_initialize();

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

extern "C" void ICACHE_FLASH_ATTR WEAK_ATTR user_pre_init(void)
{
	Storage::initialize();

	using PartType = Storage::Partition::SubType::Data;
	auto sysParam = *Storage::findPartition(PartType::sysParam);
	auto rfCal = *Storage::findPartition(PartType::rfCal);
	auto phy = *Storage::findPartition(PartType::phy);

	static const partition_item_t partitions[]{
		{
			SYSTEM_PARTITION_BOOTLOADER,
			0,
			SPI_FLASH_SEC_SIZE,
		},
		{
			SYSTEM_PARTITION_PHY_DATA,
			uint32_t(phy.address()),
			uint32_t(phy.size()),
		},
		{
			SYSTEM_PARTITION_SYSTEM_PARAMETER,
			uint32_t(sysParam.address()),
			uint32_t(sysParam.size()),
		},
		{
			SYSTEM_PARTITION_RF_CAL,
			uint32_t(rfCal.address()),
			uint32_t(rfCal.size()),
		},
	};

	enum flash_size_map sizeMap = system_get_flash_size_map();
	if(!system_partition_table_regist(partitions, ARRAY_SIZE(partitions), sizeMap)) {
		os_printf("system_partition_table_regist: failed\n");
		os_printf("size_map = %u\n", sizeMap);
		for(unsigned i = 0; i < ARRAY_SIZE(partitions); ++i) {
			auto& part = partitions[i];
			os_printf("partition[%u]: %u, 0x%08x, 0x%08x\n", i, part.type, part.addr, part.size);
		}
		while(1) {
			// Cannot proceed
		};
	}
}
