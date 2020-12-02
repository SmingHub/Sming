#!/usr/bin/env python3
#
# Sming hardware configuration tool
#
# Forked from Espressif gen_esp32part.py
#
# Original license:
#
# Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
from __future__ import print_function, division
from __future__ import unicode_literals
import argparse
import os
import struct
import sys
import hashlib
import binascii
import errno
import json, configparser

MAX_PARTITION_LENGTH = 0xC00  # 3K for partition data (96 entries) leaves 1K in a 4K sector for signature
MD5_PARTITION_BEGIN = b"\xEB\xEB" + b"\xFF" * 14  # The first 2 bytes are like magic numbers for MD5 sum
PARTITION_TABLE_SIZE = 0x1000  # Size of partition table

MIN_PARTITION_SUBTYPE_APP_OTA = 0x10
NUM_PARTITION_SUBTYPE_APP_OTA = 16

__version__ = '1.2'

APP_TYPE = 0x00
DATA_TYPE = 0x01

TYPES = {
    "app": APP_TYPE,
    "data": DATA_TYPE,
}

# Keep this map in sync with esp_partition_subtype_t enum in esp_partition.h
SUBTYPES = {
    APP_TYPE: {
        "factory": 0x00,
        "test": 0x20,
    },
    DATA_TYPE: {
        "ota": 0x00,
        "phy": 0x01,
        "nvs": 0x02,
        "coredump": 0x03,
        "nvs_keys": 0x04,
        "efuse": 0x05,
        "esphttpd": 0x80,
        "fat": 0x81,
        "spiffs": 0x82,
    },
}

quiet = False
md5sum = True
secure = False
offset_part_table = 0


def status(msg):
    """ Print status message to stderr """
    if not quiet:
        critical(msg)


def critical(msg):
    """ Print critical message to stderr """
    sys.stderr.write(msg)
    sys.stderr.write('\n')


