#!/usr/bin/env python3
#
# Virtual flash tool to support operations on flash backing file for Sming Host.
#
# Copyright 2021 mikee47 <mike@sillyhouse.net>
#
# This file is part of the Sming Framework Project
#
# This library is free software: you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation, version 3 or later.
#
# This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#

import os, argparse

# Interpret argument such as 0x1234
def auto_int(x):
    return int(x, 0)

def fill_region(offset, length, value):
    print("  fill_region(0x%08x, 0x%04x, 0x%02x)" % (offset, length, value))
    flashImage.seek(offset, os.SEEK_SET)
    tmp = bytearray([value] * length)
    flashImage.write(tmp)

def erase(args):
    fill_region(0, args.imagesize, 0xff)

def erase_region(args):
    fill_region(args.offset, args.size, 0xff)

# Offset,size
def erase_regions(args):
    for s in args.chunks:
        offset, size = s.split(',')
        fill_region(int(offset, 0), int(size, 0), 0xff)

# Offset,size=value
def fill_regions(args):
    for s in args.chunks:
        offset, tmp = s.split(',')
        x = tmp.split('=')
        if len(x) == 1:
            size = x[0]
            value = '0xff'
        else:
            size, value = x
        fill_region(int(offset, 0), int(size, 0), int(value,0))

def write_chunk(offset, filename):
    print("  write_chunk(0x%08x, '%s')" % (offset, filename))
    flashImage.seek(offset, os.SEEK_SET)
    data = open(filename, "rb").read()
    len = flashImage.write(data)
    print("    - wrote 0x%04x bytes" % len)

# Offset=filename
def write_chunks(args):
    for s in args.chunks:
        offset, file = s.split('=')
        write_chunk(int(offset, 0), file)


def read_chunk(offset, size, filename):
    print("  read_chunk(0x%08x, 0x%04x, '%s')" % (offset, size, filename))
    flashImage.seek(offset, os.SEEK_SET)
    data = flashImage.read(size)
    open(filename, "wb").write(data)

# Offset,size=filename
def read_chunks(args):
    for s in args.chunks:
        offset, x = s.split(',')
        size, filename = x.split('=')
        read_chunk(int(offset, 0), int(size, 0), filename)


if __name__ == "__main__":

    parser = argparse.ArgumentParser(description='Virtual flash tool')
    parser.add_argument('filename', help='Flash image filename')
    parser.add_argument('imagesize', type=auto_int, help='Flash image size')
    parser.set_defaults(func=None)
    subparsers = parser.add_subparsers()

    fill_regions_parser = subparsers.add_parser('fill-regions', help='Fill region(s) of flash')
    fill_regions_parser.add_argument('chunks', nargs='+', help='List of "offset,size[=value]" chunks, default value is 0xff')
    fill_regions_parser.set_defaults(func=fill_regions)

    erase_parser = subparsers.add_parser('erase', help='Erase entire flash')
    erase_parser.set_defaults(func=erase)

    write_chunks_parser = subparsers.add_parser('write-chunks', help='Write chunks to flash')
    write_chunks_parser.add_argument('chunks', nargs='+', help='List of "offset=file" chunks')
    write_chunks_parser.set_defaults(func=write_chunks)

    read_chunks_parser = subparsers.add_parser('read-chunks', help='Read chunks from flash')
    read_chunks_parser.add_argument('chunks', nargs='+', help='List of "offset,size=file" chunks')
    read_chunks_parser.set_defaults(func=read_chunks)

    args = parser.parse_args()

    flashImage = open(args.filename, "rb+" if os.path.isfile(args.filename) else "wb+")

    print("vflash('%s')" % args.filename)

    # Pad flash image file if it's not big enough
    flashImage.seek(0, os.SEEK_END)
    imgsize = flashImage.tell()
    if imgsize < args.imagesize:
        fill_region(imgsize, args.imagesize - imgsize, 0xff)

    # Invoke any user-provided functions (optional)
    fn = args.func
    if fn is None:
        parser.print_usage()
    else:
        fn(args)

    flashImage.close()
