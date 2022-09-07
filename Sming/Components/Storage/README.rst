Storage Management
==================

.. highlight:: bash

This Component provides support for using storage devices in a structured way by partitioning
them into areas for specific uses.
Partitions may can contain information such as:

-  Application (firmware) images
-  Filesystem(s)
-  Configuration/calibration/parameter data
-  Custom flash storage areas

A single partition table is located on the main flash device, :cpp:var:`Storage::spiFlash`,
and defines all partitions with a unique name and associated
:cpp:enum:`Storage::Partition::Type` / :cpp:type:`Storage::Partition::SubType`.



.. _hardware_config:

Hardware configuration
----------------------

Each project has an associated ``Hardware configuration``, specified by the :envvar:`HWCONFIG` setting:
this is a JSON file with a ``.hw`` extension.

For user convenience, the configuration file may contain comments however these are stripped before
processing.

The build system locates the file by searching, in order:

-  ``{PROJECT_DIR}`` the root project directory
-  ``{SMING_HOME}/Arch/{SMING_ARCH}``
-  ``{SMING_HOME}``

Each architecture provides a ``standard`` configuration which defines such things as the
partition table location and standard system partitions. Other configurations inherit
from this by providing a ``base_config`` value.

You can list the available configs like this::

   make hwconfig-list

This also shows the file path should you wish to view or edit it.

To select and view the resulting configuration, do this::

   make hwconfig HWCONFIG=spiffs

or, to show the partition map::

   make map HWCONFIG=spiffs

.. note::

   You can set :envvar:`HWCONFIG` in your project's ``component.mk`` file, however as with other
   configuration variables it will be overridden by the cached value set on the command line.

   For example, if you want to change from ``standard`` to ``standard-4m`` for your project,
   first add this line to your component.mk file::

      HWCONFIG := standard-4m

   Then either run ``make HWCONFIG=standard-4m`` or ``make config-clean``.

.. _hwconfig_options:

Hardware configuration options
------------------------------

Commonly used settings can be stored in an option library for easier use.
The library files are named ``options.json`` and located in the place as .hw files.

For example, we can do this::

   make HWCONFIG=standard HWCONFIG_OPTS=4m,spiffs

This loads the 'standard' profile then merges the fragments found in the option library with the given names.
This is how the ``standard-4m`` profile is constructed.

If using this approach, remember to updated your project's ``component.mk`` with the desired settings,
and verify the layout is correct using ``make map``.


OTA updates
-----------

When planning OTA updates please check that the displayed partition map corresponds to your project.
For example, the partition table requires a free sector so must not overlap other partitions.

Your OTA update process must include a step to write the partition table to the correct location.

It is not necessary to update the bootloader. See :component:`rboot` for further information.


Custom configurations
---------------------

To customise the hardware configuration for a project, for example 'my_project':

1. Create a new configuration file in your project root, such as ``my_project.hw``:

   .. code-block:: json

      {
         "name": "My project config",
         "base_config": "spiffs",
         "options": ["vdd"]
      }

   You can use any available configuration as the base_config.
   Option fragments can be pulled in as shown. See :ref:`hwconfig_options`.


2. If required, modify any inherited settings:

   .. code-block:: json

      {
         "name": "My config",
         "base_config": "standard",
         "devices": {
            "spiFlash": {
               "speed": 80,
               "mode": "qio",
               "size": "2M"
            }
         },
         "partitions": {
            "rom0": {
               "address": "0x10000",
               "size": "0x80000"
            }
         }
      }

   This will adjust flash parameters (previously via SPI_SPEED, SPI_MODE and SPI_SIZE),
   and the location/size of the primary application partition.

3. Add any additional partitions:

   .. code-block:: json

      {
         "name": "My config",
         "base_config": "standard-4m",
         "partitions": {
            "rom0": {
               "address": "0x10000",
               "size": "0x80000"
            },
            "spiffs1": {
                  "address": "0x00280000",
                  "size": "256K",
                  "type": "data",
                  "subtype": "spiffs",
                  "filename": "$(FW_BASE)/spiffs1_rom.bin",
                  "build": {
                     "target": "spiffsgen",
                     "files": "files/spiffs1"
                  }
            }
         }
      }

   This adds a second SPIFFS partition, and instructs the build system to generate
   an image file for it using the files in the project's ``files/spiffs1`` directory.

