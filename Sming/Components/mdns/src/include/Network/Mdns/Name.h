/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Name.h
 *
 ****/

#pragma once

#include <WString.h>

namespace mDNS
{
class Response;
struct Packet;

/**
 * @brief Encoded DNS name
 */
class Name
{
public:
	Name(Response& response, uint8_t* data) : response(response), data(data)
	{
	}

	uint16_t getDataLength() const;

	String toString() const;

	operator String() const
	{
		return toString();
	}

private:
	Response& response;
	uint8_t* data;
};

} // namespace mDNS