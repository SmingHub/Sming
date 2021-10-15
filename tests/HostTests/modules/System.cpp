#include <HostTests.h>
#include <esp_spi_flash.h>

/*
 * Various system functions must be available for all architectures.
 */
class SystemTest : public TestGroup
{
public:
	SystemTest() : TestGroup(_F("System"))
	{
	}

	void execute() override
	{
		TEST_CASE("Heap")
		{
			REQUIRE_NEQ(system_get_free_heap_size(), 0);
		}

		TEST_CASE("Identification")
		{
			REQUIRE_NEQ(String(system_get_sdk_version()), nullptr);

			auto chip_id = system_get_chip_id();
			REQUIRE_NEQ(chip_id, 0);
			debug_i("chip_id = 0x%08x", chip_id);
		}

		TEST_CASE("Clocks")
		{
			debug_i("CPU running at %u MHz", System.getCpuFrequency());
		}

		TEST_CASE("Watchdog functions available")
		{
			system_soft_wdt_stop();
			system_soft_wdt_restart();
			system_soft_wdt_feed();
		}

#ifndef ARCH_HOST
		TEST_CASE("System restart")
		{
			auto info = system_get_rst_info();
			REQUIRE_NEQ(uint32_t(info), 0);
			debug_i("Reason 0x%08x", info->reason);

			switch(info->reason) {
			case REASON_DEFAULT_RST:
				debug_i("Hanging to check watchdog reboots system...\n\n");
				for(;;) {
				}
				break;

			case REASON_SOFT_WDT_RST:
				debug_i("Reset by watchdog, rebooting...\n\n");
				System.restart(1000);
				pending();
				break;

			case REASON_SOFT_RESTART:
				debug_i("Reset by software. Continuing...\n\n");
				break;

			default:
				// Could be exception, deep sleep, etc.
				debug_i("Restarted for some other reason...\n\n");
			}
		}
#endif
	}
};

void REGISTER_TEST(System)
{
	registerGroup<SystemTest>();
}
