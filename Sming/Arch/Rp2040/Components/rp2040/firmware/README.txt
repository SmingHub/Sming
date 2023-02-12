Pico-W Firmware
===============

This directory contains firmware for RP2040 devices.
Files are combined and compressed during build.

Version information can be found by inspecting binary files or building with PICO_DEBUG=1.

43439A0-7.95.49.00.bin
    Nov 29 2021 22:49:09 version 7.95.49 (2271bb6 CY) FWID 01-c51d9400

clm_blob.bin
    API: 12.2
    Data: RaspberryPi.PicoW
    Compiler: 1.29.4
    ClmImport: 1.47.1
    Customization: v5 22/06/24
    Creation: 2022-06-24 06:55:08


Files extracted from pico-sdk/lib/cyw43-driver/firmware/43439A0-7.95.49.00.combined::

    SRCFILE='../pico-sdk/lib/cyw43-driver/firmware/43439A0-7.95.49.00.combined'
    dd if="$SRCFILE" of=43439A0-7.95.49.00.bin bs=1 count=224190
    dd if="$SRCFILE" of=clm_blob.bin bs=1 iseek=224256
