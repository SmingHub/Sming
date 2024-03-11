Partition table migration
=========================

.. highlight:: c++

This guidance applies to esp8266 devices only.

Sming after v4.2 requires a valid partition table.
If existing devices running previous versions of Sming require updating via OTA then
an intermediate firmware should be created which installs this partition table.

After rBoot hands control to the SDK entrypoint, the ``user_pre_init()`` function is called.
This function is documented in the NON-OS-SDK guide and was introduced in version 3.
It is called by the SDK before user_init() so nothing else in the framework has yet been initialised,
including any C++ static initialisers.

Sming uses this function to read the partition table into memory.
Applications may override this function to perform any custom upgrade operations.

Add to application's `component.mk`:

.. code-block:: make

    EXTRA_LDFLAGS := $(call Wrap,user_pre_init)
    USER_CFLAGS += -DPARTITION_TABLE_OFFSET=$(PARTITION_TABLE_OFFSET)

Add this to your application::

    // Support updating legacy devices without partition tables (Sming 4.2 and earlier)
    #ifdef ARCH_ESP8266

    namespace
    {
    // Note: This file won't exist on initial build!
    IMPORT_FSTR(partitionTableData, PROJECT_DIR "/out/Esp8266/debug/firmware/partitions.bin")
    }

    extern "C" void __wrap_user_pre_init(void)
    {
        static_assert(PARTITION_TABLE_OFFSET == 0x3fa000, "Bad PTO");
        Storage::initialize();
            auto& flash = *Storage::spiFlash;
        if(!flash.partitions()) {
            LOAD_FSTR(data, partitionTableData)
            flash.erase_range(PARTITION_TABLE_OFFSET, flash.getBlockSize());
            flash.write(PARTITION_TABLE_OFFSET, data, partitionTableData.size());
            flash.loadPartitions(PARTITION_TABLE_OFFSET);
        }

        extern void __real_user_pre_init(void);
        __real_user_pre_init();
    }

    #endif // ARCH_ESP8266

.. note::
    
    You will get a 'file not found' error because the partition table gets built *after* compiling the application.
    You can run ``make partmap-build`` manually first to get around this.


An alternative method is to build the partition table layout in code, so there are no external file dependencies::

    // Support updating legacy devices without partition tables (Sming 4.2 and earlier)
    #ifdef ARCH_ESP8266

    #include <Storage/partition_info.h>

    extern "C" void __wrap_user_pre_init(void)
    {
        static_assert(PARTITION_TABLE_OFFSET == 0x3fa000, "Bad PTO");

        Storage::initialize();

        auto& flash = *Storage::spiFlash;
        if(!flash.partitions()) {
            using FullType = Storage::Partition::FullType;
            using SubType = Storage::Partition::SubType;
    #define PT_ENTRY(name, fulltype, offset, size) \
        { ESP_PARTITION_MAGIC, FullType(fulltype).type, FullType(fulltype).subtype, offset, size, name, 0 }

            static constexpr Storage::esp_partition_info_t partitionTableData[] PROGMEM{
                PT_ENTRY("spiFlash", Storage::Device::Type::flash, 0, 0x400000),
                PT_ENTRY("rom0", SubType::App::ota0, 0x2000, 0xf8000),
                PT_ENTRY("rom1", SubType::App::ota1, 0x102000, 0xf8000),
                PT_ENTRY("spiffs0", SubType::Data::spiffs, 0x200000, 0xc0000),
                PT_ENTRY("spiffs1", SubType::Data::spiffs, 0x2c0000, 0xc0000),
                PT_ENTRY("rf_cal", SubType::Data::rfCal, 0x3fb000, 0x1000),
                PT_ENTRY("phy_init", SubType::Data::phy, 0x3fc000, 0x1000),
                PT_ENTRY("sys_param", SubType::Data::sysParam, 0x3fd000, 0x3000),
            };

            uint8_t buffer[sizeof(partitionTableData)];
            memcpy(buffer, partitionTableData, sizeof(partitionTableData));
            flash.erase_range(PARTITION_TABLE_OFFSET, flash.getBlockSize());
            flash.write(PARTITION_TABLE_OFFSET, buffer, sizeof(buffer));
            flash.loadPartitions(PARTITION_TABLE_OFFSET);
        }

        extern void __real_user_pre_init(void);
        __real_user_pre_init();
    }

    #endif // ARCH_ESP8266


The above examples are provided as templates and should be modified as required and tested thoroughly!
