#pragma once

#include <Data/LinkedObjectList.h>
#include "Name.h"
#include "Resource.h"

namespace mDNS
{
class Response;
struct Packet;

/**
 * @brief A single mDNS Answer
 */
class Answer : public LinkedObjectTemplate<Answer>
{
public:
	using List = LinkedObjectListTemplate<Answer>;
	using OwnedList = OwnedLinkedObjectListTemplate<Answer>;

	Answer(Response& response) : response(response)
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

	Resource::Type getType() const;

	/**
	 * @brief ResourceRecord Class: Normally the value 1 for Internet (“IN”)
	 */
	uint16_t getClass() const;

	/**
	 * @brief Flush cache of records matching this name
	 */
	bool isCachedFlush() const;

	/**
	 * @brief ResourceRecord Time To Live: Number of seconds ths should be remembered
	 */
	uint32_t getTtl() const;

	String getRecordString() const;

	Response& getResponse() const
	{
		return response;
	}

	uint8_t* getRecordPtr() const
	{
		return namePtr + nameLen + 10;
	}

	uint16_t getRecordSize() const
	{
		return recordSize;
	}

private:
	Response& response;
	uint8_t* namePtr;
	uint16_t recordSize;
	uint16_t nameLen;
};

} // namespace mDNS
