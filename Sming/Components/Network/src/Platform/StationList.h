/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationList.h
 *
 ****/

#pragma once

#include <WString.h>
#include <IpAddress.h>
#include <MacAddress.h>
#include <Data/LinkedObjectList.h>

class StationInfo : public LinkedObjectTemplate<StationInfo>
{
public:
	virtual MacAddress mac() const = 0;

	/**
	 * @brief Current average rssi of connected station.
	 * Available only on some architectures
	 */
	virtual int8_t rssi() const = 0;

	/**
	 * @brief Assigned IP address.
	 * Available only on some architectures.
	 */
	virtual IpAddress ip() const = 0;
};

class StationList : public OwnedLinkedObjectListTemplate<StationInfo>
{
public:
	virtual ~StationList()
	{
	}
};

/** @} */
