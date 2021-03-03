/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Question.h
 *
 ****/

#pragma once

#include <Data/LinkedObjectList.h>
#include "Name.h"
#include "Resource.h"

namespace mDNS
{
class Response;
struct Packet;

/**
 * @brief A single mDNS Question
 */
class Question : public LinkedObjectTemplate<Question>
{
public:
	using List = LinkedObjectListTemplate<Question>;
	using OwnedList = OwnedLinkedObjectListTemplate<Question>;

	Question(Response& response) : response(response)
	{
	}

	bool parse(Packet& pkt);

	/**
	 * @brief Object, domain or zone name
	 */
	Name getName() const
	{
		return Name(response, namePtr);
	}

	/**
	 * @brief ResourceRecord type
	 */
	Resource::Type getType() const;

	/**
	 * @brief ResourceRecord Class: Normally the value 1 for Internet (“IN”)
	 */
	uint16_t getClass() const;

	/**
	 * @brief Whether response to this question should be unicast or multicast
	 */
	bool isUnicastResponse() const;

	Response& getResponse() const
	{
		return response;
	}

private:
	Response& response;
	uint16_t namePtr;
	uint16_t nameLen;
};

} // namespace mDNS
