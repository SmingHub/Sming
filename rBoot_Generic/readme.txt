rBoot Generic example

This is an alpha implementation of a Generic rBoot layer for easing the use of rBoot and provide a rBoot Bios application to support ESP fully remote.

This PR includes a "RBootClass" which implements :

    An initial layer around rBoot/rbootHttpUpdate
    An initial way to mount/unmount spiffs filesystems, based on rBoot configuration
    An initial CommandHandler interface to the functionality.
    An example application using the above : rboot_generic

Not much documentation yet,

    see RBootClass.h for usage & interfaces
    connect with telnet client to esp, rboot commands are then available : "rboot info" "rboot load 1 http://10.0.0.201/rom0.bin" "rboot ls" etc type "rboot" for all options.

All command examples have equivalent API's -> available within application

Current V0.1 is only usable/has hardcoded values for use on 4Mb (esp-12) devices, no other supported yet.

The used romconfig is the following :

ROM0 -> 0x002000	-> number from initial rboot figures, application area
ROM1 -> 0x202000	-> number from initial rboot figures, application area
ROM2 -> 0x100000;	-> number set in application code, spiffs area, size 65536
ROM3 -> 0x300000;	-> number set in application code, spiffs area, size 65536

Running the example the functionality can be shown by the following commands :

rboot info -> shows ROM info
rboot load 0 http://10.0.0.200/rom0.bin -> error, don't allow current rom update
rboot load 1 http://10.0.0.200/rom0.bin -> http update rom0.bin to ROM1
rboot load 2 http://10.0.0.200/spiffs_rom.bin -> http update spiffs to ROM2
rboot load 3 http://10.0.0.200/spiffs_rom2.bin -> http update spiffs2 to ROM3

rboot start -> start current ROM (= restart)
rboot start 1 -> start ROM1

rboot mount 2 -> mount ROM2 spiffs
rboot ls -> show files on spiffs
rboot unmount -> unmount spiffs
rboot ls -> error not mounted
rboot mount 3 -> mount ROM3 spiffs
rboot ls -> show files on spiffs


