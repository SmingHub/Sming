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