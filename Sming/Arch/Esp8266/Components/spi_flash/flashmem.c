/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * flashmem.c
 *
 * Based on NodeMCU platform_flash
 * https://github.com/nodemcu/nodemcu-firmware
 *
 * @author: 27/8/2018 - Mikee47 <mike@sillyhouse.net>
 *
 * Rewritten to deal with misaligned RAM buffers.
 *
 ****/

#include "include/esp_spi_flash.h"
#include "espinc/peri.h"

extern char _flash_code_end[];

/*
 * To ensure memory alignment a temporary buffer is used by flashmem_read and flashmem_write functions.
 *
 * The buffer must be an integer multiple of INTERNAL_FLASH_WRITE_UNIT_SIZE.
 */
#define FLASH_BUFFERS 32

/** @brief determines if the given value is aligned to a word (4-byte) boundary */
#undef IS_ALIGNED
#define IS_ALIGNED(x) (((uint32_t)(x)&0x00000003) == 0)

// Buffers need to be word aligned for flash access
#define __aligned __attribute__((aligned(4)))

// If this module were in C++ we could use std::min
static inline uint32_t min(uint32_t a, uint32_t b)
{
	return (a < b) ? a : b;
}

uint32_t flashmem_write(const void* from, uint32_t toaddr, uint32_t size)
{
	if(IS_ALIGNED(from) && IS_ALIGNED(toaddr) && IS_ALIGNED(size))
		return flashmem_write_internal(from, toaddr, size);

	const uint32_t blksize = INTERNAL_FLASH_WRITE_UNIT_SIZE;
	const uint32_t blkmask = INTERNAL_FLASH_WRITE_UNIT_SIZE - 1;

	__aligned char tmpdata[FLASH_BUFFERS * INTERNAL_FLASH_WRITE_UNIT_SIZE];
	const uint8_t* pfrom = (const uint8_t*)from;
	uint32_t remain = size;

	// Align the start
	if(toaddr & blkmask)
	{
		uint32_t rest = toaddr & blkmask;
		uint32_t addr_aligned = toaddr & ~blkmask; // this is the actual aligned address

		// Read existing unit and overlay with new data
		if(flashmem_read_internal(tmpdata, addr_aligned, blksize) != blksize)
			return 0;
		while(remain && rest < blksize)
		{
			tmpdata[rest++] = *pfrom++;
			--remain;
		}

		// Write the unit
		uint32_t written = flashmem_write_internal(tmpdata, addr_aligned, blksize);
		if(written != blksize)
			return written;

		if (remain == 0)
			return size;

		toaddr = addr_aligned + blksize;
	}

	// The start address is now a multiple of blksize
	// Compute how many bytes we can write as multiples of blksize
	uint32_t rest = remain & blkmask;
	remain &= ~blkmask;
	// Program the blocks now
	while(remain)
	{
		unsigned count = min(remain, sizeof(tmpdata));
		memcpy(tmpdata, pfrom, count);
		uint32_t written = flashmem_write_internal(tmpdata, toaddr, count);
		remain -= written;
		if(written != count)
			return size - remain;
		toaddr += count;
		pfrom += count;
	}

	// And the final part of a block if needed
	if(rest)
	{
		if(flashmem_read_internal(tmpdata, toaddr, blksize) != blksize)
			return size - remain;
		unsigned i;
		for(i = 0; i < rest; ++i)
			tmpdata[i] = *pfrom++;
		uint32_t written = flashmem_write_internal(tmpdata, toaddr, blksize);
		remain -= written;
		if(written != blksize)
			return size - remain;
	}

	return size;
}

uint32_t flashmem_read(void* to, uint32_t fromaddr, uint32_t size)
{
	if(IS_ALIGNED(to) && IS_ALIGNED(fromaddr) && IS_ALIGNED(size))
		return flashmem_read_internal(to, fromaddr, size);

	const uint32_t blksize = INTERNAL_FLASH_READ_UNIT_SIZE;
	const uint32_t blkmask = INTERNAL_FLASH_READ_UNIT_SIZE - 1;

	__aligned char tmpdata[FLASH_BUFFERS * INTERNAL_FLASH_READ_UNIT_SIZE];
	uint8_t* pto = (uint8_t*)to;
	uint32_t remain = size;

	// Align the start
	if(fromaddr & blkmask)
	{
		uint32_t rest = fromaddr & blkmask;
		uint32_t addr_aligned = fromaddr & ~blkmask; // this is the actual aligned address
		if (flashmem_read_internal(tmpdata, addr_aligned, blksize) != blksize)
			return 0;
		// memcpy(pto, &tmpdata[rest], std::min(blksize - rest, remain))
		while(remain && rest < blksize)
		{
			*pto++ = tmpdata[rest++];
			--remain;
		}
		if (remain == 0)
			return size;
		fromaddr = addr_aligned + blksize;
	}

	// The start address is now a multiple of blksize
	// Compute how many bytes we can read as multiples of blksize
	uint32_t rest = remain & blkmask;
	remain &= ~blkmask;
	// Read the blocks now
	while(remain)
	{
		unsigned count = min(remain, sizeof(tmpdata));
		uint32_t read = flashmem_read_internal(tmpdata, fromaddr, count);
		memcpy(pto, tmpdata, read);
		remain -= read;
		if(read != count)
			return size - remain;
		fromaddr += count;
		pto += count;
	}

	// And the final part of a block if needed
	if(rest)
	{
		if(flashmem_read_internal(tmpdata, fromaddr, blksize) != blksize)
			return size - remain;
		unsigned i;
		for(i = 0; i < rest; ++i)
			*pto++ = tmpdata[i];
	}

	return size;
}

