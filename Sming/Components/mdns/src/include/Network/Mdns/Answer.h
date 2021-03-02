#pragma once

#include <Data/LinkedObjectList.h>
#include "Name.h"
#include "ResourceType.h"

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

	Response& response;
	uint8_t* namePtr;
	uint8_t* rawData;
	uint16_t rawDataLen;
	// Name name;			///< object, domain or zone name.
	String data;		///< The decoded data portion of the resource record.
	ResourceType type;  ///< ResourceRecord Type.
	uint16_t klass;		///< ResourceRecord Class: Normally the value 1 for Internet (“IN”)
	uint32_t ttl;		///< ResourceRecord Time To Live: Number of seconds ths should be remembered.
	bool isCachedFlush; ///< Flush cache of records matching this name.
	bool isValid;		///< False if problems were encountered decoding packet.
	// Decoded fields dependent upon resource type
	union {
		struct {
			uint32_t addr;
		} a;
		struct {
			uint16_t priority;
			uint16_t weight;
			uint16_t port;
		} srv;
	};
};

} // namespace mDNS