#!/bin/python
########################################################
#
#  Memory Analyzer
#  Author: Slavey Karadzhov <slav@attachix.com>
#  Based on https://github.com/Sermus/ESP8266_memory_analyzer
#
########################################################
from collections import OrderedDict
import os.path
import shlex
import subprocess
import sys

TOTAL_RAM = 264 * 1024

sections = OrderedDict([
    ("data", "Initialized Data (RAM)"),
    ("bss", "Uninitialized Data (RAM)"),
    ("flash_binary", "Uncached Code (SPI)")
])

if len(sys.argv) < 2:
    print("Usage: \n\t%s%s <path_to_objdump> <path_to_app_out>" % sys.argv[0])
    sys.exit(1)

objectDumpBin = sys.argv[1]
appOut        = sys.argv[2]

if not os.path.exists(appOut):
    print("Cannot find application out file: %s" % appOut)
    sys.exit(1)


command = "%s -t '%s' " % (sys.argv[1], sys.argv[2])
response = subprocess.check_output(shlex.split(command))
if isinstance(response, bytes):
    response = response.decode('utf-8')
lines = response.split('\n')

print("{0: >12}|{1: >30}|{2: >12}|{3: >12}|{4: >8}".format("Section", "Description", "Start (hex)", "End (hex)", "Used space"));
print("------------------------------------------------------------------------------");

usedRAM = 0

i = 0
for (name, descr) in list(sections.items()):
    sectionStartToken = " __%s_start" %  name
    sectionEndToken   = " __%s_end" % name
    sectionStart = -1;
    sectionEnd = -1;
    for line in lines:
        if sectionStartToken in line:
            data = line.split(' ')
            sectionStart = int(data[0], 16)

        if sectionEndToken in line:
            data = line.split(' ')
            sectionEnd = int(data[0], 16)

        if sectionStart != -1 and sectionEnd != -1:
            break

    sectionLength = sectionEnd - sectionStart
    if i < 2:
        usedRAM += sectionLength

    print("{0: >12}|{1: >30}|{2:12X}|{3:12X}|{4:8}".format(name, descr, sectionStart, sectionEnd, sectionLength))
    i += 1

print("Total Used RAM : %d" % usedRAM)
print("Free RAM : %d" % (TOTAL_RAM - usedRAM))
