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
	Iterator(Device& device, uint8_t partitionIndex);

	Iterator(Device& device, Partition::Type type, uint8_t subtype) : mSearch{&device, type, subtype}
	{
		mDevice = &device;
		next();
	}

	Iterator(Partition::Type type, uint8_t subtype);

	explicit operator bool() const
	{
		return (mDevice != nullptr) && (mPos > beforeStart) && (mPos < afterEnd);
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
		return (mDevice == other.mDevice) && (mPos == other.mPos);
	}

	bool operator!=(const Iterator& other) const
	{
		return !operator==(other);
	}

	Partition operator*() const;

private:
	static constexpr int8_t beforeStart{-1};
	static constexpr int8_t afterEnd{0x7f};

	bool seek(uint8_t pos);
	bool next();

	struct Search {
		Device* device;
		Partition::Type type;
		uint8_t subType;
	};
	Search mSearch{};
	Device* mDevice{nullptr};
	int8_t mPos{beforeStart};
};

} // namespace Storage
