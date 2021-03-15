#!/usr/bin/env python
########################################################
#
#  Simple PHY binary patcher
#  Author: Slavey Karadzhov <slav@attachix.com>
#
########################################################
import os
import sys

def usage():
    print("Usage: \n\t%s <file.bin> [offset]" % sys.argv[0])

if __name__ == "__main__":
    if len(sys.argv)  not in list(range(2,4)):
        usage()
        sys.exit(1)

    offset = 107
    if len(sys.argv) > 2:
        offset = int(sys.argv[2])

    with os.fdopen(os.open(sys.argv[1], os.O_RDWR | os.O_CREAT), 'rb+') as f:
        f.seek(offset)
        f.write(bytearray(b'\xff'));
