/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Iterator.h
 *
 ****/
#pragma once

#include "Partition.h"

namespace Storage
{
class Device;

class Iterator : public std::iterator<std::forward_iterator_tag, Partition>
{
public:
	Iterator(Device& device) : mSearch{&device, Partition::Type::any, Partition::SubType::any}, mDevice(&device)
	{
		next();
	}

	Iterator(Device& device, Partition::Type type, uint8_t subtype) : mSearch{&device, type, subtype}, mDevice(&device)
	{
		next();
	}

	Iterator(Partition::Type type, uint8_t subtype);

	explicit operator bool() const
	{
		return mDevice && mInfo;
	}

	Iterator operator++(int)
	{
		auto result = *this;
		next();
		return result;
	}

	Iterator& operator++()
	{
		next();
		return *this;
	}

	bool operator==(const Iterator& other) const
	{
		return mInfo == other.mInfo;
	}

	bool operator!=(const Iterator& other) const
	{
		return !operator==(other);
	}

	Partition operator*() const
	{
		return mDevice && mInfo ? Partition(*mDevice, *mInfo) : Partition{};
	}

	Iterator begin()
	{
		return mSearch.device ? Iterator(*mSearch.device) : Iterator(mSearch.type, mSearch.subType);
	}

	Iterator end()
	{
		return Iterator();
	}

private:
	Iterator()
	{
	}

	void next();

	struct Search {
		Device* device;
		Partition::Type type;
		uint8_t subType;
	};
	Search mSearch{};
	Device* mDevice{nullptr};
	const Partition::Info* mInfo{nullptr};
};

} // namespace Storage
