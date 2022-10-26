/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Partition.cpp
 *
 ****/

#include "include/Storage/Partition.h"
#include "include/Storage/Device.h"
#include <FlashString/Map.hpp>
#include <Print.h>
#include <debug_progmem.h>

using namespace Storage;

namespace
{
/* APP type strings */

#define XX(subtype, value, desc) DEFINE_FSTR_LOCAL(app_subTypeStr_##subtype, #subtype)
PARTITION_APP_SUBTYPE_MAP(XX)
#undef XX

#define XX(subtype, value, desc) {Partition::SubType::App::subtype, &app_subTypeStr_##subtype},
DEFINE_FSTR_MAP_LOCAL(appSubTypeStrings, Partition::SubType::App, FlashString, PARTITION_APP_SUBTYPE_MAP(XX))
#undef XX

#define XX(subtype, value, desc) DEFINE_FSTR_LOCAL(app_subTypeStr_long_##subtype, desc)
PARTITION_APP_SUBTYPE_MAP(XX)
#undef XX

#define XX(subtype, value, desc) {Partition::SubType::App::subtype, &app_subTypeStr_long_##subtype},
DEFINE_FSTR_MAP_LOCAL(longAppSubTypeStrings, Partition::SubType::App, FlashString, PARTITION_APP_SUBTYPE_MAP(XX))
#undef XX

/* DATA subtype strings */

#define XX(subtype, value, desc) DEFINE_FSTR_LOCAL(data_subTypeStr_##subtype, #subtype)
PARTITION_DATA_SUBTYPE_MAP(XX)
#undef XX

#define XX(subtype, value, desc) {Partition::SubType::Data::subtype, &data_subTypeStr_##subtype},
DEFINE_FSTR_MAP_LOCAL(dataSubTypeStrings, Partition::SubType::Data, FlashString, PARTITION_DATA_SUBTYPE_MAP(XX))
#undef XX

#define XX(subtype, value, desc) DEFINE_FSTR_LOCAL(data_subTypeStr_long_##subtype, desc)
PARTITION_DATA_SUBTYPE_MAP(XX)
#undef XX

#define XX(subtype, value, desc) {Partition::SubType::Data::subtype, &data_subTypeStr_long_##subtype},
DEFINE_FSTR_MAP_LOCAL(longDataSubTypeStrings, Partition::SubType::Data, FlashString, PARTITION_DATA_SUBTYPE_MAP(XX))
#undef XX

} // namespace

String toString(Partition::Type type, uint8_t subType)
{
	String s;
	switch(type) {
	case Partition::Type::app:
		s = F("app/");
		if(auto v = appSubTypeStrings[Partition::SubType::App(subType)]) {
			s += String(v);
		} else {
			s += subType;
		}
		break;
	case Partition::Type::data:
		s = F("data/");
		if(auto v = dataSubTypeStrings[Partition::SubType::Data(subType)]) {
			s += String(v);
		} else {
			s += subType;
		}
		break;

	case Partition::Type::storage:
		s = F("storage/");
		if(auto v = toString(Device::Type(subType))) {
			s += v;
		} else {
			s += subType;
		}
		break;

	default:
		s = unsigned(type);
		s += '.';
		s += subType;
	}
	return s;
}

String toLongString(Partition::Type type, uint8_t subType)
{
	switch(type) {
	case Partition::Type::app:
		if(auto v = longAppSubTypeStrings[Partition::SubType::App(subType)]) {
			return F("App: ") + String(v);
		}
		break;
	case Partition::Type::data:
		if(auto v = longDataSubTypeStrings[Partition::SubType::Data(subType)]) {
			return F("Data: ") + String(v);
		}
		break;

	case Partition::Type::storage:
		if(auto s = toLongString(Device::Type(subType))) {
			return F("Storage: ") + s;
		}
		break;

	default:;
		// Unknown
	}

	return toString(type, subType);
}

namespace Storage
{
Partition::FullType::operator String() const
{
	return toString(type, subtype);
}

String Partition::typeString() const
{
	return toString(type(), subType());
}

String Partition::longTypeString() const
{
	return toLongString(type(), subType());
}

bool Partition::verify(Partition::Type type, uint8_t subtype) const
{
	if(mPart == nullptr) {
		debug_e("[Partition] invalid");
		return false;
	}

	if(type != mPart->type) {
		debug_e("[Partition] type mismatch, expected %u got %u", unsigned(type), unsigned(mPart->type));
		return false;
	}

	if(mPart->subtype != subtype) {
		debug_e("[Partition] subtype mismatch, expected %u got %u", subtype, mPart->subtype);
		return false;
	}

	return true;
}

bool Partition::getDeviceAddress(storage_size_t& address, storage_size_t size) const
{
	if(mDevice == nullptr || mPart == nullptr) {
		debug_e("[Partition] Invalid");
		return false;
	}

	if(address >= mPart->size || (address + size - 1) >= mPart->size) {
		debug_e("[Partition] Invalid range, address: 0x%08llx, size: 0x%08llx", uint64_t(address), uint64_t(size));
		return false;
	}

	// Storage partitions refer directly to the underlying device
	if(type() != Partition::Type::storage) {
		address += mPart->offset;
	}

	return true;
}

String Partition::getDeviceName() const
{
	return mDevice ? mDevice->getName() : nullptr;
}

size_t Partition::getBlockSize() const
{
	return mDevice ? mDevice->getBlockSize() : 0;
}

bool Partition::allowRead()
{
	if(mDevice == nullptr || mPart == nullptr) {
		debug_e("[Partition] Invalid");
		return false;
	}

	return true;
}

bool Partition::allowWrite()
{
	if(!allowRead()) {
		return false;
	}

	if(mPart->flags[Flag::readOnly]) {
		debug_e("[Partition] %s is read-only", mPart ? mPart->name.c_str() : "?");
		return false;
	}

	return true;
}

bool Partition::read(storage_size_t offset, void* dst, size_t size)
{
	if(!allowRead()) {
		return false;
	}

	auto addr = offset;
	if(!getDeviceAddress(addr, size)) {
		return false;
	}

	return mDevice->read(addr, dst, size);
}

bool Partition::write(storage_size_t offset, const void* src, size_t size)
{
	if(!allowWrite()) {
		return false;
	}

	auto addr = offset;
	if(!getDeviceAddress(addr, size)) {
		return false;
	}

	return mDevice->write(addr, src, size);
}

bool Partition::erase_range(storage_size_t offset, storage_size_t size)
{
	if(!allowWrite()) {
		return false;
	}

	auto addr = offset;
	if(!getDeviceAddress(addr, size)) {
		return false;
	}

	return mDevice->erase_range(addr, size);
}

size_t Partition::printTo(Print& p) const
{
	size_t n{0};
	if(*this) {
		n += p.print(getDeviceName());
		n += p.print('/');
		n += p.print(name());
		n += p.print(" (");
		n += p.print(typeString());
		n += p.print(" @ 0x");
		n += p.print(address(), HEX);
		n += p.print(_F(", size 0x"));
		n += p.print(size(), HEX);
		n += p.print(')');
	} else {
		n += p.print(_F("(none)"));
	}
	return n;
}

} // namespace Storage
