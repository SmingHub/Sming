/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Query.h
 *
 ****/

#pragma once

#include "Resource.h"
#include <Data/LinkedObjectList.h>

namespace mDNS
{
/**
 * @brief A single mDNS question
 */
class Query : public LinkedObjectTemplate<Query>
{
public:
	using List = LinkedObjectListTemplate<Query>;
	using OwnedList = OwnedLinkedObjectListTemplate<Query>;

	Query(const String& name, ResourceType type = ResourceType::PTR) : name(name), type(type)
	{
	}

	uint16_t getQClass() const
	{
		return qclass & 0x7fff;
	}

	void setQClass(uint16_t value)
	{
		qclass = (value & 0x7fff) | (qclass & 0x8000);
	}

	bool isUnicastResponse() const
	{
		return qclass & 0x8000;
	}

	void setUnicastResponse(bool state)
	{
		if(state) {
			qclass |= 0x8000;
		} else {
			qclass &= 0x7fff;
		}
	}

	bool operator==(const Query& other)
	{
		return name == other.name && type == other.type && qclass == other.qclass;
	}

	uint16_t serialize(uint8_t* buffer) const;

private:
	String name;						  ///< Contains the object, domain or zone name
	ResourceType type{ResourceType::PTR}; ///< Type of question being asked by client
	uint16_t qclass{0x0001};			  ///< Normally the value 1 for Internet ('IN')
};

size_t serialize(const Query::List& list, uint8_t* buffer, size_t bufSize);

} // namespace mDNS
