Storage Partition notes
=======================

External devices
----------------

Support for 'external' storage devices is implemented by marking groups of entries
with a ``storage`` type partition entry. This is intended to support designs where the
memory devices are fixed (such as additional SPI RAM or flash chips) rather than
removable (such as SD cards). When these devices are registered with the Storage subsystem,
the primary partition table is scanned for related entries.

Devices may contain their own partition table, but such tables are not loaded automatically.
This is done by calling :cpp:func:`Storage::Device::loadPartitions`.
The application must also consider how the partition table will be initialised on external
devices as this is not handled by the build system.

 

But it also has some disadvantages:

-  Adding/removing storage devices dynamically does not fit well.
   For example, when an SD Card is inserted the partition layout must be established
   dynamically.

The main purpose of the partition table is to manage the system's fixed storage.
Mostly that will just be the main flash device, but could also be 'external' devices
such as SPI RAM or flash.

Devices can be dynamically registered/un-registered to the partition table: existing Partition objects remain valid but calls will fail.

Dynamic partitions have additional issues.

Partition identification
------------------------

Two devices may have identical partition names so these would need
to be qualified, e.g. ``spiFlash/spiffs0``. Without qualification the first matching
name would be used.

Alternatively, each :cpp:class:`Storage::Device` object could manage its own partitions,
rather than having them in a single table. The existing layout scheme is fine, we can
just extend it so that devices can support their own partitions.

Device serial numbers should be exposed so that devices can be uniquely identified
by applications.

Device partitions
-----------------

A global :cpp:class:`Storage::DeviceManager` object replaces ``partitionTable``.

Storage each device manages its own partitions, therefore spiFlash loads its own partition table.



Partitions in devices ?
-----------------------

JSON may be more logically arranged so that partitions are defined within each device.

This implies that partition_table_offset is also defined within the device,
so adding this entry to external devices allows definition of a partition table there also.
The application now only has to handle how to write any supplementary tables to external devices.

Problem: Way too complicated.

So, if we want external partition tables how to do that?
Leave for another time.