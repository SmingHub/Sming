Bulk Storage and Filing Systems
===============================

Sming uses a class-based layered approach to bulk storage.

The :component:`Storage` Component defines the :cpp:class:`Storage::Device` abstract class,
which devices such as SPI flash implement to provide raw read/write/erase access.
Devices are partitioned into areas for specific uses which applications
access using a :cpp:class:`Storage::Partition` object.

The :library:`DiskStorage` library provides support for block-access devices
which use standard partitioning schemes (MBR, GPT).
SD cards are supported via the :library:`SdStorage` library.

Sming uses an installable (virtual) filing system mechanism based on :cpp:class:`IFS::IFileSystem`.
This is managed by the :component:`IFS` Component and contains the FWFS lightweight read-only filing system.

Additional filing system implementations are provided in separate libraries:

- :library:`Spiffs`
- :library:`LittleFS`
- :library:`FatIFS`

Note that when using bulk storage of more than about 4GB in size applications should be built with
:cpp:envvar:`ENABLE_STORAGE_SIZE64` =1. This changes all sizes and offsets to 64-bit.

If manipulating files greater than about 2GB (signed 32-bit value) then the :cpp:envvar:`ENABLE_FILE_SIZE64`
setting should also be enabled.
