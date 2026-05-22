from config import Config
import struct
import partition
import json

def is_block(data: bytes):
    PICO_MAGIC_BYTES = b"\xD3\xDE\xFF\xFF"
    return data[0:4] == PICO_MAGIC_BYTES


def parse_config(data: bytes) -> Config:
    '''Parse limited version of pico configuration.
    Picotool has the 'partition info' command so parsing that output might be simpler.

        00000000  d3 de ff ff 
               4  0a size_flag (0), item_type (PARTITION_TABLE)
               5   0e           size in words
               6   00
               7   02           partition count
               8  00 80 00 fc   unpartitioned_space_permissions_and_flags
               Partition #0
              0c  01 e0 1f fc   permissions_and_location
              10  01 10 06 fc   permissions_and_flags
              14  00 00 00 00   ID low
              18  00 00 00 00   ID high
              1c  04            name length
              1d   6f 74 61 30   ota0
              21   00 00 00
               Partition #1
              24  00 e1 3f fc   permissions_and_location
              28  01 10 06 fc   permissions_and_flags
              2C  01 00 00 00   ID low
              30  00 00 00 00   ID high
              34  04            name length
              35   6f 74 61 31   ota1
              39   00 00 00

        0000003C  48            ITEM_1BS_VERSION
                   02           size
                   00 00
              40  00 00         major version
                  01 00         minor version

        00000048  ff            ITEM_LAST
                   10 00        size
                   00
                  00 00 00 00   link
                  79 35 12 ab   Footer
    '''

    PICOBIN_BLOCK_ITEM_PARTITION_TABLE = 0x0a
    PICOBIN_BLOCK_ITEM_1BS_VERSION = 0x48
    PICOBIN_BLOCK_ITEM_2BS_LAST = 0xff

    cfg = Config()
    cfg.name = 'from Pico binary'
    cfg.arch = 'Rp2040'
    cfg.bootloader_size = 0
    cfg.partition_table_offset = 0

    # Skip header
    off = 4

    while off < len(data):
        start = off
        item_type = data[off]
        off += 1
        size = data[off] # In 4-byte words
        off += 1
        if item_type & 0x80:
            size |= data[off] << 8
        end = start + size * 4
        # print(f'item_type {item_type:02x}, size {size}, off {off}, len {len(data)}')
        # print(data[off:end].hex(sep=' '))
        off = end

        if item_type == PICOBIN_BLOCK_ITEM_PARTITION_TABLE:
            parse_partition_table(cfg, data[start:end])
        elif item_type == PICOBIN_BLOCK_ITEM_1BS_VERSION:
            pass
        elif item_type == PICOBIN_BLOCK_ITEM_2BS_LAST:
            break
        elif size == 0:
            break

    return cfg


'''
We need a way to store our ESP-style partition types.
Three choices come to mind:

1. Extra family IDs. We'd need to pick a range which is unlikely to cause future collisions.
2. Store mapping information outside the partition table in custom metadata block.
3. Incorporate into ID. ESP partitions are identified purely by name so this is easy enough.

(3) is the simplest, and easily managed using existing tooling.

The ID is 64 bits long. We need only 16 bits for the partition type.

Magic   2 bytes: Confirms this ID relates to Sming-style partitions
Type    2 bytes: Type and subtype bytes
User    4 bytes: User-defined ID

'''

SMING_ID_MAGIC = 0x6d73 # 'sm'

def parse_partition_table(cfg: Config, data: bytes):
    FLAG_HAS_ID = 0x00000001
    FLAG_HAS_NAME = 0x00001000
    FLAG_ACCEPTS_NUM_EXTRA_FAMILIES_LSB = 7
    FLAG_ACCEPTS_NUM_EXTRA_FAMILIES_BITS = 0x00000180
    FLASH_SECTOR_SIZE = 0x1000

    dev = partition.storage.Device('spiflash', partition.storage.TYPES['flash'], 0x400000)
    cfg.devices.append(dev)

    partition_count = data[3] & 0x0f
    unpartitioned_space_permissions_and_flags = struct.unpack('<L', data[4:8])
    off = 8
    for partition_index in range(partition_count):
        permissions_and_location, permissions_and_flags = struct.unpack('<LL', data[off:off+8])
        first_sector = permissions_and_location & 0x1fff
        last_sector = (permissions_and_location >> 13) & 0x1fff
        num_extra_families = (permissions_and_flags & FLAG_ACCEPTS_NUM_EXTRA_FAMILIES_BITS) >> FLAG_ACCEPTS_NUM_EXTRA_FAMILIES_LSB
        off += 8
        if permissions_and_flags & FLAG_HAS_ID:
            id, = struct.unpack('<Q', data[off:off+8])
            off += 8
        else:
            id = -1
        if num_extra_families:
            extra_families = struct.unpack(f'<{num_extra_families}L', data[off:off+num_extra_families*4])
            off += num_extra_families*4
        else:
            extra_families = []
        if permissions_and_flags & FLAG_HAS_NAME:
            name_len = data[off] & 0x7f
            off += 1
            name = data[off:off+name_len].decode()
            off = (off + name_len + 3) & 0xfffffffc
        else:
            name = None
        start_offset = first_sector * FLASH_SECTOR_SIZE
        end_offset = (last_sector + 1) * FLASH_SECTOR_SIZE
        extra_families_str = ', '.join(f'0x{a:x}' for a in extra_families)
        # print(f'{start_offset=:x}, {end_offset=:x}, {id=:x}, {name=}, {extra_families_str=}')

        if id & 0xffff != SMING_ID_MAGIC:
            print('** UNKNOWN **')
            continue

        e = partition.Entry()
        e.device = dev
        e.name = name
        e.type = (id >> 24) & 0xff
        e.subtype = (id >> 16) & 0xff
        e.address = start_offset
        e.size = end_offset - start_offset

        cfg.partitions.append(e)


def create_partition_table(config: Config) -> bytes:
    # Create intermediate JSON for picotool
    partlist = []
    for e in config.partitions:
        id = (e.type << 24) | (e.subtype << 16) | SMING_ID_MAGIC
        pt = {
            'name': e.name,
            'id': hex(id),
            'start': hex(e.address),
            'size': hex(e.size),
            'families': [],
            "permissions": {
                "secure": "rw",
                "nonsecure": "rw",
                "bootloader": "rw"
            }
        }

        if e.type == partition.APP_TYPE:
            pt['families'] = [ "rp2350-arm-s", "rp2350-riscv" ]
            if e.subtype == partition.SUBTYPES[e.type]['ota_1']:
                try:
                    e2 = next(config.partitions.find_by_type(partition.APP_TYPE, partition.SUBTYPES[e.type]['ota_0']))
                except StopIteration:
                    raise InputError('Missing ota_0 partition')
                pt['link'] = [ 'a', config.partitions.index(e2.name) ]
        else:
            pt['families'] = [ "data" ]

        partlist.append(pt)

    table = {
        'version': [1, 0],
        'unpartitioned': {
            'families': [ 'absolute' ],
            "permissions": {
                "secure": "rw",
                "nonsecure": "rw",
                "bootloader": "rw"
            }
        },
        "partitions": partlist
    }

    return json.dumps(table, indent=2).encode()


