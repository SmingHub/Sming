/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationListImpl.h
 *
 ****/

#pragma once

#include <Platform/StationList.h>
#include <Data/Range.h>

class StationListImpl : public StationList
{
public:
	void reset() override
	{
		index = 0;
	}

	bool next() override
	{
		++index;
		return isValid();
	}

	MacAddress mac() const override
	{
		if(!isValid()) {
			return MacAddress{};
		}
		return MacAddress({5, 4, 3, 2, 1, index});
	}

	int8_t rssi() const override
	{
		if(!isValid()) {
			return 0;
		}
		return TRange<int8_t>(-90, -20).random();
	}

	IpAddress ip() const override
	{
		return IpAddress(10, 0, 0, 100 + index);
	}

private:
	static constexpr uint8_t entryCount{5};

	bool isValid() const
	{
		return index < entryCount;
	}

	uint8_t index{0};
};
