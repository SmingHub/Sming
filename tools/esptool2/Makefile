#
# Makefile for esptool2
# https://github.com/raburton/esp8266
#

CFLAGS = -O2 -Wall
CC = gcc
LD = gcc

all: esptool2

esptool2.o: esptool2.c esptool2.h esptool2_elf.h elf.h
	@echo "CC $<"
	$(CC) $(CFLAGS) -c $< -o $@

esptool2_elf.o: esptool2_elf.c esptool2.h esptool2_elf.h elf.h
	@echo "CC $<"
	$(CC) $(CFLAGS) -c $< -o $@

esptool2: esptool2.o esptool2_elf.o
	@echo "LD $@"
	$(LD) -o $@ $^

clean:
	@echo "RM *.o esptool2 esptool2.exe"
	@rm -f *.o
	@rm -f esptool2 esptool2.exe
