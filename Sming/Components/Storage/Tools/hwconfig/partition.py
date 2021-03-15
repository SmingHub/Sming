#!/usr/bin/env python3
#
# From Espressif gen_esp32part.py
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

import struct, hashlib, storage, binascii
from common import *

MAX_PARTITION_LENGTH = 0xC00  # 3K for partition data (96 entries) leaves 1K in a 4K sector for signature
MD5_PARTITION_BEGIN = b"\xEB\xEB" + b"\xFF" * 14  # The first 2 bytes are like magic numbers for MD5 sum
PARTITION_TABLE_SIZE = 0x1000  # Size of partition table
PARTITION_ENTRY_SIZE = 32

MIN_PARTITION_SUBTYPE_APP_OTA = 0x10
NUM_PARTITION_SUBTYPE_APP_OTA = 16

__version__ = '1.2'

APP_TYPE = 0x00
DATA_TYPE = 0x01
STORAGE_TYPE = 0x02 # Reference to storage device
USER_TYPE = 0x40 # First user-defined type

# Default is 4
ALIGNMENT = {
    "Esp32": {
        APP_TYPE: 0x10000,
    },
    "Esp8266": {
        APP_TYPE: 0x1000,
    },
    "Host": {
        APP_TYPE: 0x1000,
    }
}

TYPES = {
    "app": APP_TYPE,
    "data": DATA_TYPE,
    "storage": STORAGE_TYPE,
    "user": USER_TYPE,
}

# Keep this map in sync with esp_partition_subtype_t enum in esp_partition.h
SUBTYPES = {
    APP_TYPE: {
        "factory": 0x00,
        "ota_0": 0x10,
        "ota_1": 0x11,
        "ota_2": 0x12,
        "ota_3": 0x13,
        "ota_4": 0x14,
        "ota_5": 0x15,
        "ota_6": 0x16,
        "ota_7": 0x17,
        "ota_8": 0x18,
        "ota_9": 0x19,
        "ota_10": 0x1a,
        "ota_11": 0x1b,
        "ota_12": 0x1c,
        "ota_13": 0x1d,
        "ota_14": 0x1e,
        "ota_15": 0x1f,
        "test": 0x20,
    },
    DATA_TYPE: {
        "ota": 0x00,
        "phy": 0x01,
        "nvs": 0x02,
        "coredump": 0x03,
        "nvs_keys": 0x04,
        "efuse": 0x05,
        "sysparam": 0x40,
        "esphttpd": 0x80,
        "fat": 0x81,
        "spiffs": 0x82,
        "fwfs": 0xf1,
    },
    STORAGE_TYPE: storage.TYPES
}


def parse_type(value):
    if isinstance(value, str):
        if value == "":
            raise InputError("Field 'type' can't be left empty.")
        return parse_int(value, TYPES)
    return value


def parse_subtype(ptype, value):
    if isinstance(value, str):
        if value == "":
            return 0  # default
        return parse_int(value, SUBTYPES.get(ptype, {}))
    return value