4. Select the new configuration and re-build the project::

      make HWCONFIG=my_project

   You should also add this to your project's ``component.mk`` file::

      HWCONFIG := my_project

5. Program your device::

      make flash

   This will flash everything: bootloader, partition table and all defined partitions (those with a ``filename`` entry).


.. note::

   The build system isn't smart enough to track dependencies for partition build targets.

   To rebuild these manually type::

      make buildpart

   These will be removed when ``make clean`` is run, but you can also clean them separately thus::

      make part-clean


Partition maps
--------------

This is a concise view of your flash partitions. Display it like this::

   make map

For the :sample:`Basic_Storage` sample application, we get this:

.. code-block:: text

   Basic_Storage: Invoking 'map' for Esp8266 (debug) architecture
   Partition map:
   Device            Start       End         Size        Type      SubType   Name              Filename
   ----------------  ----------  ----------  ----------  --------  --------  ----------------  ------------
   spiFlash          0x00000000  0x00001fff          8K                      Boot Sector
   spiFlash          0x00002000  0x00002fff          4K                      Partition Table
   spiFlash          0x00003000  0x00003fff          4K  data      phy       phy_init          $(FLASH_INIT_DATA)
   spiFlash          0x00004000  0x00007fff         16K  data      sysparam  sys_param
   spiFlash          0x00008000  0x000fffff        992K  app       factory   rom0              $(RBOOT_ROM_0_BIN)
   spiFlash          0x00100000  0x001effff        960K                      (unused)
   spiFlash          0x001f0000  0x001f3fff         16K  user      0         user0             user0.bin
   spiFlash          0x001f4000  0x001f7fff         16K  user      1         user1
   spiFlash          0x001f8000  0x001fffff         32K                      (unused)
   spiFlash          0x00200000  0x0027ffff        512K  data      spiffs    spiffs0           $(SPIFF_BIN_OUT)
   spiFlash          0x00280000  0x002bffff        256K  data      spiffs    spiffs1           $(FW_BASE)/spiffs1_rom.bin
   spiFlash          0x002c0000  0x002fffff        256K  data      spiffs    spiffs2           $(FW_BASE)/spiffs2_rom.bin
   spiFlash          0x00300000  0x003fffff          1M                      (unused)

For comparison, here's the output for Esp32:

.. code-block:: text

   Basic_Storage: Invoking 'map' for Esp32 (debug) architecture
   Partition map:
   Device            Start       End         Size        Type      SubType   Name              Filename
   ----------------  ----------  ----------  ----------  --------  --------  ----------------  ------------
   spiFlash          0x00000000  0x00007fff         32K                      Boot Sector
   spiFlash          0x00008000  0x00008fff          4K                      Partition Table
   spiFlash          0x00009000  0x0000efff         24K  data      nvs       nvs
   spiFlash          0x0000f000  0x0000ffff          4K  data      phy       phy_init
   spiFlash          0x00010000  0x001fffff       1984K  app       factory   factory           $(TARGET_BIN)
   spiFlash          0x001f0000  0x001f3fff         16K  user      0         user0             user0.bin
   spiFlash          0x001f4000  0x001f7fff         16K  user      1         user1
   spiFlash          0x001f8000  0x001fffff         32K                      (unused)
   spiFlash          0x00200000  0x0027ffff        512K  data      spiffs    spiffs0           $(SPIFF_BIN_OUT)
   spiFlash          0x00280000  0x002bffff        256K  data      spiffs    spiffs1           $(FW_BASE)/spiffs1_rom.bin
   spiFlash          0x002c0000  0x002fffff        256K  data      spiffs    spiffs2           $(FW_BASE)/spiffs2_rom.bin
   spiFlash          0x00300000  0x003fffff          1M                      (unused)


To compare this with the partition map programmed into a device, do this::

   make readmap map


JSON validation
---------------

When the binary partition table is built or updated, the configuration is first
validated against a schema :source:`Sming/Components/Storage/schema.json`.

