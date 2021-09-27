/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Storage.h
 *
 ****/
#pragma once

#include "Storage/Device.h"

namespace Storage
{
/**
 * @brief Called early in the startup phase
 */
void initialize();

/**
 * @brief Get read-only reference to device list
 */
const Device::List getDevices();

/**
 * @brief Register a storage device
 * @retval bool true on success, false if another device already registered with same name
 */
bool registerDevice(Device* device);

/**
 * @brief Unregister a storage device
 *
 * Use extreme care: behaviour is unpredictable if partitions are in use
 */
bool unRegisterDevice(Device* device);

/**
 * @brief Find a registered device
 */
Device* findDevice(const String& name);

/**
 * @brief Find the first partition matching the given name
 */
Partition findPartition(const String& name);

/**
 * @brief Find partitions of the given type
 */
inline Iterator findPartition(Partition::Type type = Partition::Type::any, uint8_t subType = Partition::SubType::any)
{
	return Iterator(type, subType);
}

template <typename T> Iterator findPartition(T subType)
{
	return Iterator(Partition::Type(T::partitionType), uint8_t(subType));
}

template <typename T> Storage::Partition findDefaultPartition(T subType)
{
	auto part = *Storage::findPartition(subType);
	if(part) {
		debug_i("[%s] Found '%s'", part.typeString().c_str(), part.name().c_str());
	} else {
		debug_e("[%s] No partition found", toString(subType).c_str());
	}
	return part;
}

} // namespace Storage