class Table(list):

    def __init__(self):
        super(Table, self).__init__(self)

    def parse_dict(self, data, devices):
        partnames = []
        for name, entry in data.items():
            if name in partnames:
                raise InputError("Duplicate partition '%s'" % name)
            if contains_whitespace(name):
                raise InputError("Partition names may not contain spaces '%s'" % name)
            partnames += name
            part = self.find_by_name(name)
            if part is None:
                part = Entry(devices[0], name)
                self.append(part)
            part.parse_dict(entry, devices)

    def sort(self):
        super(Table, self).sort(key=lambda p: p.device.name + p.address_str())

    def dict(self):
        res = {}
        for entry in self:
            res[entry.name] = entry.dict()
        return res

    def to_json(self):
        return to_json(self.dict());

    def to_csv(self):
        res =  "Device            Start       End         Size        Type      SubType   Name              Filename\n"
        res += "----------------  ----------  ----------  ----------  --------  --------  ----------------  ------------\n"
        for p in self:
            res += "%-16s  %10s  %10s  %10s  %-8s  %-8s  %-16s  %s\n" \
                % (p.device.name, p.address_str(), p.end_str(), p.size_str(), p.type_str(), p.subtype_str(), p.name, p.filename)
        return res

    def offset_str(self):
        return addr_format(self.offset)

    def buildVars(self):
        dict = {}
        dict['PARTITION_NAMES'] = " ".join(p.name for p in self)
        for p in self:
            dict.update(p.buildVars())
        return dict

    def __getitem__(self, item):
        """Allow partition table access by name or index
        """
        if isinstance(item, str):
            p = self.find_by_name(item)
            if p is None:
                raise ValueError("No partition entry named '%s'" % item)
            return p
        return super(Table, self).__getitem__(item)

    def find_by_type(self, ptype, subtype):
        """Return a partition by type & subtype, returns None if not found
        """
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

    def find_by_address(self, addr):
        for p in self:
            if p.contains(addr):
                return p
        return None

    def verify(self, arch, secure):
        """Verify partition layout
        """
        # verify each partition individually
        for p in self:
            p.verify(arch, secure)

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
        minPartitionAddress = self.offset + PARTITION_TABLE_SIZE
        dev = ''
        last = None
        for p in self:
            if p.device != dev:
                last = None
                dev = p.device
            if dev == self[0].device and p.address < minPartitionAddress:
                raise InputError("Partition '%s' @ %s-%s must be located after @ %s" \
                                 % (p.name, p.address_str(), p.end_str(), addr_format(minPartitionAddress)))
            if last is not None and p.address <= last.end():
                raise InputError("Partition '%s' @ %s-%s overlaps '%s' @ %s-%s" \
                                 % (p.name, p.address_str(), p.end_str(), \
                                    last.name, last.address_str(), last.end_str()))
            last = p

    def parse_binary(self, b, devices):
        """Construct partition table object from binary image
        """
        dev = None
        md5 = hashlib.md5()
        for o in range(0, len(b), PARTITION_ENTRY_SIZE):
            data = b[o:o + PARTITION_ENTRY_SIZE]
            if len(data) != PARTITION_ENTRY_SIZE:
                raise InputError("Partition table length must be a multiple of %d bytes" % PARTITION_ENTRY_SIZE)
            if data == b'\xFF' * PARTITION_ENTRY_SIZE:
                return # got end marker

            if data[:2] == MD5_PARTITION_BEGIN[:2]:  # check only the magic number part
                if data[16:] == md5.digest():
                    md5.update(data)
                    continue  # the next iteration will check for the end marker
                else:
                    raise InputError("MD5 checksums don't match! (computed: 0x%s, parsed: 0x%s)" % (md5.hexdigest(), binascii.hexlify(data[16:])))

            md5.update(data)

            if data[:2] == b"\xff\xff": # Pseudo-end marker to keep esp32 bootloader happy
                continue

            e = Entry.from_binary(data)
            if e.type == STORAGE_TYPE:
                dev = storage.Device(e.name, e.subtype, e.size)
                devices.append(dev)
            else:
                e.device = dev
                self.append(e)
        raise InputError("Partition table is missing an end-of-table marker")

    def to_binary(self, devices):
        """Create binary image of partition table
        """
        dev_count = 0
        dev = None
        result = b""
        for e in self:
            if e.device != dev:
                if dev_count == 1:
                    result += MD5_PARTITION_BEGIN + hashlib.md5(result).digest()
                    # esp32 bootloader will see this as end of partition table 
                    result += struct.pack(Entry.STRUCT_FORMAT,
                                b"\xff\xff",
                                0xff, 0xff,
                                0, 0,
                                b"SMING EXTENSIONS",
                                0)
                dev = e.device
                s = Entry(dev, dev.name, 0, dev.size, STORAGE_TYPE, dev.type)
                result += s.to_binary()
                dev_count += 1
            result += e.to_binary()

        result += MD5_PARTITION_BEGIN + hashlib.md5(result).digest()

        if len(result) >= MAX_PARTITION_LENGTH:
            raise InputError("Binary partition table length (%d) longer than max" % len(result))
        result += b"\xFF" * (MAX_PARTITION_LENGTH - len(result))  # pad the sector, for signing
        return result


