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

class StationList
{
public:
	virtual ~StationList()
	{
	}

	/**
	 * @brief Reset to start of list
	 */
	virtual void reset() = 0;

	/**
	 * @brief Query next entry
	 * @retval bool true on success, false if there are no more entries
	 */
	virtual bool next() = 0;

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

/** @} */
