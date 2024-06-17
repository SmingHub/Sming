/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * flashmem.cpp
 *
 ****/

#include <esp_spi_flash.h>
#include <hardware/flash.h>
#include <hardware/dma.h>
#include <hardware/regs/addressmap.h>
#include <hardware/structs/xip_ctrl.h>
#include <hardware/structs/ssi.h>
#include <hardware/regs/ssi.h>
#include <debug_progmem.h>

#define FLASHCMD_READ_SFDP 0x5a
#define FLASHCMD_READ_JEDEC_ID 0x9f

// Buffers need to be word aligned for flash access
#define ATTR_ALIGNED __attribute__((aligned(4)))

namespace
{
/*
 * To ensure memory alignment a temporary buffer is used by flashmem_read and flashmem_write functions.
 *
 * The buffer must be an integer multiple of INTERNAL_FLASH_WRITE_UNIT_SIZE.
 */
constexpr dma_channel_transfer_size dmaTransferSize{DMA_SIZE_32};
constexpr size_t flashReadUnitSize{1 << dmaTransferSize};
constexpr size_t flashBufferCount{FLASH_PAGE_SIZE / flashReadUnitSize};

// JEDEC flash ID read from chip
uint32_t flash_id;
// Size of flash chip read from device
uint32_t sfdp_flash_size_bytes;

uint32_t sfdp_read_size()
{
	uint32_t hdr[4];
	flashmem_sfdp_read(0, hdr, sizeof(hdr));

	// Check magic
	constexpr uint32_t sfdp_magic{'S' | ('F' << 8) | ('D' << 16) | ('P' << 24)};
	if(sfdp_magic != hdr[0]) {
		debug_e("[SFDP] Magic invalid: 0x%08x (expected 0x%08x)", hdr[0], sfdp_magic);
		return 0;
	}

	// Skip NPH -- we don't care about nonmandatory parameters.
	// Check header byte for mandatory parameter table
	// | ID | MinRev | MajRev | Length in words | ptr[2] | ptr[1] | ptr[0] | unused|
	// ID must be 0 (JEDEC) for mandatory PTH
	if((hdr[2] & 0xff) != 0) {
		debug_e("[SFDP] PTH ID invalid");
		return 0;
	}

	uint32_t param_table_ptr = hdr[3] & 0xffffffu;
	uint32_t param[2];
	flashmem_sfdp_read(param_table_ptr, &param, sizeof(param));

	// MSB set: array >= 2 Gbit, encoded as log2 of number of bits
	auto array_size_word = param[1];
	if(array_size_word & BIT(31)) {
		return 1UL << ((array_size_word & ~BIT(31)) - 3);
	}
	// MSB clear: array < 2 Gbit, encoded as direct bit count, minus 1
	return (array_size_word + 1) / 8;
}

void initFlashInfo()
{
	if(flash_id != 0) {
		return;
	}

	// Read JEDEC ID: command, 3 data bytes (manf. ID, Memory Type ID, Capacity ID)
	uint8_t buf[4] = {FLASHCMD_READ_JEDEC_ID};
	flash_do_cmd(buf, buf, 4);
	flash_id = (buf[1] << 16) | (buf[2] << 8) | buf[3];

	sfdp_flash_size_bytes = sfdp_read_size();
}

uint32_t writeAligned(const void* from, uint32_t toaddr, uint32_t size)
{
	auto flashaddr = XIP_BASE + toaddr;
	if(!isFlashPtr(flashaddr)) {
		debug_e("[FLSH] write toaddr not in flash 0x%08x", toaddr);
		return 0;
	}

	debug_d("[FLSH] write(%p, 0x%08x, 0x%08x)", from, toaddr, size);

	flash_range_program(toaddr, static_cast<const uint8_t*>(from), size);

	return size;
}

uint32_t readAligned(void* to, uint32_t fromaddr, uint32_t size)
{
	auto flashaddr = XIP_BASE + fromaddr;
	if(!isFlashPtr(flashaddr)) {
		debug_e("[FLSH] read fromaddr not in flash 0x%08x", fromaddr);
		return 0;
	}

	debug_d("[FLSH] read(%p, 0x%08x, 0x%08x)", to, fromaddr, size);

	auto transfer_count = size >> dmaTransferSize;

	/*
	 * https://github.com/raspberrypi/pico-examples/tree/master/flash/xip_stream
	 *
	 * The XIP has some internal hardware that can stream a linear access sequence to
	 * a DMAable FIFO, while the system is still doing random accesses on flash code + data.
	 */

	/*
	 * Transfer started by writing nonzero value to stream_ctr.
	 * stream_ctr will count down as the transfer progresses.
	 * Can terminate early by writing 0 to stream_ctr.
	 * It's a good idea to drain the FIFO first!
	*/
	while(!(xip_ctrl_hw->stat & XIP_STAT_FIFO_EMPTY)) {
		(void)xip_ctrl_hw->stream_fifo;
	}

	xip_ctrl_hw->stream_addr = XIP_NOCACHE_NOALLOC_BASE + fromaddr;
	xip_ctrl_hw->stream_ctr = transfer_count;

	/*
	 * Start DMA transfer from XIP stream FIFO to our buffer in memory.
	 * Use the auxiliary bus slave for the DMA<-FIFO accesses, to avoid stalling
	 * the DMA against general XIP traffic.
	 */
	auto dma_chan = dma_claim_unused_channel(true);
	dma_channel_config cfg = dma_channel_get_default_config(dma_chan);
	channel_config_set_transfer_data_size(&cfg, dmaTransferSize);
	channel_config_set_read_increment(&cfg, false);
	channel_config_set_write_increment(&cfg, true);
	channel_config_set_dreq(&cfg, DREQ_XIP_STREAM);
	dma_channel_configure(dma_chan, &cfg,
						  to,										   // Write addr
						  reinterpret_cast<const void*>(XIP_AUX_BASE), // Read addr
						  transfer_count,							   // Transfer count
						  true										   // Start immediately!
	);

	dma_channel_wait_for_finish_blocking(dma_chan);
	dma_channel_unclaim(dma_chan);

	return size;
}

} // namespace

