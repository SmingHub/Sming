/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Device.cpp
 *
 ****/

#include "include/Storage.h"
#include "include/Storage/Device.h"
#include "include/Storage/partition_info.h"
#include <FlashString/Vector.hpp>
#include <Print.h>
#include <debug_progmem.h>

namespace
{
#define XX(type, value, desc) DEFINE_FSTR_LOCAL(typestr_##type, #type)
STORAGE_TYPE_MAP(XX)
#undef XX

#define XX(type, value, desc) &typestr_##type,
DEFINE_FSTR_VECTOR_LOCAL(typeStrings, FlashString, STORAGE_TYPE_MAP(XX))
#undef XX

#define XX(type, value, desc) DEFINE_FSTR_LOCAL(long_typestr_##type, desc)
STORAGE_TYPE_MAP(XX)
#undef XX

#define XX(type, value, desc) &long_typestr_##type,
DEFINE_FSTR_VECTOR_LOCAL(longTypeStrings, FlashString, STORAGE_TYPE_MAP(XX))
#undef XX

} // namespace

String toString(Storage::Device::Type type)
{
	return typeStrings[unsigned(type)];
}

String toLongString(Storage::Device::Type type)
{
	return longTypeStrings[unsigned(type)];
}

namespace Storage
{
Device::~Device()
{
	unRegisterDevice(this);
}

bool Device::loadPartitions(Device& source, uint32_t tableOffset)
{
	constexpr size_t maxEntries = ESP_PARTITION_TABLE_MAX_LEN / sizeof(esp_partition_info_t);
	auto buffer = std::make_unique<esp_partition_info_t[]>(maxEntries);
	if(!buffer) {
		return false;
	}
	if(!source.read(tableOffset, buffer.get(), ESP_PARTITION_TABLE_MAX_LEN)) {
		debug_e("[Partition] Failed to read partition table at offset 0x%08x", tableOffset);
		return false;
	}

	if(buffer[0].type != Partition::Type::storage) {
		debug_e("[Partition] Bad partition table for device '%s' @ 0x%08x", source.getName().c_str(), tableOffset);
		return false;
	}

	String devname = getName();
	for(unsigned i = 0; i < maxEntries; ++i) {
		auto entry = &buffer[i];
		if(entry->magic != ESP_PARTITION_MAGIC) {
			continue;
		}
		if(entry->type != Partition::Type::storage) {
			continue;
		}

		auto len = devname.length();
		if(len > Partition::nameSize) {
			continue;
		}
		if(strncmp(entry->name, devname.c_str(), len) != 0) {
			continue;
		}

		if(entry->subtype != uint8_t(getType())) {
			debug_w("[Device] '%s' type mismatch, '%s' in partition table but device reports '%s'", getName().c_str(),
					toString(Device::Type(entry->subtype)).c_str(), toString(getType()).c_str());
		}
		if(entry->size != getSize()) {
			debug_w("[Device] '%s' size mismatch, 0x%08x in partition table but device reports 0x%08llx",
					getName().c_str(), entry->size, uint64_t(getSize()));
		}

		// Skip the storage entry, not required
		++entry;
		++i;
		unsigned count{0};
		while(i < maxEntries && buffer[i].magic == ESP_PARTITION_MAGIC) {
			++i;
			++count;
		}

		mPartitions.load(entry, count);
		return true;
	}

	// No partitions found
	return false;
}

size_t Device::printTo(Print& p) const
{
	size_t n{0};
	n += p.print(getName());
	n += p.print(_F(": type "));
	n += p.print(getType());
	n += p.print(_F(", size 0x"));
	n += p.print(getSize(), HEX);
	return n;
}

} // namespace Storage