bool flashmem_erase_sector(uint32_t sector_id)
{
	WDT_FEED();
	return spi_flash_erase_sector(sector_id) == SPI_FLASH_RESULT_OK;
}

SPIFlashInfo flashmem_get_info()
{
    volatile SPIFlashInfo spi_flash_info STORE_ATTR;
    spi_flash_info = *((SPIFlashInfo *)(INTERNAL_FLASH_START_ADDRESS));
    return spi_flash_info;
}

uint8_t flashmem_get_size_type()
{
    return flashmem_get_info().size;
}

uint32_t flashmem_get_size_bytes()
{
    uint32_t flash_size = 0;
    switch (flashmem_get_info().size)
    {
    case SIZE_2MBIT:
        // 2Mbit, 256kByte
        flash_size = 256 * 1024;
        break;
    case SIZE_4MBIT:
        // 4Mbit, 512kByte
        flash_size = 512 * 1024;
        break;
    case SIZE_8MBIT:
        // 8Mbit, 1MByte
        flash_size = 1 * 1024 * 1024;
        break;
    case SIZE_16MBIT:
        // 16Mbit, 2MByte
        flash_size = 2 * 1024 * 1024;
        break;
    case SIZE_32MBIT:
        // 32Mbit, 4MByte
        flash_size = 4 * 1024 * 1024;
        break;
    default:
        // Unknown flash size, fall back mode.
        flash_size = 512 * 1024;
        break;
    }
    return flash_size;
}

uint16_t flashmem_get_size_sectors()
{
    return flashmem_get_size_bytes() / SPI_FLASH_SEC_SIZE;
}

uint32_t flashmem_find_sector(uint32_t address, uint32_t *pstart, uint32_t *pend)
{
  // All the sectors in the flash have the same size, so just align the address
  uint32_t sect_id = address / INTERNAL_FLASH_SECTOR_SIZE;

  if( pstart )
    *pstart = sect_id * INTERNAL_FLASH_SECTOR_SIZE;
  if( pend )
    *pend = ( sect_id + 1 ) * INTERNAL_FLASH_SECTOR_SIZE - 1;
  return sect_id;
}

uint32_t flashmem_get_sector_of_address( uint32_t addr )
{
  return flashmem_find_sector( addr, NULL, NULL );
}

/////////////////////////////////////////////////////

uint32_t flashmem_write_internal( const void *from, uint32_t toaddr, uint32_t size )
{
  assert(IS_ALIGNED(from) && IS_ALIGNED(toaddr) && IS_ALIGNED(size));

  WDT_FEED();

  SpiFlashOpResult r = spi_flash_write(toaddr, (uint32*)from, size);
  if(SPI_FLASH_RESULT_OK == r)
    return size;
  else{
	SYSTEM_ERROR( "ERROR in flash_write: r=%d at %08X\n", ( int )r, ( unsigned )toaddr );
    return 0;
  }
}

uint32_t flashmem_read_internal( void *to, uint32_t fromaddr, uint32_t size )
{
  assert(IS_ALIGNED(to) && IS_ALIGNED(fromaddr) && IS_ALIGNED(size));

  WDT_FEED();

  SpiFlashOpResult r = spi_flash_read(fromaddr, (uint32*)to, size);
  if(SPI_FLASH_RESULT_OK == r)
    return size;
  else{
	SYSTEM_ERROR( "ERROR in flash_read: r=%d at %08X\n", ( int )r, ( unsigned )fromaddr );
    return 0;
  }
}

uint32_t flashmem_get_first_free_block_address()
{
  if(_flash_code_end == NULL)
  {
	  debugf("_flash_code_end is null");
	  return 0;
  }

  // Round the total used flash size to the closest flash block address
  uint32_t end;
  flashmem_find_sector( ( uint32_t )_flash_code_end - INTERNAL_FLASH_START_ADDRESS - 1, NULL, &end);
  return end + 1;
}