uint32_t flashmem_write(const void* from, uint32_t toaddr, uint32_t size)
{
	const uint32_t blksize = FLASH_PAGE_SIZE;
	const uint32_t blkmask = FLASH_PAGE_SIZE - 1;

	if((toaddr & blkmask) == 0 && (size & blkmask) == 0) {
		return writeAligned(from, toaddr, size);
	}

	uint8_t tmpdata[FLASH_PAGE_SIZE];
	auto pfrom = static_cast<const uint8_t*>(from);
	uint32_t remain = size;

	// Align the start
	uint32_t rest = toaddr & blkmask;
	if(rest != 0) {
		uint32_t addr_aligned = toaddr & ~blkmask; // this is the actual aligned address

		// Read existing unit and overlay with new data
		if(readAligned(tmpdata, addr_aligned, blksize) != blksize) {
			return 0;
		}

		while(remain != 0 && rest < blksize) {
			tmpdata[rest++] = *pfrom++;
			--remain;
		}

		// Write the unit
		uint32_t written = writeAligned(tmpdata, addr_aligned, blksize);
		if(written != blksize) {
			return written;
		}

		if(remain == 0) {
			return size;
		}

		toaddr = addr_aligned + blksize;
	}

	// The start address is now a multiple of blksize
	// Compute how many bytes we can write as multiples of blksize
	rest = remain & blkmask;
	remain &= ~blkmask;
	// Program the blocks now
	while(remain != 0) {
		unsigned count = std::min(size_t(remain), sizeof(tmpdata));
		memcpy(tmpdata, pfrom, count);
		uint32_t written = writeAligned(tmpdata, toaddr, count);
		remain -= written;
		if(written != count) {
			return size - remain;
		}
		toaddr += count;
		pfrom += count;
	}

	// And the final part of a block if needed
	if(rest != 0) {
		if(readAligned(tmpdata, toaddr, blksize) != blksize) {
			return size - remain;
		}
		for(unsigned i = 0; i < rest; ++i) {
			tmpdata[i] = *pfrom++;
		}
		uint32_t written = writeAligned(tmpdata, toaddr, blksize);
		remain -= written;
		if(written != blksize) {
			return size - remain;
		}
	}

	return size;
}

