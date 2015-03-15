/*
 * BitManipulations.h
 *
 *  Created on: 03 февр. 2015 г.
 *      Author: Anakonda
 */

#ifndef WIRING_BITMANIPULATIONS_H_
#define WIRING_BITMANIPULATIONS_H_

#define 	_BV(bit)   (1 << (bit))

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



#endif /* WIRING_BITMANIPULATIONS_H_ */
