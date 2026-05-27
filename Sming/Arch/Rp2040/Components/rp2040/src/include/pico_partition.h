/****
 * @brief Support for RP2350 partitions
 */

#ifndef SOC_RP2040

#include <cstdint>
#include <esp_spi_flash.h>
#include <WString.h>

namespace Pico
{
struct PartitionHeader {
	uint32_t first_sector : 13;
	uint32_t last_sector : 13;
	uint32_t permission_s_r : 1;
	uint32_t permission_s_w : 1;
	uint32_t permission_ns_r : 1;
	uint32_t permission_ns_w : 1;
	uint32_t permission_nsboot_r : 1;
	uint32_t permission_nsboot_w : 1;

	uint32_t has_id : 1;									  // 0
	uint32_t link_type : 2;									  // 1, 2
	uint32_t link_value : 4;								  // 3 - 6
	uint32_t accepts_num_extra_families : 2;				  // 7, 8
	uint32_t not_bootable_arm : 1;							  // 9
	uint32_t not_bootable_riscv : 1;						  // 10
	uint32_t uf2_download_ab_non_bootable_owner_affinity : 1; // 11
	uint32_t has_name : 1;									  // 12
	uint32_t uf2_download_no_reboot : 1;					  // 13
	uint32_t accepts_default_family_rp2040 : 1;				  // 14
	uint32_t accepts_default_family_absolute : 1;			  // 15
	uint32_t accepts_default_family_data : 1;				  // 16
	uint32_t accepts_default_family_rp2350_arm_s : 1;		  // 17
	uint32_t accepts_default_family_rp2350_riscv : 1;		  // 18
	uint32_t accepts_default_family_rp2350_arm_ns : 1;		  // 19

	uint32_t startOffset() const
	{
		return first_sector * INTERNAL_FLASH_SECTOR_SIZE;
	}

	uint32_t endOffset() const
	{
		return (last_sector + 1) * INTERNAL_FLASH_SECTOR_SIZE;
	}

	uint32_t size() const
	{
		return endOffset() - startOffset();
	}

	// If permissions do not include write access (other than from bootloader)
	bool isReadOnly() const
	{
		return !(permission_s_w || permission_ns_w);
	}

	// If permissions indicate partition access only via secure mode
	bool isEncrypted() const
	{
		return !permission_ns_r;
	}
};

struct PartitionTableInfo {
	uint32_t fields;
	uint32 partition_count : 8;
	uint32_t table_present : 1;
	PartitionHeader unpartitioned_space;
};

union PartitionID {
	struct {
		uint32_t low;
		uint32_t high;
	};

	struct {
		uint16_t tag;
		uint8_t subtype;
		uint8_t type;
	};

	operator uint64_t() const
	{
		return (uint64_t(high) << 32) | low;
	}
};

struct PartitionInfo {
	uint32_t fields;
	PartitionHeader header;
	PartitionID id;
	char name_buffer[128]; // name length is indicated by 7 bits
	uint32_t extra_family_ids[3];

	uint8_t nameLength() const
	{
		return name_buffer[0];
	}

	String name() const
	{
		return String(&name_buffer[1], nameLength());
	}
};

constexpr uint16_t SMING_TAG = 0x6d73; //  'sm'

bool getPartitionTableInfo(PartitionTableInfo& info);
bool getPartitionInfo(PartitionInfo& info, uint8_t partIndex);

} // namespace Pico

#endif
