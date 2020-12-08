/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DeviceManager.cpp
 *
 ****/

#include "include/Storage.h"
#include "include/Storage/SpiFlash.h"
#include <debug_progmem.h>

namespace Storage
{
void initialize()
{
	if(spiFlash == nullptr) {
		spiFlash = new SpiFlash;
		registerDevice(spiFlash);
		spiFlash->loadPartitions(PARTITION_TABLE_OFFSET);
	}
}

const Device::List getDevices()
{
	return Device::List(spiFlash);
}

bool registerDevice(Device* device)
{
	if(device == nullptr) {
		return false;
	}
	auto devname = device->getName();

	Device::List devices(spiFlash);
	auto it = std::find(devices.begin(), devices.end(), devname);
	if(!it) {
		devices.add(device);
		device->loadPartitions(*spiFlash, PARTITION_TABLE_OFFSET);
		debug_i("[Storage] Device '%s' registered", devname.c_str());
	} else if(*it != *device) {
		debug_e("[Storage] Another device is already registered with name '%s'", devname.c_str());
		return false;
	}

	return true;
}

bool unRegisterDevice(Device* device)
{
	return Device::List(spiFlash).remove(device);
}

Device* findDevice(const String& name)
{
	Device::List devices(spiFlash);
	return std::find(devices.begin(), devices.end(), name);
}

Partition findPartition(const String& name)
{
	for(auto& dev : getDevices()) {
		auto part = dev.partitions().find(name);
		if(part) {
			return part;
		}
	}

	return Partition{};
}

} // namespace Storage
