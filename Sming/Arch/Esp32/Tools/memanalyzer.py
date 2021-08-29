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

# esptool defines a load of useful stuff
sys.path.insert(1, os.path.expandvars('${SMING_HOME}/Components/esptool/esptool'))
import esptool

# Get expected memory sizes from loader tables
loader = esptool._chip_to_rom_loader(os.environ['ESP_VARIANT'])
TOTAL_DRAM = sum(end - start for (start, end, n) in loader.MEMORY_MAP if n in ['DRAM'])
TOTAL_IRAM = sum(end - start for (start, end, n) in loader.MEMORY_MAP if n in ['IRAM'])

# TESTING
def printMemoryMaps():
    loaders = OrderedDict({
            'esp8266': esptool.ESP8266ROM,
            'esp32': esptool.ESP32ROM,
            'esp32s2': esptool.ESP32S2ROM,
            'esp32s3beta2': esptool.ESP32S3BETA2ROM,
            'esp32s3': esptool.ESP32S3ROM,
            'esp32c3': esptool.ESP32C3ROM,
            'esp32c6beta': esptool.ESP32C6BETAROM,
            'esp32h2': esptool.ESP32H2ROM
    })
    for k, v in loaders.items():
        print(k)
        for (start, end, n) in v.MEMORY_MAP:
            print("  %16s: %8u (0x%08x - 0x%08x)" % (n, end - start, start, end))
        dram = sum(end - start for (start, end, n) in v.MEMORY_MAP if n in ['DRAM'])
        iram = sum(end - start for (start, end, n) in v.MEMORY_MAP if n in ['IRAM'])
        print("  %16s: %8u" % ('TOTAL_DRAM', dram))
        print("  %16s: %8u" % ('TOTAL_IRAM', iram))
        print("  %16s: %8u" % ('DRAM+IRAM', dram + iram))

# printMemoryMaps()

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