uint32_t flashmem_read(void* to, uint32_t fromaddr, uint32_t size)
{
	if(IS_ALIGNED(to) && IS_ALIGNED(fromaddr) && IS_ALIGNED(size)) {
		return readAligned(to, fromaddr, size);
	}

	const uint32_t blksize = flashReadUnitSize;
	const uint32_t blkmask = flashReadUnitSize - 1;

	ATTR_ALIGNED uint8_t tmpdata[flashBufferCount * blksize];
	auto pto = static_cast<uint8_t*>(to);
	size_t remain = size;

	// Align the start
	uint32_t rest = fromaddr & blkmask;
	if(rest != 0) {
		uint32_t addr_aligned = fromaddr & ~blkmask; // this is the actual aligned address
		if(readAligned(tmpdata, addr_aligned, blksize) != blksize) {
			return 0;
		}
		// memcpy(pto, &tmpdata[rest], std::min(blksize - rest, remain))
		while(remain != 0 && rest < blksize) {
			*pto++ = tmpdata[rest++];
			--remain;
		}
		if(remain == 0) {
			return size;
		}
		fromaddr = addr_aligned + blksize;
	}

	// The start address is now a multiple of blksize
	// Compute how many bytes we can read as multiples of blksize
	rest = remain & blkmask;
	remain &= ~blkmask;
	// Read the blocks now
	while(remain != 0) {
		unsigned count = std::min(remain, sizeof(tmpdata));
		uint32_t read = readAligned(tmpdata, fromaddr, count);
		memcpy(pto, tmpdata, read);
		remain -= read;
		if(read != count) {
			return size - remain;
		}
		fromaddr += count;
		pto += count;
	}

	// And the final part of a block if needed
	if(rest != 0) {
		if(readAligned(tmpdata, fromaddr, blksize) != blksize) {
			return size - remain;
		}
		for(unsigned i = 0; i < rest; ++i) {
			*pto++ = tmpdata[i];
		}
	}

	return size;
}

bool flashmem_erase_sector(uint32_t sector_id)
{
	debug_d("flashmem_erase_sector(0x%08x)", sector_id);
	system_soft_wdt_feed();
	flash_range_erase(sector_id * FLASH_SECTOR_SIZE, FLASH_SECTOR_SIZE);
	return true;
}

SPIFlashInfo flashmem_get_info()
{
	SPIFlashInfo info{};
	info.size = flashmem_get_size_type();

	// Flash mode
	uint32_t ctrlr0 = ssi_hw->ctrlr0;
	auto ssi_frame_format = (ctrlr0 & SSI_CTRLR0_SPI_FRF_BITS) >> SSI_CTRLR0_SPI_FRF_LSB;
	auto trans_type = (ctrlr0 & SSI_SPI_CTRLR0_TRANS_TYPE_BITS) >> SSI_SPI_CTRLR0_TRANS_TYPE_LSB;

	switch(ssi_frame_format) {
	case SSI_CTRLR0_SPI_FRF_VALUE_DUAL:
		info.mode = (trans_type == 0) ? MODE_DOUT : MODE_DIO;
		break;
	case SSI_CTRLR0_SPI_FRF_VALUE_QUAD:
		info.mode = (trans_type == 0) ? MODE_QOUT : MODE_QIO;
		break;
	case SSI_CTRLR0_SPI_FRF_VALUE_STD:
	default:
		info.mode = MODE_SLOW_READ;
	}

	return info;
}

SPIFlashSize flashmem_get_size_type()
{
	initFlashInfo();

	switch(flashmem_get_size_bytes()) {
	case 0x40000:
		return SIZE_2MBIT;
	case 0x80000:
		return SIZE_4MBIT;
	case 0x100000:
		return SIZE_8MBIT;
	case 0x200000:
		return SIZE_16MBIT;
	case 0x400000:
		return SIZE_32MBIT;
	default:
		return SIZE_1MBIT;
	}
}

uint32_t flashmem_get_size_bytes()
{
	initFlashInfo();

	// Use SFDP data if available
	if(sfdp_flash_size_bytes != 0) {
		return sfdp_flash_size_bytes;
	}

	/*
	* Determine flash size from the JEDEC device ID.
	* The lower byte generally contains the device size in bytes as a power of 2.
	* However, this isn't a hard standard so may not work in all situations.
	*/
	auto size_bytes = flash_id & 0xff;
	return 1U << size_bytes;
}

uint32_t spi_flash_get_id(void)
{
	initFlashInfo();
	return flash_id;
}

uint32_t flashmem_get_address(const void* memptr)
{
	auto addr = uint32_t(memptr);
	if(addr < XIP_BASE || addr >= XIP_NOALLOC_BASE) {
		return 0;
	}
	return addr - XIP_BASE;
}

void flashmem_sfdp_read(uint32_t addr, void* buffer, size_t count)
{
	size_t buflen = 5 + count;
	uint8_t buf[buflen] = {
		FLASHCMD_READ_SFDP,
		uint8_t(addr >> 16),
		uint8_t(addr >> 8),
		uint8_t(addr & 0xff),
		0, // dummy
	};

	flash_do_cmd(buf, buf, buflen);
	memcpy(buffer, &buf[5], count);
}
