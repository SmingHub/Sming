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
 * 
 * mDNS-SD names are represented as instance.service.domain
 */
class Name
{
public:
	// The mDNS spec says this should never be more than 256 (including trailing '\0').
	static constexpr size_t maxLength{256};

	Name(Response& response, uint16_t ptr) : response(response), ptr(ptr)
	{
	}

	/**
	 * @brief Get number of bytes occupied by the name
	 * Not the same as the string length because content is encoded.
	 */
	uint16_t getDataLength() const
	{
		return parse().dataLength;
	}

	String toString() const
	{
		return getString(0, 255);
	}

	operator String() const
	{
		return toString();
	}

	String getDomain() const;
	String getService() const;
	String getInstance() const;

private:
	struct Info {
		uint16_t dataLength;
		uint16_t textLength;
		uint8_t components;
	};

	Info parse() const;
	uint16_t read(char* buffer, uint16_t bufSize, uint8_t firstElement, uint8_t count) const;
	String getString(uint8_t firstElement, uint8_t count) const;

	Response& response;
	uint16_t ptr;
};

} // namespace mDNS
