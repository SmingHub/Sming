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
class Message;
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

	Answer(Message& message, Kind kind) : message(message), kind(kind)
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
		return Name(message, namePtr);
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

	Message& getResponse() const
	{
		return message;
	}

	uint8_t* getRecord() const;

	/**
	 * @brief Get pointer to Resource Record data
	 */
	uint16_t getRecordPtr() const
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

	// Writing
	uint16_t init(uint16_t namePtr, const String& name, Resource::Type type, uint16_t rclass, bool flush, uint32_t ttl);
	void allocate(uint16_t size);

private:
	Message& message;
	uint16_t namePtr{0};
	uint16_t recordSize{0};
	uint16_t nameLen{0};
	Kind kind;
};

} // namespace mDNS

String toString(mDNS::Answer::Kind kind);
