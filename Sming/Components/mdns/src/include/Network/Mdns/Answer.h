#pragma once

#include <Data/LinkedObjectList.h>
#include "Name.h"
#include "ResourceType.h"
#include <IpAddress.h>

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

	struct Record {
		Record(const Answer& answer) : answer(answer)
		{
		}

		String toString() const;

		const Answer& answer;
	};

	// IP4 address
	struct A : public Record {
		using Record::Record;
		IpAddress getAddress() const;
		String toString() const
		{
			return getAddress().toString();
		}
	};

	// Pointer to a canonical name
	struct PTR : public Record {
		using Record::Record;
		Name getName() const;
		String toString() const
		{
			return getName();
		}
	};

	// Host information
	struct HINFO : public Record {
		using Record::Record;
	};

	// Originally for arbitrary human-readable text in a DNS record
	struct TXT : public Record {
		using Record::Record;
		uint8_t count() const;
		String operator[](uint8_t index) const;
		String operator[](const char* name) const
		{
			return getValue(name);
		}
		String toString(const char* sep = ", ") const;
		String getValue(const char* name, uint16_t namelen) const;
		String getValue(const char* name) const
		{
			return getValue(name, strlen(name));
		}
		String getValue(const String& name) const
		{
			return getValue(name.c_str(), name.length());
		}

	private:
		const char* get(uint8_t index, uint8_t& len) const;
		mutable uint8_t mCount{0};
	};

	// A 128-bit IPv6 address
	struct AAAA : public Record {
		using Record::Record;
		String toString() const;
	};

	// Server Selection
	struct SRV : public Record {
		using Record::Record;
		uint16_t getPriority() const;
		uint16_t getWeight() const;
		uint16_t getPort() const;
		Name getHost() const;
		String toString() const;
	};

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

	Record getRecord() const
	{
		return Record(*this);
	}

	String getRecordString() const;

	A getA() const
	{
		return A(*this);
	}

	PTR getPTR() const
	{
		return PTR(*this);
	}

	HINFO getHINFO() const
	{
		return HINFO(*this);
	}

	TXT getTXT() const
	{
		return TXT(*this);
	}

	AAAA getAAAA() const
	{
		return AAAA(*this);
	}

	SRV getSRV() const
	{
		return SRV(*this);
	}

	Response& response;
	uint8_t* namePtr;
	uint8_t* record;
	uint16_t recordSize;
	// Name name;			///< object, domain or zone name.
	String data;		///< The decoded data portion of the resource record.
	ResourceType type;  ///< ResourceRecord Type.
	uint16_t klass;		///< ResourceRecord Class: Normally the value 1 for Internet (“IN”)
	uint32_t ttl;		///< ResourceRecord Time To Live: Number of seconds ths should be remembered.
	bool isCachedFlush; ///< Flush cache of records matching this name.
};

} // namespace mDNS