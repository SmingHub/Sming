Storage Manager
===============

This Component provides support for using storage devices in a structured way by partitioning
them into areas for specific uses.

Partitions may can contain information such as:

-  Application (firmware) images
-  Filesystem(s)
-  Configuration/calibration/parameter data
-  Custom flash storage areas

A single partition table is located on the main flash device, :cpp:var:`Storage::spiFlash`,
and defines all partitions with a unique name and associated :cpp:type:`Storage::Partition::Type` / :cpp:type:`Storage::Partition::SubType`.



.. _hardware_config:

Hardware configuration
----------------------

Each project has an associated ``Hardware configuration``, specified by the :envvar:`HWCONFIG` setting.

This information is defined in one or more JSON files, identified with a ``.hw`` extension.
You can find a schema for this file in :doc:`schema.json`.

The build system locates the file by searching, in order:

-  ``{PROJECT_DIR}`` the root project directory
-  ``{SMING_HOME}/Arch/{SMING_ARCH}``
-  ``{SMING_HOME}``

Files (other than standard) will have a ``base_config`` setting. The specified profile is pulled
in following the same search pattern as above.

This provides a mechanism for inheritance to allow adjustment of existing profiles
without having to copy it entirely.

For example, to adjust the address/size of the main application partition
(for the esp8266, this is ``rom0``), you can do this:

.. code-block:: json

   {
      "name": "My config",
      "base_config": "standard-4m",
      "partitions": {
         "rom0": {
            "address": "0x10000",
            "size": "0x80000"
         }
      }
   }


You can examine your project's active configuration like this::

   make hwconfig

This also shows a list of the discovered configurations available.

You can also display the memory map::

   make map

Which, for Host arch, produces:

.. code-block:: text

   HostTests: Invoking 'map' for Host (debug) architecture
   Device            Start     End       Size      Type      SubType   Name              Filename
   ----------------  --------  --------  --------  --------  --------  ----------------  ------------
   spiFlash          0x000000  0x001fff        8K                      Boot Sector
   spiFlash          0x002000  0x002fff        4K                      Partition Table
   spiFlash          0x003000  0x007fff       20K                      (unused)
   spiFlash          0x008000  0x0fffff      992K  app       factory   rom0              $(BLANK_BIN)
   spiFlash          0x100000  0x1fffff        1M                      (unused)
   spiFlash          0x200000  0x27ffff      512K  data      spiffs    spiffs0           $(FW_BASE)/spiffs0.bin
   spiFlash          0x280000  0x3fffff     1536K                      (unused)
   test-device       0x000000  0x3fffff        4M  data      spiffs    external1
   test-device       0x400000  0x5fffff        2M                      (unused)
   test-device       0x600000  0x63bfff      240K  data      37        external2
   test-device       0x63c000  0xffffff    10000K                      (unused)

For comparison, here's the output for Esp32:

.. code-block:: text

   HostTests: Invoking 'map' for Esp32 (debug) architecture
   Device            Start     End       Size      Type      SubType   Name              Filename
   ----------------  --------  --------  --------  --------  --------  ----------------  ------------
   spiFlash          0x000000  0x007fff       32K                      Boot Sector
   spiFlash          0x008000  0x008fff        4K                      Partition Table
   spiFlash          0x009000  0x00efff       24K  data      nvs       nvs
   spiFlash          0x00f000  0x00ffff        4K  data      phy       phy_init
   spiFlash          0x010000  0x1fffff     1984K  app       factory   factory           $(TARGET_BIN)
   spiFlash          0x200000  0x27ffff      512K  data      spiffs    spiffs0           $(FW_BASE)/spiffs0.bin
   spiFlash          0x280000  0x3fffff     1536K                      (unused)
   test-device       0x000000  0x3fffff        4M  data      spiffs    external1
   test-device       0x400000  0x5fffff        2M                      (unused)
   test-device       0x600000  0x63bfff      240K  data      37        external2
   test-device       0x63c000  0xffffff    10000K                      (unused)


Configuration
-------------

.. envvar:: HWCONFIG

   default: standard

   Set this to the hardware configuration file to use for your project (without .hw extension)

   Default configurations:

   standard
      Base profile which should work on all device variants.
      Located in the ``Sming/Arch/{SMING_ARCH}`` directory.
   standard-4m
      Supports only devices with 4Mbyte flash
   spiffs
      Based on standard-4m and adds a single SPIFFS partition

   Other configurations may be available, depending on architecture.

   You can see these by running ``make hwconfig``.


Binary partition table
----------------------

Sming uses the same binary partition table structure as ESP-IDF, located immediately after the boot sector.
However, it is organised slighly differently to allow partitions to be registered for multiple storage devices.

Entries are fixed 32-byte structures, :cpp:class:`Storage::esp_partition_info_t`, organised as follows:

-  The first entry is always a ``storage`` type defining the main :cpp:var:`spiFlash` device.
-  This is followed by regular partition entries sorted in ascending address order.
   There may be gaps between the partitions.
-  The partition table md5sum entry is inserted as normal
-  If any external devices are defined:
   -  A SMING_EXTENSION entry, which the esp32 bootloader interprets as the end of the partition table.
   -  The next entry is a ``storage`` type for the 'external` device.
   -  This is followed by regular partition entries as before.
   -  A second md5sum entry is inserted for the entire partition table thus far
-  The end of the partition table is identified by an empty sector (i.e. all bytes 0xFF).

On initialisation, partition entries with the correct MAGIC entry are loaded and all others are ignored.

A storage partition entry `address` field is always initialised to 0.
When :cpp:func:`PartitionTable::registerStorageDevice()` is called, this field is updated with
a pointer to the :cpp:class:`Storage::Device` implementation.

Located :cpp:class:`Partition` objects contain a reference to both the requested partition
entry and the storage partition entry.


Partition API
-------------

This is a C++ interface. Some examples::

   auto part = partitionTable.find("spiffs0"); // Find by name

   // Enumerate all partitions
   for(auto part: partitionTable) {
      debugf("Found '%s' at 0x%08x, size 0x%08x", part.name().c_str(), part.address(), part.size());
   }

   // Enumerate all SPIFFS partitions
   for(auto it = partitionTable.find(Partition::SubType::Data::spiffs; it; it++) {
      auto part = *it;
      debugf("Found '%s' at 0x%08x, size 0x%08x", part.name().c_str(), part.address(), part.size());
   }


A :cpp:class:`Storage::Partition` object is just a wrapper and can be freely copied around.
It defines methods which should be used to read/write/erase the partition contents.

Each partition has an associated :cpp:class:`Storage::Device`.
This is usually :cpp:var:`Storage::spiFlash` for the main flash device.

Other devices must be registed via :cpp:func:`Storage::PartitionTable::registerStorageDevice`.


.. doxygennamespace:: Storage
   :members:
