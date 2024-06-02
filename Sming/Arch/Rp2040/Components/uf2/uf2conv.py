#!/usr/bin/env python3
#
# uf2conv.py
#
# Tool for packing and unpacking UF2 files
#

import sys, struct, os, re, subprocess, argparse, json, time

# Sizes
UF2_BLOCK_SIZE       = 512
UF2_PAYLOAD_SIZE     = 256
UF2_MAX_PAYLOAD_SIZE = 476
UF2_HEADER_SIZE      = 32
UF2_FOOTER_SIZE      = 4
# Header
UF2_MAGIC_START0 = 0x0A324655 # "UF2\n"
UF2_MAGIC_START1 = 0x9E5D5157 # Randomly selected
# Footer
UF2_MAGIC_END    = 0x0AB16F30 # Ditto
# Flag bits
UF2_FLAG_NOFLASH        = 0x00000001 # Block is "comment", do not flash
UF2_FLAG_FILE_CONTAINER = 0x00001000
UF2_FLAG_FAMILY_ID      = 0x00002000
UF2_FLAG_MD5_CHECKSUM   = 0x00004000
UF2_FLAG_EXTENSION_TAGS = 0x00008000 # Block padding contains extension tags

# Info file
INFO_FILE = "/INFO_UF2.TXT"
INFO_MODEL = "Model"
INFO_BOARD_ID = "Board-ID"

DEFAULT_FAMILY_ID = "RP2040"
DEFAULT_BASE_ADDR = 0x10000000 # Flash window start address

DEFAULT_WAIT_SECS = 20

# Content must be padded to complete flash sectors. Bootloader messes up otherwise.
FLASH_SECTOR_SIZE = 4096

# Global variables
base_address = 0
familyid = 0
verbose = False

def is_uf2(buf):
    w = struct.unpack("<II", buf[0:8])
    return w[0] == UF2_MAGIC_START0 and w[1] == UF2_MAGIC_START1


def convert_from_uf2(buf):
    """Return dictionary of addr=content blocks."""
    global familyid
    numblocks = len(buf) // UF2_BLOCK_SIZE
    if verbose:
        print("File size %u bytes, %u blocks" % (len(buf), numblocks))
    chunk_offset = None
    curraddr = None
    currfamilyid = None
    families_found = {}
    prev_flag = None
    all_flags_same = True

    output = {}
    if verbose:
        print("#Block  Magic 0     Magic 1     flags       address     payload sz  blockno     numblocks   family/sz")
        print("------  ----------  ----------  ----------  ----------  ----------  ----------  ----------  ----------")

    for blockno in range(numblocks):
        offset = blockno * UF2_BLOCK_SIZE
        block = buf[offset:offset + UF2_BLOCK_SIZE]
        hd = struct.unpack(b"<IIIIIIII", block[0:UF2_HEADER_SIZE])
        if verbose:
            s = "  ".join("0x%08x" % e for e in hd)
            print("%6u  %s" % (blockno, s))
        if hd[0] != UF2_MAGIC_START0 or hd[1] != UF2_MAGIC_START1:
            print("Skipping block at " + offset + ", bad magic")
            continue
        if hd[2] & 1:
            # NO-flash flag set; skip block
            continue
        datalen = hd[4]
        if datalen > UF2_MAX_PAYLOAD_SIZE:
            error("Invalid UF2 data size at 0x%08x" % offset)
        newaddr = hd[3]
        if chunk_offset is None or curraddr != newaddr:
            chunk_offset = newaddr - base_address
            output[chunk_offset] = bytes()
        if (hd[2] & UF2_FLAG_FAMILY_ID) and currfamilyid is None:
            currfamilyid = hd[7]
        if curraddr is None or ((hd[2] & UF2_FLAG_FAMILY_ID) and hd[7] != currfamilyid):
            currfamilyid = hd[7]
            curraddr = newaddr
        if familyid == 0x0 or ((hd[2] & UF2_FLAG_FAMILY_ID) and familyid == hd[7]):
            output[chunk_offset] += block[UF2_HEADER_SIZE : UF2_HEADER_SIZE + datalen]
        curraddr = newaddr + datalen
        if hd[2] & UF2_FLAG_FAMILY_ID:
            if hd[7] in families_found.keys():
                if families_found[hd[7]] > newaddr:
                    families_found[hd[7]] = newaddr
            else:
                families_found[hd[7]] = newaddr
        if prev_flag is None:
            prev_flag = hd[2]
        if prev_flag != hd[2]:
            all_flags_same = False

    print("----- UF2 File Header Info -----")
    families = load_families()
    for family_hex in families_found.keys():
        family_short_name = ""
        for name, value in families.items():
            if value == family_hex:
                family_short_name = name
        print("Family ID is %s, hex value is 0x%08x" % (family_short_name, family_hex))
        print("Target Address is 0x%08x" % families_found[family_hex])
    if all_flags_same:
        print("All block flag values consistent, 0x%04x" % hd[2])
    else:
        print("Flags were not all the same")
    print("  Offset      Size")
    for addr, content in output.items():
        print("  0x%08x  0x%06x  %u" % (addr, len(content), len(content)))
    print("--------------------------------")
    if len(families_found) > 1 and familyid == 0x0:
        output = []

    return output


