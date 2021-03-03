/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Answer.h
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
 * @brief A single mDNS Answer
 */
class Answer : public LinkedObjectTemplate<Answer>
{
public:
	using List = LinkedObjectListTemplate<Answer>;
	using OwnedList = OwnedLinkedObjectListTemplate<Answer>;

	enum class Kind : uint8_t {
		answer,
		name,
		additional,
	};

	Answer(Response& response, Kind kind) : response(response), kind(kind)
	{
	}

	bool parse(Packet& pkt);

	/**
	 * @brief Identifies what kind of answer this is
	 */
	Kind getKind() const
	{
		return kind;
	}

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
	 * @brief Flush cache of records matching this name
	 */
	bool isCachedFlush() const;

	/**
	 * @brief ResourceRecord Time To Live: Number of seconds ths should be remembered
	 */
	uint32_t getTtl() const;

	/**
	 * @brief Get content of record as string
	 */
	String getRecordString() const;

	Response& getResponse() const
	{
		return response;
	}

	/**
	 * @brief Get pointer to Resource Record data
	 */
	uint8_t* getRecordPtr() const
	{
		return namePtr + nameLen + 10;
	}

	/**
	 * @brief Get size of Resource Record
	 */
	uint16_t getRecordSize() const
	{
		return recordSize;
	}

private:
	Response& response;
	uint8_t* namePtr;
	uint16_t recordSize;
	uint16_t nameLen;
	Kind kind;
};

} // namespace mDNS

String toString(mDNS::Answer::Kind kind);
