SPI Flash Emulation
===================

This Component emulates an embedded flash memory device using a backing file. It includes additional
checks on addresses, sizes and alignments to detect common issues which can be more difficult to find
when running on target hardware.

The default backing file is called ``flash.bin``, located in the same directory as the host executable.

See :component-host:`vflash` for configuration details.