def getSectorCount(length):
    return (length + FLASH_SECTOR_SIZE - 1) // FLASH_SECTOR_SIZE


def convert_to_uf2(content_map):
    global familyid
    global base_address

    numblocks = 0
    for addr, content in content_map.items():
        numblocks += getSectorCount(len(content)) * FLASH_SECTOR_SIZE // UF2_PAYLOAD_SIZE

    outp = []
    blockno = 0
    for addr in sorted(content_map):
        content = bytearray(content_map[addr])
        length = len(content)
        padding = (getSectorCount(length) * FLASH_SECTOR_SIZE) - length
        content.extend(bytearray([0xff] * padding))
        length = len(content)
        addr += base_address
        offset = 0
        while offset < length:
            chunk = content[offset:offset + UF2_PAYLOAD_SIZE]
            flags = 0x0
            if familyid:
                flags |= 0x2000
            block = struct.pack(b"<IIIIIIII",
                UF2_MAGIC_START0, UF2_MAGIC_START1,
                flags, addr + offset, 256, blockno, numblocks, familyid)
            block += chunk
            block += b"\x00" * (UF2_MAX_PAYLOAD_SIZE - UF2_PAYLOAD_SIZE)
            block += struct.pack(b"<I", UF2_MAGIC_END)
            if len(block) != UF2_BLOCK_SIZE:
                error("INTERNAL ERROR")
            outp.append(block)
            offset += UF2_PAYLOAD_SIZE
            blockno += 1
    return b"".join(outp)


def get_drives():
    drives = []
    if sys.platform == "win32":
        r = subprocess.check_output(["wmic", "PATH", "Win32_LogicalDisk",
                                     "get", "DeviceID,", "VolumeName,",
                                     "FileSystem,", "DriveType"])
        for line in r.decode().split('\n'):
            words = re.split(r'\s+', line)
            if len(words) >= 3 and words[1] == "2" and words[2] == "FAT":
                drives.append(words[0])
    else:
        r = subprocess.check_output(["findmnt", "-o", "TARGET", "-t", "vfat", "-ln"])
        for dev in r.decode().split('\n'):
            drives.append(dev)

    def has_info(d):
        try:
            return os.path.isfile(d + INFO_FILE)
        except:
            return False

    return list(filter(has_info, drives))


def board_info(path):
    infofile = path + INFO_FILE
    if not os.path.isfile(infofile):
        return None
    with open(infofile, mode='r') as f:
        lines = f.readlines()
    if len(lines) < 2:
        return None
    res = {}
    res['header'] = lines[0].strip()
    for line in lines[1:]:
        k, v = line.split(': ')
        res[k] = v.strip()
    return res


def list_drives():
    drives = get_drives()
    if len(drives) == 0:
        print("No drives found.")
        return
    for d in drives:
        info = board_info(d)
        print("'%s' %s (%s)" % (d, info[INFO_BOARD_ID], info[INFO_MODEL]))
        if verbose:
            for k, v in info.items():
                print("  %s = %s" % (k, v))


def write_file(name, content):
    with open(name, "wb") as f:
        f.write(content)
    print("Wrote '%s', %u bytes" % (name, len(content)))


def upload_file(drive, content):
    """Gives progress indication."""
    blockSize = UF2_BLOCK_SIZE * 32
    blockCount = (len(content) + blockSize - 1 ) // blockSize
    offset = 0
    lastpercent = -1

    # https://stackoverflow.com/questions/3173320/text-progress-bar-in-the-console
    def progress():
        nonlocal lastpercent
        block = (offset + blockSize - 1) // blockSize
        percent = round(100 * block / blockCount)
        if percent == lastpercent:
            return
        lastpercent = percent

        prefix = 'Progress:'
        suffix = 'Complete'
        length = 50
        fill = '█'
        filledLength = round(length * percent / 100)
        bar = fill * filledLength + '-' * (length - filledLength)
        print(f'\r{prefix} |{bar}| {percent}% {suffix}', end = "\r")

    with open(drive + "/NEW.UF2", "wb") as f:
        while offset < len(content):
            f.write(content[offset:offset+blockSize])
            offset += blockSize
            progress()

    print()


