rBoot Sming sample
------------------

This sample integrates rBoot and Sming, for the many people who have been asking
for it. It demonstrates dual rom booting, big flash support, OTA updates and
dual spiffs filesystems. You must enable big flash support in rBoot and use on
an ESP12 (or similar device with 4mb flash). When using rBoot big flash support
with multiple 1mb slots only one rom image needs to be created. If you don't
want to use big flash support (e.g. for a device with smaller flash) see the
separate instructions below. You can easily take the ota files and add them to
your own project to add OTA support.

To build any project (in threory) with rBoot support edit the Makefile to
include Makefile-rboot.mk instead of Makefile-project.mk. Then add variables
to Makefile-user.mk as in this sample.

Building
--------
 1) Set ESP_HOME & SMING_HOME, as environment variables or edit the Makefile, as
    you would for general Sming compiling.
 2) Set ESPTOOL2 (env var or in the Makefile) to point to the esptool2 binary.
 3) Set WIFI_SSID & WIFI_PWD environment variable with your wifi details.
 4) Edit the OTA server details in include/user_config.h
 5) Check overridable variables in Makefile-user.mk, or set as env vars.
 6) make && make flash
 7) Put rom0.bin and spiff_rom.bin in the root of your webserver for OTA.
 8) Interact with the sample using a terminal, sorry no web-gui (yet).

Flashing
--------
If flashing manually use esptool.py to flash rBoot, rom & spiffs e.g.:
 esptool.py --port <port> write_flash -fs 32m 0x00000 rboot.bin 0x02000 rom0.bin
   0x100000 spiffs.rom

Using the correct -fs parameter is important. This will be -fs 32m on an ESP12.

You can also flash rom0.bin to 0x202000, but booting and using OTA is quicker!

Notes
-----
spiffs_mount_manual(address, length) must be called from init. The address must
be 0x40200000 + physical flash address. It does not use memory mapped flash so
the reason for this strange addressing is not clear.

Important compiler flags used:
BOOT_BIG_FLASH - when using big flash mode, ensures flash mapping code is built
  in to the rom.
DISABLE_SPIFFS_AUTO - prevents automounting at the wrong location. (by code in
  Sming/appinit/user_main.cpp). Instead we call spiffs_mount_manual from init.
RBOOT_BUILD_SMING - ensures big flash support function is correcly marked to
  remain in iram (plus potentially other sming specific code in future).
SPIFF_SIZE=value - passed through to code for mounting the filesystem. Also used
  in the Makefile to create the spiffs.

Disabling big flash
-------------------
If you want to use, for example, two 512k roms in the first 1mb block of flash
(old style) then follow these instructions to produce two separately linked
roms. If you are flashing a single rom to multiple 1mb flash blocks (using big
flash) you only need one linked rom that can be used on each.

This assumes you understand the concepts explained in the rBoot readme about
memory mapping and setting linker script address. This is not covered here, just
how to use this sample without bigflash support.

- Do not copy/link rboot-bigflash.c in to your app directory.
- Copy rom0.ld to rom1.ld.
- Adjust the rom offsets and length as appropriate in each ld file.
- Uncomment 'RBOOT_TWO_ROMS ?= 1' in Makefile-user.mk (or set as an environment
  variable).
- After building copy all the rom*.bin files to the root of your web server.

If you want more than two roms you must be an advanced user and should be able
to work out what to copy and edit to acheive this!

Credits
-------
Assistance from piperpilot & gschmott on esp8266.com made this sample possible.
