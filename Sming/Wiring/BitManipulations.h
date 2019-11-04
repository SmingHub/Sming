/*
 * BitManipulations.h
 *
 *  Created on: 03 ����. 2015 �.
 *      Author: Anakonda
 */

#pragma once

#define 	_BV(bit)   (1 << (unsigned)(bit))

#define bit(x) _BV(x)
#define setBits(x, y) ((x) |= (y))
#define clearBits(x, y) ((x) &= (~(y)))
#define bitsSet(x, y) (((x) & (y)) == (y))
#define bitsClear(x, y) (((x) & (y)) == 0)

#define bitRead(value, bit) (((value) >> (unsigned)(bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (unsigned)(bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (unsigned)(bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define _SFR_MEM_ADDR(sfr) ((uint32_t) &(sfr)) // 32 Bit!
#define _SFR_ADDR(sfr) _SFR_MEM_ADDR(sfr)

#define _MMIO_BYTE(mem_addr) (*(volatile uint8_t *)(mem_addr))
#define _MMIO_WORD(mem_addr) (*(volatile uint16_t *)(mem_addr))
#define _MMIO_DWORD(mem_addr) (*(volatile uint32_t *)(mem_addr))

#define _SFR_BYTE(sfr) _MMIO_BYTE(_SFR_ADDR(sfr))
#define _SFR_WORD(sfr) _MMIO_WORD(_SFR_ADDR(sfr))
#define _SFR_DWORD(sfr) _MMIO_DWORD(_SFR_ADDR(sfr))

#define _SFR_MEM8(mem_addr)  _MMIO_BYTE(mem_addr)
#define _SFR_MEM16(mem_addr) _MMIO_WORD(mem_addr)
#define _SFR_MEM32(mem_addr) _MMIO_DWORD(mem_addr)

#define 	bit_is_set(sfr, bit)   (_SFR_BYTE(sfr) & _BV(bit))
#define 	bit_is_clear(sfr, bit)   (!(_SFR_BYTE(sfr) & _BV(bit)))
#define 	loop_until_bit_is_set(sfr, bit)   do { } while (bit_is_clear(sfr, bit))
#define 	loop_until_bit_is_clear(sfr, bit)   do { } while (bit_is_set(sfr, bit))