class PartitionTable(list):

    def __init__(self):
        super(PartitionTable, self).__init__(self)

    @classmethod
    def from_json(cls, table):
        res = PartitionTable()

        if 'offset' in table:
            global offset_part_table
            offset_part_table = parse_int(table['offset'])

        index = 0
        for (label, entry) in table['entries'].items():
            try:
                res.append(PartitionDefinition.from_json(label, entry, index))
                index += 1
            except InputError as e:
                raise InputError("Error in partition '%s'" % (label))
            except Exception:
                critical("Unexpected error in partition '%s': %s" % (label, entry))
                raise

        # fix up missing offsets & negative sizes
        last_end = offset_part_table + PARTITION_TABLE_SIZE  # first offset after partition table
        for e in res:
            if e.offset is not None and e.offset < last_end:
                if e == res[0]:
                    raise InputError("Error: First partition offset 0x%x overlaps end of partition table 0x%x"
                                     % (e.offset, last_end))
                else:
                    raise InputError("Error: Partitions overlap. Partition '%s' sets offset 0x%x. Previous partition ends 0x%x"
                                     % (e.name, e.offset, last_end))
            if e.offset is None:
                pad_to = e.alignment()
                if last_end % pad_to != 0:
                    last_end += pad_to - (last_end % pad_to)
                e.offset = last_end
            if e.size < 0:
                e.size = -e.size - e.offset
            last_end = e.offset + e.size

        return res

    def __getitem__(self, item):
        """ Allow partition table access via name as well as by
        numeric index. """
        if isinstance(item, str):
            for x in self:
                if x.name == item:
                    return x
            raise ValueError("No partition entry named '%s'" % item)
        else:
            return super(PartitionTable, self).__getitem__(item)

    def find_by_type(self, ptype, subtype):
        """ Return a partition by type & subtype, returns
        None if not found """
        # convert ptype & subtypes names (if supplied this way) to integer values
        try:
            ptype = TYPES[ptype]
        except KeyError:
            try:
                ptype = int(ptype, 0)
            except TypeError:
                pass
        try:
            subtype = SUBTYPES[int(ptype)][subtype]
        except KeyError:
            try:
                subtype = int(subtype, 0)
            except TypeError:
                pass

        for p in self:
            if p.type == ptype and p.subtype == subtype:
                yield p
        return

    def find_by_name(self, name):
        for p in self:
            if p.name == name:
                return p
        return None

    def verify(self):
        # verify each partition individually
        for p in self:
            p.verify()

        # check on duplicate name
        names = [p.name for p in self]
        duplicates = set(n for n in names if names.count(n) > 1)

        # print sorted duplicate partitions by name
        if len(duplicates) != 0:
            print("A list of partitions that have the same name:")
            for p in sorted(self, key=lambda x:x.name):
                if len(duplicates.intersection([p.name])) != 0:
                    print("%s" % (p.to_csv()))
            raise InputError("Partition names must be unique")

        # check for overlaps
        last = None
        for p in sorted(self, key=lambda x:x.offset):
            if p.offset < offset_part_table + PARTITION_TABLE_SIZE:
                raise InputError("Partition '%s' offset 0x%x is below 0x%x" % (p.name, p.offset, offset_part_table + PARTITION_TABLE_SIZE))
            if last is not None and p.offset < last.offset + last.size:
                raise InputError("Partition '%s' at 0x%x overlaps 0x%x-0x%x" % (p.name, p.offset, last.offset, last.offset + last.size - 1))
            last = p

    def flash_size(self):
        """ Return the size that partitions will occupy in flash
            (ie the offset the last partition ends at)
        """
        try:
            last = sorted(self, reverse=True)[0]
        except IndexError:
            return 0  # empty table!
        return last.offset + last.size

    @classmethod
    def from_binary(cls, b):
        md5 = hashlib.md5()
        result = cls()
        for o in range(0, len(b), 32):
            data = b[o:o + 32]
            if len(data) != 32:
                raise InputError("Partition table length must be a multiple of 32 bytes")
            if data == b'\xFF' * 32:
                return result  # got end marker
            if md5sum and data[:2] == MD5_PARTITION_BEGIN[:2]:  # check only the magic number part
                if data[16:] == md5.digest():
                    continue  # the next iteration will check for the end marker
                else:
                    raise InputError("MD5 checksums don't match! (computed: 0x%s, parsed: 0x%s)" % (md5.hexdigest(), binascii.hexlify(data[16:])))
            else:
                md5.update(data)
            result.append(PartitionDefinition.from_binary(data))
        raise InputError("Partition table is missing an end-of-table marker")

    def to_binary(self):
        result = b"".join(e.to_binary() for e in self)
        if md5sum:
            result += MD5_PARTITION_BEGIN + hashlib.md5(result).digest()
        if len(result) >= MAX_PARTITION_LENGTH:
            raise InputError("Binary partition table length (%d) longer than max" % len(result))
        result += b"\xFF" * (MAX_PARTITION_LENGTH - len(result))  # pad the sector, for signing
        return result

    def to_csv(self, simple_formatting=False):
        rows = ["# ESP-IDF Partition Table",
                "# Name, Type, SubType, Offset, Size, Flags"]
        rows += [x.to_csv(simple_formatting) for x in self]
        return "\n".join(rows) + "\n"