class Entry(object):
    MAGIC_BYTES = b"\xAA\x50"

    # dictionary maps flag name (as used in CSV flags list, property name)
    # to bit set in flags words in binary format
    FLAGS = {
        "encrypted": 0,
        "readonly": 31,
    }

    def __init__(self, device=None, name="", address=None, size=None, ptype=None, subtype=None):
        self.device = device
        self.name = name
        self.address = address
        self.size = size
        self.type = parse_type(ptype)
        self.subtype = parse_subtype(self.type, subtype)
        self.readonly = False
        self.encrypted = False
        self.filename = ''
        self.build = None


    def parse_dict(self, data, devices):
        """Construct a partition object from JSON definition
        """
        try:
            # Sort out type information first
            v = data.pop('type', None)
            if not v is None:
                self.type = parse_type(v)
            v = data.pop('subtype', None)
            if not v is None:
                self.subtype = parse_subtype(self.type, v)
            if self.type is None or self.subtype is None:
                raise InputError("type/subtype missing")

            for k, v in data.items():
                if k == 'device':
                    self.device = devices.find_by_name(v)
                elif k == 'address':
                    self.address = parse_int(v)
                elif k == 'size':
                    self.size = parse_int(v)
                elif k == 'filename':
                    self.filename = v
                elif k == 'build':
                    if self.build is None:
                        self.build = v
                    else:
                        self.build.update(v)
                else:
                    setattr(self, k, v)

            if self.address is None or self.size is None:
                raise InputError("address/size missing")
            if self.end() >= self.device.size:
                raise InputError("Partition '%s' %s-%s too big for %s size %s" \
                                 % (self.name, self.address_str(), self.end_str(), self.device.name, self.device.size_str()))
        except InputError as e:
            raise InputError("Error in partition entry '%s': %s" % (self.name, e))


    def dict(self):
        res = {}
        for k, v in self.__dict__.items():
            if k == 'device':
                res[k] = v.name
            elif k == 'address':
                res[k] = self.address_str()
            elif k == 'size':
                res[k] = self.size_str()
            elif k == 'type':
                res[k] = stringnum(self.type_str())
            elif k == 'subtype':
                res[k] = stringnum(self.subtype_str())
            elif v is not None and k != 'name':
                res[k] = v
        return res

    def buildVars(self):
        res = {}

        dict = self.dict()
        dict['size_bytes'] = "0x%x" % self.size
        dict.pop('build', None)
        for k, v in dict.items():
            k = "PARTITION_%s_%s" % (self.name, k.upper())
            res[k] = int(v) if type(v) is bool else v

        return res

    def to_json(self):
        return to_json(self.dict());

    def end(self):
        return self.address + self.size - 1

    def end_str(self):
        return addr_format(self.end())

    def address_str(self):
        return addr_format(self.address)

    def size_str(self):
        return size_format(self.size)

    def contains(self, addr):
        return addr >= self.address and addr <= self.end()

    def type_str(self):
        return "" if self.type == 0xff else lookup_keyword(self.type, TYPES)

    def type_is(self, t):
        return self.type_str() == t if isinstance(t, str) else self.type == t

    def subtype_str(self):
        return "" if self.subtype == 0xff else lookup_keyword(self.subtype, SUBTYPES.get(self.type, {}))

    def subtype_is(self, subtype):
        return self.subtype_str() == subtype if isinstance(subtype, str) else self.subtype == subtype

    def __eq__(self, other):
        if isinstance(other, str):
            return self.name == other
        else:
            return self.name == other.name and self.type == other.type \
                and self.subtype == other.subtype and self.address == other.address \
                and self.size == other.size

    def __repr__(self):

        def maybe_hex(x):
            return "0x%x" % x if x is not None else "None"

        return "Entry('%s', 0x%x, 0x%x, %s, %s)" % (self.name, self.type, self.subtype or 0,
                                                                  maybe_hex(self.address), maybe_hex(self.size))

    def __str__(self):
        return "Part '%s' %s/%s @ 0x%x size 0x%x" % (self.name, self.type_str(), self.subtype_str(), self.address or -1, self.size or -1)

    def alignment(self, arch):
        return ALIGNMENT[arch].get(self.type, 4)

    def verify(self, arch, secure):
        if self.type is None:
            raise ValidationError(self, "Type field is not set")
        if self.subtype is None:
            raise ValidationError(self, "Subtype field is not set")
        if self.address is None:
            raise ValidationError(self, "Address field is not set")
        align = self.alignment(arch)
        if self.address % align:
            raise ValidationError(self, "Offset 0x%x is not aligned to 0x%x" % (self.address, align))
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
        if len(b) != PARTITION_ENTRY_SIZE:
            raise InputError("Partition entry size incorrect. Expected %d bytes, got %d." % (PARTITION_ENTRY_SIZE, len(b)))
        res = cls()
        (magic, res.type, res.subtype, res.address,
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

    def to_binary(self):
        flags = 0
        for key in self.FLAGS:
            if getattr(self, key):
                flags |= (1 << self.FLAGS[key])
        return struct.pack(self.STRUCT_FORMAT,
                           self.MAGIC_BYTES,
                           self.type, self.subtype,
                           self.address, self.size,
                           self.name.encode(),
                           flags)


class Map(Table):
    """Contiguous map of flash memory
    """
    def __init__(self, table, devices):
        device = devices[0]

        def add(name, address, size):
            entry = Entry(device, name, address, size, 0xff, 0xff)
            self.append(entry)
            return entry

        def add_unused(address, last_end):
            if address > last_end + 1:
                add('(unused)', last_end + 1, address - last_end - 1)

        if table.offset == 0:
            last = None
        else:
            add("Boot Sector", 0, table.offset)
            last = add("Partition Table", table.offset, PARTITION_TABLE_SIZE)

        for p in table:
            if last is not None:
                if p.device != last.device:
                    add_unused(last.device.size, last.end())
                    device = p.device
                elif p.address > last.end() + 1:
                    add_unused(p.address, last.end())
            self.append(p)
            last = p

        add_unused(device.size, last.end())
