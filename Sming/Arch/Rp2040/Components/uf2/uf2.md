# USB Flashing Format (UF2)

UF2 is a file format, developed by Microsoft for [PXT](https://github.com/Microsoft/pxt) 
(also known as [Microsoft MakeCode](https://makecode.com)), that is particularly suitable for 
flashing microcontrollers over MSC (Mass Storage Class; aka removable flash drive).

For a more friendly explanation, check out [this blog post](https://makecode.com/blog/one-chip-to-flash-them-all).
Also, take a look at the list of [implementations](#implementations) at the bottom of this document.

## Overview

The UF2 file consists of 512 byte blocks, each of which is self-contained and independent of others. 
Each 512 byte block consists of (see below for details):
  * magic numbers at the beginning and at the end
  * address where the data should be flashed
  * up to 476 bytes of data

The data transfers over MSC always arrive in multiples of 512 bytes.
Together with the FAT file system structure, this means that blocks of the
UF2 file are always aligned with the MSC writes - the microcontroller 
never gets a partial file.

The magic numbers let the microcontroller distinguish an UF2 file block from
other data (eg., FAT table entry, or various book-keeping files stored by some
operating systems).  When a UF2 block is recognized, it can be flashed
immediately (unless flash page size is more than 256 bytes; in that case a buffer
is needed). The actual handling of file format during writing is very simple
(~10 lines of C code in simplest version).

## File format

A UF2 file consists of 512 byte blocks. Each block starts with a 32 byte
header, followed by data, and a final magic number.
All fields, except for data, are 32 bit unsigned little endian integers.

| Offset | Size | Value                                             |
|--------|------|---------------------------------------------------|
| 0      | 4    | First magic number, `0x0A324655` (`"UF2\n"`)      |
| 4      | 4    | Second magic number, `0x9E5D5157`                 |
| 8      | 4    | Flags                                             |
| 12     | 4    | Address in flash where the data should be written |
| 16     | 4    | Number of bytes used in data (often 256)          |
| 20     | 4    | Sequential block number; starts at 0              |
| 24     | 4    | Total number of blocks in file                    |
| 28     | 4    | File size or board family ID or zero              |
| 32     | 476  | Data, padded with zeros                           |
| 508    | 4    | Final magic number, `0x0AB16F30`                  |

The following C struct can be used:

```C
struct UF2_Block {
    // 32 byte header
    uint32_t magicStart0;
    uint32_t magicStart1;
    uint32_t flags;
    uint32_t targetAddr;
    uint32_t payloadSize;
    uint32_t blockNo;
    uint32_t numBlocks;
    uint32_t fileSize; // or familyID;
    uint8_t data[476];
    uint32_t magicEnd;
} UF2_Block;
```

### Flags

Currently, there are five flags defined:

  * `0x00000001` - **not main flash** - this block should be skipped when writing the
    device flash; it can be used to store "comments" in the file, typically
    embedded source code or debug info that does not fit on the device flash

  * `0x00001000` - **file container** - see below

  * `0x00002000` - **familyID present** - when set, the `fileSize/familyID` holds a value
    identifying the board family (usually corresponds to an MCU)

  * `0x00004000` - **MD5 checksum present** - see below

  * `0x00008000` - **extension tags present** - see below

### Family ID

This field is optional, and should be set only when the corresponding
flag is set. It is recommended that new bootloaders require the field to
be set appropriately, and refuse to flash UF2 files without it.
If you're developing your own bootloader, and your
board family isn't listed here, pick a new family ID at random. It's good
to also send a PR here, so your family can be listed.

If the `familyID` doesn't match, the bootloader should disregard the
entire block, including `blockNo` and `numBlocks` fields.
In particular, writing a full UF2 file with non-matching `familyID`
should not reset the board.
This also allows for several files with different `familyID` to be
simply concatenated together, and the whole resulting file to be copied
to the device with only one actually being written to flash.

#### Picking numbers at random

The reason to pick numbers at random is to minimize risk of collisions
in the wild. Do not pick random numbers by banging on keyboard, or by using
`0xdeadf00d`, `0x42424242` etc. A good way is to use the following
shell command: `printf "0x%04x%04x\n" $RANDOM $RANDOM`
Another good way is the link at the bottom of https://microsoft.github.io/uf2/patcher/
This procedure was unfortunately not used for the SAMD51 and NRF52840 below.

#### Family list

The current master list of family IDs is maintained in a [JSON file](utils/uf2families.json).

### Rationale

The magic number at the end is meant to mitigate partial block writes.

Second and final magic numbers were randomly selected, except for the last byte
of final magic number, which was forced to be `'\n'` (`0xA`). Together with the
first magic number being `"UF2\n"` this makes it easy to identify UF2 blocks in
a text editor.

The header is padded to 32 bytes, as hex editors commonly use 16 or 32 bytes
as line length.  This way, the data payload is aligned to line start.

32 bit integers are used for all fields so that large flash sizes can be
supported in future, as well as for simplicity. Little endian is used, as most
microcontrollers are little endian. 8 bit microcontrollers can choose to just
use the first 16 bits of various header fields.

The total number of blocks in the file and the sequential block number make it
easy for the bootloader to detect that all blocks have been transferred. It
requires one bit of memory per block (eg., on SAMD21G18A it's 128 bytes).
Alternatively, the bootloader might ignore that and just implement a reset
after say 1 second break in incoming UF2 blocks.

### Payload sizes

The number of data bytes is configurable and depends on the size of
the flash page (that is the smallest size that can be erased) on the
microcontroller.

  * if the page size is more than `476` bytes, the bootloader should support
    any payload size, as it needs to buffer the entire page in memory anyway

  * if the page size is less than `476` bytes, the payload should be a multiple
    of page size, so it can be written without buffering; the target address
    should also be a multiple of page size

In any event, payload size and target address should always be 4-byte aligned.

Note that payload size of `256` is always correct, and makes it easy to convert
between flash addresses and UF2 file offsets.

For example, on Atmel's SAMD21 chips the page size is `256` bytes, and this 
also is the payload size. If the page size was `128` bytes, one could use
payload of `128*3`. Nordic nRF51 has page size of `1024` bytes, and thus 
any payload size should be allowed.

### Embedding sources

Some IDEs will embed program sources in the UF2 file. This allows a UF2 files to be
loaded by the IDE and serve as a natural backup and transfer format.
This can be done in two ways:

  * using the "not main flash" flag
  * using normal blocks that are flashed to the device

If the bootloader can expose `CURRENT.UF2` file (see below) and there is enough
flash available, than the second option is more desirable, as it allows sharing
programs directly from the board.

See https://makecode.com/source-embedding for more info.

### Robustness

The file format is designed specifically to deal with the following problems:

  * operating system (OS) writing blocks in different order than occurs in a file
  * OS writing blocks multiple times
  * OS writing data that is not UF2 blocks
  * OS writing first/final part of a block, possibly for metadata detection or search indexing

The only file system assumption we make is that blocks of file are aligned with
blocks on the hard drive. It's likely true of many file systems besides FAT.

We also assume that USB MSC device reports its block size to be a multiple of `512`
bytes. In the wild these devices always almost report exactly `512`, and some
operating systems do not support other values.

## Files exposed by bootloaders

Bootloaders may expose virtual files in their MSC devices.  These are
standardized here, so that flashing tools can automatically detect the
bootloaders.

  * `INFO_UF2.TXT` - contains information about the bootloader build and the board on which it is running

  * `INDEX.HTM` - redirects to a page that contains an IDE or other information

  * `CURRENT.UF2` - the contents of the entire flash of the device, starting at `0x00000000`, with `256` payload size;
    thus, the size of this file will report as twice the size of flash

Flashing tools can use the presence of `INFO_UF2.TXT` (in upper or lower case,
as FAT is case-insensitive) file as an indication that a given directory is
actually a connected UF2 board. The other files should not be used for
detection.

Typical `INFO_UF2.TXT` file looks like this:

```
UF2 Bootloader v1.1.3 SFA
Model: Arduino Zero
Board-ID: SAMD21G18A-Zero-v0
```

The `Board-ID` field is machine-readable and consists of a number of dash-separated tokens.
The first token is the CPU type, second is the board type, and third is the board revision.
More tokens can be also added.

The bootloader should contain its info file as a static string somewhere in its code.
If possible, the last word of the bootloader code should point to this string.
This way, the info file can be found in the initial section of the `CURRENT.UF2`
file as well. Thus, a board type can be determined from the contents of `CURRENT.UF2`.
This is particularly useful with the source embedding (see above).

## File containers

It is also possible to use the UF2 format as a container for one or more
regular files (akin to a TAR file, or ZIP archive without compression).  This
is useful when the embedded device being flashed sports a file system.

The program to run may reside in one of the files, or in the main flash memory.

In such a usage the `file container` flag is set on blocks, the field `fileSize`
holds the file size of the current file, and the field `targetAddr` holds the
offset in current file.

The `not main flash` flag on blocks should be ignored when the `file container` is set.

The file name is stored at `&data[payloadSize]` (ie., right after the actual payload) and
terminated with a `0x00` byte.  The format of filename is dependent on the
bootloader (usually implemented as some sort of file system daemon). 

The bootloader will usually allow any size of the payload.

The current files on device might be exposed as multiple UF2 files, instead of
a single `CURRENT.UF2`. They may reside in directories, however, due to UF2 general
design, it doesn't matter which directory the UF2 file is written to.

Typical writing procedure is as follows:

  * validate UF2 magic numbers
  * make sure that `targetAddr < fileSize` and that `fileSize` isn't out of reasonable range
  * write `0x00` at `data[475]` to ensure NUL termination of file name
  * read file name from `&data[payloadSize]`; perform any mapping on the file name
  * create a directory where the file is to be written if it doesn't exist
  * open the file for writing
  * truncate the file to `fileSize`
  * seek `targetAddr`
  * write the payload (ie., `data[0 ... payloadSize - 1]`)
  * close the file

The fields `blockNo` and `numBlocks` refer to the entire UF2 file, not the current
file.

## MD5 checksum

When the `0x4000` flag is set, the last 24 bytes of `data[]` hold the following structure:

| Offset | Size | Value                                             |
|--------|------|---------------------------------------------------|
| 0      | 4    | Start address of region                           |
| 4      | 4    | Length of region in bytes                         |
| 8      | 16   | MD5 checksum in binary format                     |

The flashing program should compute the MD5 sum of the specified region.
If the region checksum matches, flashing of the current block can be skipped.
Typically, many blocks in sequence will have the same region specified,
and can all be skipped, if the matching succeeded.
The position of the current block will typically be inside of the region.
The position and size of the region should be multiple of page erase size
(4k or 64k on typical SPI flash).

This is currently only used on ESP32, which is also why MD5 checksum is used.

## Extension tags

When the `0x8000` flag is set, additional information can be appended right after
payload data (i.e., it starts at `32 + payloadSize`).
Every tag starts at 4 byte boundary.
The first byte of tag contains its total size in bytes (including the size byte
and type designation).
The next three bytes designate the type of tag (if you want to define custom
tags, pick them at random).
The last tag has size of `0` and type of `0`.

Standard tag designations follow:

  * `0x9fc7bc` - version of firmware file - UTF8 semver string

  * `0x650d9d` - description of device for which the firmware file is destined (UTF8)

  * `0x0be9f7` - page size of target device (32 bit unsigned number)

  * `0xb46db0` - SHA-2 checksum of firmware (can be of various size)

  * `0xc8a729` - device type identifier - a refinement of `familyID` meant to identify a kind of device
    (eg., a toaster with specific pinout and heating unit), not only MCU; 32 or 64 bit number; can be hash of `0x650d9d`

For example, the following bytes encode firmware version `0.1.2` for device
named `ACME Toaster mk3` (line breaks added for clarity):

```
09 bc c7 9f 30 2e 31 2e 32 00 00 00
14 9d 0d 65 41 43 4d 45 20 54 6f 61 73 74 65 72 20 6d 6b 33
00 00 00 00
```

Extension tags can, but don't have to, be repeated in all blocks.

## Implementations

### Bootloaders

  * [Microchip ATSAMD21 and ATSAMD51](https://github.com/Microsoft/uf2-samdx1)
  * [Arduino UNO](https://github.com/mmoskal/uf2-uno)
  * [STM32F103](https://github.com/mmoskal/uf2-stm32)
  * [STM32F4](https://github.com/mmoskal/uf2-stm32f)
  * [Nordic NRF52840](https://github.com/adafruit/Adafruit_nRF52840_Bootloader)
  * [Linux (RPi Zero)](https://github.com/microsoft/uf2-linux)
  * [Cypress FX2](https://github.com/whitequark/libfx2/tree/master/firmware/boot-uf2)
  * [Tiny UF2](https://github.com/adafruit/tinyuf2) - Support ESP32-S2, iMXRT10xx, STM32F4
  * [RP2040 chip](https://www.raspberrypi.org/products/raspberry-pi-pico/) - native support in silicon
  * [UF2-ChibiOS](https://github.com/striso/uf2-ChibiOS) - Supports STM32H7

There's an ongoing effort to implement UF2 in [Codal](https://github.com/lancaster-university/codal-core).

### Editors

* https://arcade.makecode.com
* https://makecode.adafruit.com
* https://makecode.seeedstudio.com
* https://maker.makecode.com

### Libraries

* https://www.npmjs.com/package/uf2

## License

MIT

## Code of Conduct

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/). For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.