class PartitionDefinition(object):
    MAGIC_BYTES = b"\xAA\x50"

    ALIGNMENT = {
        APP_TYPE: 0x1000,
        DATA_TYPE: 0x04,
    }

    # dictionary maps flag name (as used in CSV flags list, property name)
    # to bit set in flags words in binary format
    FLAGS = {
        "encrypted": 0
    }

    # add subtypes for the 16 OTA slot values ("ota_XX, etc.")
    for ota_slot in range(NUM_PARTITION_SUBTYPE_APP_OTA):
        SUBTYPES[TYPES["app"]]["ota_%d" % ota_slot] = MIN_PARTITION_SUBTYPE_APP_OTA + ota_slot

    def __init__(self):
        self.name = ""
        self.type = None
        self.subtype = None
        self.offset = None
        self.size = None
        self.encrypted = False

    @classmethod
    def from_json(cls, label, entry, index):
        """ Parse a JSON partition table entry """
        res = PartitionDefinition()
        res.line_no = index
        res.name = label
        res.type = res.parse_type(entry['type'])
        res.subtype = res.parse_subtype(entry['subtype'])
        res.offset = res.parse_address(entry['address'])
        res.size = res.parse_address(entry['size'])
        if res.offset is None or res.size is None:
            raise InputError("Offset/Size fields may not be empty")

        if 'flags' in entry:
            flags = entry['flags'].split(":")
            for flag in flags:
                if flag in cls.FLAGS:
                    setattr(res, flag, True)
                elif len(flag) > 0:
                    raise InputError("Unknown flag '%s' in partition entry '%s'" % (flag, label))

        return res

    def __eq__(self, other):
        return self.name == other.name and self.type == other.type \
            and self.subtype == other.subtype and self.offset == other.offset \
            and self.size == other.size

    def __repr__(self):

        def maybe_hex(x):
            return "0x%x" % x if x is not None else "None"

        return "PartitionDefinition('%s', 0x%x, 0x%x, %s, %s)" % (self.name, self.type, self.subtype or 0,
                                                                  maybe_hex(self.offset), maybe_hex(self.size))

    def __str__(self):
        return "Part '%s' %d/%d @ 0x%x size 0x%x" % (self.name, self.type, self.subtype, self.offset or -1, self.size or -1)

    def __cmp__(self, other):
        return self.offset - other.offset

    def __lt__(self, other):
        return self.offset < other.offset

    def __gt__(self, other):
        return self.offset > other.offset

    def __le__(self, other):
        return self.offset <= other.offset

    def __ge__(self, other):
        return self.offset >= other.offset

    def parse_type(self, strval):
        if strval == "":
            raise InputError("Field 'type' can't be left empty.")
        return parse_int(strval, TYPES)

    def parse_subtype(self, strval):
        if strval == "":
            return 0  # default
        return parse_int(strval, SUBTYPES.get(self.type, {}))

    def parse_address(self, strval):
        if strval == "":
            return None  # PartitionTable will fill in default
        return parse_int(strval)

    def alignment(self):
        return self.ALIGNMENT.get(self.type, 4)

    def verify(self):
        if self.type is None:
            raise ValidationError(self, "Type field is not set")
        if self.subtype is None:
            raise ValidationError(self, "Subtype field is not set")
        if self.offset is None:
            raise ValidationError(self, "Offset field is not set")
        align = self.alignment()
        if self.offset % align:
            raise ValidationError(self, "Offset 0x%x is not aligned to 0x%x" % (self.offset, align))
        if self.size % align and secure:
            raise ValidationError(self, "Size 0x%x is not aligned to 0x%x" % (self.size, align))
        if self.size is None:
            raise ValidationError(self, "Size field is not set")

        if self.name in TYPES and TYPES.get(self.name, "") != self.type:
            critical("WARNING: Partition has name '%s' which is a partition type, but does not match this partition's "
                     "type (0x%x). Mistake in partition table?" % (self.name, self.type))
        all_subtype_names = []
        for names in (t.keys() for t in SUBTYPES.values()):
            all_subtype_names += names
        if self.name in all_subtype_names and SUBTYPES.get(self.type, {}).get(self.name, "") != self.subtype:
            critical("WARNING: Partition has name '%s' which is a partition subtype, but this partition has "
                     "non-matching type 0x%x and subtype 0x%x. Mistake in partition table?" % (self.name, self.type, self.subtype))

    STRUCT_FORMAT = b"<2sBBLL16sL"

    @classmethod
    def from_binary(cls, b):
        if len(b) != 32:
            raise InputError("Partition definition length must be exactly 32 bytes. Got %d bytes." % len(b))
        res = cls()
        (magic, res.type, res.subtype, res.offset,
         res.size, res.name, flags) = struct.unpack(cls.STRUCT_FORMAT, b)
        if b"\x00" in res.name:  # strip null byte padding from name string
            res.name = res.name[:res.name.index(b"\x00")]
        res.name = res.name.decode()
        if magic != cls.MAGIC_BYTES:
            raise InputError("Invalid magic bytes (%r) for partition definition" % magic)
        for flag, bit in cls.FLAGS.items():
            if flags & (1 << bit):
                setattr(res, flag, True)
                flags &= ~(1 << bit)
        if flags != 0:
            critical("WARNING: Partition definition had unknown flag(s) 0x%08x. Newer binary format?" % flags)
        return res

    def get_flags_list(self):
        return [flag for flag in self.FLAGS.keys() if getattr(self, flag)]

    def to_binary(self):
        flags = sum((1 << self.FLAGS[flag]) for flag in self.get_flags_list())
        return struct.pack(self.STRUCT_FORMAT,
                           self.MAGIC_BYTES,
                           self.type, self.subtype,
                           self.offset, self.size,
                           self.name.encode(),
                           flags)

    def to_csv(self, simple_formatting=False):

        def addr_format(a, include_sizes):
            if not simple_formatting and include_sizes:
                for (val, suffix) in [(0x100000, "M"), (0x400, "K")]:
                    if a % val == 0:
                        return "%d%s" % (a // val, suffix)
            return "0x%x" % a

        def lookup_keyword(t, keywords):
            for k, v in keywords.items():
                if simple_formatting is False and t == v:
                    return k
            return "%d" % t

        def generate_text_flags():
            """ colon-delimited list of flags """
            return ":".join(self.get_flags_list())

        return ",".join([self.name,
                         lookup_keyword(self.type, TYPES),
                         lookup_keyword(self.subtype, SUBTYPES.get(self.type, {})),
                         addr_format(self.offset, False),
                         addr_format(self.size, True),
                         generate_text_flags()])


def parse_int(v, keywords={}):
    """Generic parser for integer fields - int(x,0) with provision for
    k/m/K/M suffixes and 'keyword' value lookup.
    """
    try:
        for letter, multiplier in [("k", 1024), ("m", 1024 * 1024)]:
            if v.lower().endswith(letter):
                return parse_int(v[:-1], keywords) * multiplier
        return int(v, 0)
    except ValueError:
        if len(keywords) == 0:
            raise InputError("Invalid field value %s" % v)
        try:
            return keywords[v.lower()]
        except KeyError:
            raise InputError("Value '%s' is not valid. Known keywords: %s" % (v, ", ".join(keywords)))


def createConfig(input, output):
    """Parse makefile variables to create default configuration"""

    print("createConfig(%s, %s)" % (input.name, output))
    parser = configparser.ConfigParser()
    parser.optionxform = str  # Preserve case
    data = "[DEFAULT]\r\n" + input.read().decode()
    parser.read_string(data)
    vars = parser['DEFAULT']

    config = {}
    config['name'] ='Generated configuration'
    arch = os.environ['SMING_ARCH']
    config['arch'] = arch
    if arch == 'Esp8266':
        config['flash-size'] = vars['SPI_SIZE']
        config['spi-mode'] = vars['SPI_MODE']
        config['spi-speed'] = vars['SPI_SPEED']
    table = {}
    config['partition-table'] = table
    table['offset'] = "0x2000"
    entries = {}
    table['entries'] = entries

    def createEntry(address, size, type, subtype):
        entry = {}
        entry['address'] = '0x%06x' % address
        entry['size'] = '0x%06x' % size
        entry['type'] = type
        entry['subtype'] = subtype
        return entry
    
    def tryAddEntry(label, addressVar, type, subtype):
        if not addressVar in vars:
            return
        s = vars[addressVar]
        if s == '':
            return None
        addr = parse_int(s)
        size= 0x100000 - (addr % 0x100000)
        entry = createEntry(addr, size, type, subtype)
        entries[label] = entry
        return entry
    
    def addMake(entry, target):
        make = {}
        make['target'] = target
        entry['make'] = make
        return make

    def firmwareFilename(name, ext = '.bin'):
        return os.environ['BUILD_BASE'] + '/' + vars[name] + ext

    if arch == 'Esp32':
        entries['nvs'] = createEntry(0x9000, 0x6000, "data", "nvs")
        entries['phy_init'] = createEntry(0xf000, 0x1000, "data", "phy")
        entries['factory'] = createEntry(0x10000, 0x1f000, "app", "factory")
        vars['RBOOT_SPIFFS_0'] = '0x100000'
    else:
        entries['phy_init'] = createEntry(0x3000, 0x1000, "data", "phy")
        entries['sysconfig'] = createEntry(0x4000, 0x4000, "data", "nvs")
        entry = tryAddEntry('rom0', 'RBOOT_ROM0_ADDR', 'app', 'factory')
        if not entry is None:
            entry['filename'] = firmwareFilename('RBOOT_ROM_0')
            addMake(entry, 'fwimage')
        entry = tryAddEntry('rom1', 'RBOOT_ROM1_ADDR', 'app', 'ota_0')
        if not entry is None:
            entry['filename'] = firmwareFilename('RBOOT_ROM_1')
            addMake(entry, 'fwimage')
        tryAddEntry('rom2', 'RBOOT_ROM2_ADDR', 'app', 'ota_1')

    entry = tryAddEntry('spiffs1', 'RBOOT_SPIFFS_0', 'data', 'spiffs')
    if vars['DISABLE_SPIFFS'] == '0' and not entry is None:
        entry['filename'] = firmwareFilename('SPIFF_BIN')
        make = addMake(entry, 'spiffsgen')
        make['content'] = vars['SPIFF_FILES']
        make['size'] = vars['SPIFF_SIZE']
    tryAddEntry('spiffs2', 'RBOOT_SPIFFS_1', 'data', 'spiffs')

    with sys.stdout if output == '-' else open(output, 'w') as f:
        f.write(json.dumps(config, indent=4))



def main():
    global quiet
    global md5sum
    global offset_part_table
    global secure
    parser = argparse.ArgumentParser(description='Sming hardware configuration utility')

    parser.add_argument('--flash-size', help='Optional flash size limit, checks partition table fits in flash',
                        nargs='?', choices=['1MB', '2MB', '4MB', '8MB', '16MB'])
    parser.add_argument('--disable-md5sum', help='Disable md5 checksum for the partition table', default=False, action='store_true')
    parser.add_argument('--no-verify', help="Don't verify partition table fields", action='store_true')
    parser.add_argument('--verify', '-v', help="Verify partition table fields (deprecated, this behaviour is "
                                               "enabled by default and this flag does nothing.", action='store_true')
    parser.add_argument('--quiet', '-q', help="Don't print non-critical status messages to stderr", action='store_true')
    parser.add_argument('--offset', '-o', help='Set offset partition table', default='0x8000')
    parser.add_argument('--secure', help="Require app partitions to be suitable for secure boot", action='store_true')
    parser.add_argument('input', help='Path to JSON or binary file to parse.', type=argparse.FileType('rb'))
    parser.add_argument('output', help='Path to output converted binary or CSV file. Will use stdout if omitted.',
                        nargs='?', default='-')

    args = parser.parse_args()

    quiet = args.quiet
    md5sum = not args.disable_md5sum
    secure = args.secure
    offset_part_table = int(args.offset, 0)

    if args.input.name.endswith('.mk'):
        createConfig(args.input, args.output)
        return

    input = args.input.read()
    input_is_binary = (input[0:2] == PartitionDefinition.MAGIC_BYTES)
    if args.input.name.endswith('.hw'):
        status("Parsing JSON input...")
        config = json.loads(input.decode())
        table = PartitionTable.from_json(config['partition-table'])
    elif input_is_binary:
        status("Parsing binary partition input...")
        table = PartitionTable.from_binary(input)
    else:
        raise InputError("Unknown input file format")

    if not args.no_verify:
        status("Verifying table...")
        table.verify()

    if args.flash_size:
        size_mb = int(args.flash_size.replace("MB", ""))
        size = size_mb * 1024 * 1024  # flash memory uses honest megabytes!
        table_size = table.flash_size()
        if size < table_size:
            raise InputError("Partitions defined in '%s' occupy %.1fMB of flash (%d bytes) which does not fit in configured "
                             "flash size %dMB. Please change SPI_SIZE setting." % 
                             (args.input.name, table_size / 1024.0 / 1024.0, table_size, size_mb))

    # Make sure that the output directory is created
    output_dir = os.path.abspath(os.path.dirname(args.output))

    if not os.path.exists(output_dir):
        try:
            os.makedirs(output_dir)
        except OSError as exc:
            if exc.errno != errno.EEXIST:
                raise

    if input_is_binary:
        output = table.to_csv()
        with sys.stdout if args.output == '-' else open(args.output, 'w') as f:
            f.write(output)
    else:
        output = table.to_binary()
        try:
            stdout_binary = sys.stdout.buffer  # Python 3
        except AttributeError:
            stdout_binary = sys.stdout
        with stdout_binary if args.output == '-' else open(args.output, 'wb') as f:
            f.write(output)


class InputError(RuntimeError):

    def __init__(self, e):
        super(InputError, self).__init__(e)


class ValidationError(InputError):

    def __init__(self, partition, message):
        super(ValidationError, self).__init__(
            "Partition %s invalid: %s" % (partition.name, message))


if __name__ == '__main__':
    try:
        main()
    except InputError as e:
        print(e, file=sys.stderr)
        sys.exit(2)