def load_families():
    # The expectation is that the `uf2families.json` file is in the same
    # directory as this script. Make a path that works using `__file__`
    # which contains the full path to this script.
    filename = "uf2families.json"
    pathname = os.path.join(os.path.dirname(os.path.abspath(__file__)), filename)
    with open(pathname) as f:
        raw_families = json.load(f)

    families = {}
    for family in raw_families:
        families[family["short_name"]] = int(family["id"], 0)

    return families


def error(msg):
    print(msg)
    sys.exit(1)


def main():
    global familyid
    global base_address
    global verbose

    parser = argparse.ArgumentParser(description='Utility to manage UF2 file conversion and flashing.')
    parser.add_argument('-b' , '--base', dest='base', type=str,
                        default="0x%08x" % DEFAULT_BASE_ADDR,
                        help='set base address for chunks (default: 0x%08x)' % DEFAULT_BASE_ADDR)
    parser.add_argument('input', metavar='INPUT', type=str, nargs='*',
                        help='Name of UF2 file to read, or addr=content pairs for source binary data')
    parser.add_argument('-o' , '--output', metavar="FILE", dest='output', type=str,
                        help='Write output to named file. Defaults to "flash.uf2" or "flash.bin" as appropriate.')
    parser.add_argument('-d' , '--device', dest="device_path",
                        help='Select a device path to flash. If not specified will auto-detect.')
    parser.add_argument('-w' , '--wait', dest="wait_secs", type=int,
                        default=DEFAULT_WAIT_SECS,
                        help='Time in seconds to wait for target device connection')
    parser.add_argument('-l' , '--list', action='store_true',
                        help='List connected devices')
    parser.add_argument('-c' , '--convert', action='store_true',
                        help='Convert list of "addr=file" binary source files into single .uf2 file')
    parser.add_argument('-u' , '--upload', action='store_true',
                        help='Just flash, do not convert')
    parser.add_argument('-f' , '--family', dest='family', type=str,
                        default=DEFAULT_FAMILY_ID,
                        help='Specify familyID number or name (default: %s)' % DEFAULT_FAMILY_ID)
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='enable verbose console messages')
    args = parser.parse_args()

    base_address = int(args.base, 0)
    verbose = args.verbose

    families = load_families()
    familyid = families.get(args.family.upper())
    if familyid is None:
        try:
            familyid = int(args.family, 0)
        except ValueError:
            error("Family ID needs to be a number or one of: " + ", ".join(families.keys()))

    if args.list:
        list_drives()
        return

    if not args.input:
        parser.print_usage()
        error("Need input file(s)")

    source_map = None
    output_map = None
    uf2 = None
    if len(args.input) == 1 and args.input[0].endswith('.uf2') and not args.convert:
        filename = args.input[0]
        with open(filename, mode='rb') as f:
            inpbuf = f.read()
        if not is_uf2(inpbuf):
            error("Not a UF2 file: %s" % filename)
        output_map = convert_from_uf2(inpbuf)
    else:
        source_map = {}
        for e in args.input:
            try:
                addr, srcfile = e.split('=')
                addr = int(addr, 0)
            except ValueError:
                error("Invalid parameter '%s', expecting addr=content pair" % e)
            with open(srcfile, mode='rb') as f:
                content = f.read()
            source_map[addr] = content
        uf2 = convert_to_uf2(source_map)

    if args.output:
        if uf2 is not None:
            write_file(args.output, uf2)
        if output_map is not None:
            base = os.path.splitext(args.output)
            for addr, content in output_map.items():
                filename = "%s-0x%08x%s" % (base[0], addr, base[1])
                with open(filename, "wb") as f:
                    f.write(content)
                print("Wrote '%s', %u bytes" % (filename, len(content)))

    if args.upload or args.device_path is not None:
        d = None
        info = None
        for attempt in range(args.wait_secs, 0, -1):
            print("Waiting for target device to enter boot mode... %u  " % attempt, end='\r')
            sys.stdout.flush()
            d = args.device_path
            if d is None:
                drives = get_drives()
                if len(drives) != 0:
                    d = drives[0]
            if d is not None:
                info = board_info(d)
                if info is not None:
                    break
            time.sleep(1)

        if info is None:
            error("\nUF2 drive not found!")

        print("Flashing to '%s' (%s / %s)" % (d, info[INFO_MODEL], info[INFO_BOARD_ID]))
        upload_file(d, uf2)


if __name__ == "__main__":
    main()
