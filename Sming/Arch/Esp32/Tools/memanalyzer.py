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

# From soc/soc.h
SOC_DROM_LOW    = 0x3F400000
SOC_DROM_HIGH   = 0x3F800000
SOC_DRAM_LOW    = 0x3FFAE000
SOC_DRAM_HIGH   = 0x40000000
SOC_IROM_LOW    = 0x400D0000
SOC_IROM_HIGH   = 0x40400000
SOC_IRAM_LOW    = 0x40080000
SOC_IRAM_HIGH   = 0x400A0000

TOTAL_DRAM = SOC_DRAM_HIGH - SOC_DRAM_LOW;
TOTAL_IRAM = SOC_IRAM_HIGH - SOC_IRAM_LOW;

sections = OrderedDict([
    ("data", "Initialized Data (RAM)"),
    ("rodata", "ReadOnly Data (SPI)"),
    ("bss", "Uninitialized Data (RAM)"),
    ("iram_text", "Cached Code (IRAM)"),
    ("text", "Uncached Code (SPI)")
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
for (name, descr) in list(sections.items()):
    sectionStartToken = " _%s_start" %  name
    sectionEndToken   = " _%s_end" % name
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
    if i == 0 or i == 2:
        usedRAM += sectionLength
    if i == 3:
        usedIRAM = sectionLength;

    print("{0: >10}|{1: >30}|{2:12X}|{3:12X}|{4:8}".format(name, descr, sectionStart, sectionEnd, sectionLength))
    i += 1

print("Total Used RAM : %d" % usedRAM)
print("Free RAM : %d" % (TOTAL_DRAM - usedRAM))
print("Free IRam : %d" % (TOTAL_IRAM - usedIRAM))
