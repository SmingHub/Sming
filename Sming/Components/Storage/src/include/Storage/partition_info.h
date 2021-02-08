/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * partition_info.h
 *
 ****/

#include "Partition.h"

namespace Storage
{
/**
 * @brief Internal structure describing the binary layout of a partition table entry.
 */
struct esp_partition_info_t {
	uint16_t magic;					 ///< Fixed value to identify valid entry, appears as 0xFFFF at end of table
	Partition::Type type;			 ///< Main type of partition
	uint8_t subtype;				 ///< Sub-type for partition (interpretation dependent upon type)
	uint32_t offset;				 ///< Start offset
	uint32_t size;					 ///< Size of partition in bytes
	Storage::Partition::Name name;   ///< Unique identifer for entry
	Storage::Partition::Flags flags; ///< Various option flags
};

constexpr uint16_t ESP_PARTITION_MAGIC{0x50AA};		 ///< Identifies a valid partition
constexpr uint16_t ESP_PARTITION_MAGIC_MD5{0xEBEB};  ///< Identifies an MD5 hash block
constexpr size_t ESP_PARTITION_TABLE_MAX_LEN{0xC00}; // Maximum length of partition table data

} // namespace Storage
