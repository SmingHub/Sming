Picotool
========

Picotool is a tool for inspecting RP2040 binaries, and interacting with RP2040 devices when they are in BOOTSEL mode.

See https://rptl.io/pico-get-started Appendix B for an introduction to this tool.

Sming builds picotool from source and uses it to read back flash memory with build targets such as ``make readpart``.

The tool can be invoked directly like this::

    make picotool CMD="info -a"
