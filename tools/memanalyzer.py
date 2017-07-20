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

TOTAL_DRAM = 32786;
TOTAL_IRAM = 81920;

sections = OrderedDict([
    ("data", "Initialized Data (RAM)"),
    ("rodata", "ReadOnly Data (RAM)"),
    ("bss", "Uninitialized Data (RAM)"),
    ("text", "Cached Code (IRAM)"),
    ("irom0_text", "Uncached Code (SPI)")
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

print("{0: >10}|{1: >30}|{2: >12}|{3: >12}|{4: >8}".format("Section", "Description", "Start (hex)", "End (hex)", "Used space"));
print("------------------------------------------------------------------------------");

usedRAM = 0;
usedIRAM = 0;

i = 0
for (idx, descr) in list(sections.items()):
    sectionStartToken = " _%s_start" %  idx
    sectionEndToken   = " _%s_end" % idx;
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
    if i < 3:
        usedRAM += sectionLength
    if i == 3:
        usedIRAM = TOTAL_DRAM - sectionLength;

    print("{0: >10}|{1: >30}|{2:12X}|{3:12X}|{4:8}".format(id, descr, sectionStart, sectionEnd, sectionLength))
    i += 1

print("Total Used RAM : %d" % usedRAM)
print("Free RAM : %d" % (TOTAL_IRAM - usedRAM))
print("Free IRam : %d" % usedIRAM)
