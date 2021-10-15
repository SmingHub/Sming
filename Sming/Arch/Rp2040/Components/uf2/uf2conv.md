# uf2conv -- Packing and unpacking UF2 files

## SYNOPSIS

This tool is based on the code at https://github.com/Microsoft/uf2
but has been modified for use with Sming and the RP2040:

    - Handles multiple binary inputs using addr=content format
    - Locations specified as offsets, relative to base address
    - Defaults are set for RP2040, but can be changed (family ID and base address)
    - Content is padded to flash sector size (4096 bytes) - required by RP2040 bootrom
    - Content is padded with 0xff (not 0) as this represents erased (unprogrammed) flash location
    - Renamed 'deploy' as 'upload' and added progress indicator
    - Support for hex files and 'C' arrays removed

Run `uf2conv.py -h` for full list of options.

## EXAMPLES

### Pack binary file(s) to .uf2

Specify source using `offset=content` values:

```uf2conv.py --convert 0x2000=cpx/firmware.bin 0x4000=cpx/partitions.bin --output cpx/upload.uf2```

Offsets are relative to the start of flash memory.

### Unpack a .uf2 file

```uf2conv.py current.uf2 --output current.bin```

Produces a list of `current-{addr}.bin` files for each chunk found in the .uf2 source file.
Without the `--output` option just displays a chunk summary.

Use the `--verbose` option to display details for every block in the file.