This complements the checks performed by the ``hwconfig`` tool.

You can run the validation manually like this::

   make hwconfig-validate

See `JSON Schema <https://json-schema.org/>`__ for details about JSON schemas.


Configuration
-------------

.. envvar:: HWCONFIG

   default: standard

   Set this to the hardware configuration to use for your project.

   Default configurations:

   standard
      Base profile with 1MB flash size which should work on all device variants.
      Located in the ``Sming/Arch/{SMING_ARCH}`` directory.

      standard-4m
         Overrides ``standard`` to set 4Mbyte flash size
   
         spiffs
            Adds a single SPIFFS partition. See :library:`Spiffs`.

   Other configurations may be available, depending on architecture.
   You can see these by running ``make hwconfig-list``.

   For example, to select ``spiffs`` add the following line to your project::

        HWCONFIG := spiffs

   You will also need to run ``make HWCONFIG=spiffs`` to change the cached value
   (or ``make config-clean`` to reset everything).


.. envvar:: HWCONFIG_OPTS

   Set this to adjust the hardware profile using option fragments. See :ref:`hwconfig_options`.


Binary partition table
----------------------

Sming uses the same binary partition table structure as ESP-IDF, located immediately after the boot sector.
However, it is organised slightly differently to allow partitions to be registered for multiple storage devices.

Entries are fixed 32-byte structures, :cpp:class:`Storage::esp_partition_info_t`, organised as follows:

-  The first entry is always a ``storage`` type defining the main :cpp:var:`spiFlash` device.
-  This is followed by regular partition entries sorted in ascending address order.
   There may be gaps between the partitions.
-  The partition table md5sum entry is inserted as normal
-  If any external devices are defined:
   -  A SMING_EXTENSION entry, which the esp32 bootloader interprets as the end of the partition table.
   -  The next entry is a ``storage`` type for the ``external`` device.
   -  This is followed by regular partition entries as before.
   -  A second md5sum entry is inserted for the entire partition table thus far
-  The end of the partition table is identified by an empty sector (i.e. all bytes 0xFF).



Partition API
-------------

This is a C++ interface. Some examples::

   Storage::Partition part = Storage::findPartition("spiffs0"); // Find by name
   if(part) {
     debugf("Partition '%s' found", part.name().c_str());
   } else {
     debugf("Partition NOT found");
   }

   // Enumerate all partitions
   for(auto part: Storage::findPartition()) {
      debugf("Found '%s' at 0x%08x, size 0x%08x", part.name().c_str(), part.address(), part.size());
   }

   // Enumerate all SPIFFS partitions
   for(auto part: Storage::findPartition(Storage::Partition::SubType::Data::spiffs)) {
      debugf("Found '%s' at 0x%08x, size 0x%08x", part.name().c_str(), part.address(), part.size());
   }


A :cpp:class:`Storage::Partition` object is just a wrapper and can be freely copied around.
It defines methods which should be used to read/write/erase the partition contents.

Each partition has an associated :cpp:class:`Storage::Device`.
This is usually :cpp:var:`Storage::spiFlash` for the main flash device.

Other devices must be registered via :cpp:func:`Storage::PartitionTable::registerStorageDevice`.

You can query partition entries from a Storage object directly, for example::

   #include <Storage/SpiFlash.h>

   for(auto part: Storage::spiFlash->partitions()) {
      debugf("Found '%s' at 0x%08x, size 0x%08x", part.name().c_str(), part.address(), part.size());
   }


External Storage
----------------

If your design has additional fixed storage devices, such as SPI RAM, flash or EEPROM,
you can take advantage of the partition API to manage them as follows:

-  Implement a class to manage the storage, inheriting from :cpp:class:`Storage::Device`.
-  Create a custom hardware configuration for your project and add a ``devices`` entry
   describing your storage device, plus partition entries: the ``device`` field identifies
   which device these entries relate to.
-  Create an instance of your custom device and make a call to :cpp:func:`Storage::registerDevice`
   in your ``init()`` function (or elsewhere if more appropriate).


API
---

.. doxygennamespace:: Storage
   :members:
