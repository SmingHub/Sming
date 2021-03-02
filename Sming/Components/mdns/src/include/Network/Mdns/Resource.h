#pragma once

#include "Name.h"
#include <IpAddress.h>

/**
 * @brief MDNS resource type identifiers
 * 
 * (name, value, description)
 */
#define MDNS_RESOURCE_TYPE_MAP(XX)                                                                                     \
	XX(A, 0x0001, "32-bit IPv4 address")                                                                               \
	XX(PTR, 0x000C, "Pointer to a canonical name")                                                                     \
	XX(HINFO, 0x000D, "Host Information")                                                                              \
	XX(TXT, 0x0010, "Arbitrary human-readable text")                                                                   \
	XX(AAAA, 0x001C, "128-bit IPv6 address")                                                                           \
	XX(SRV, 0x0021, "Server selection")

namespace mDNS
{
class Answer;

namespace Resource
{
enum class Type : uint16_t {
#define XX(name, value, desc) name = value,
	MDNS_RESOURCE_TYPE_MAP(XX)
#undef XX
};

class Record
{
public:
	Record(const Answer& answer) : answer(answer)
	{
	}

	String toString() const;

protected:
	uint8_t* recordPtr() const;
	uint16_t recordSize() const;

	const Answer& answer;
};

// IP4 address
class A : public Record
{
public:
	using Record::Record;

	IpAddress getAddress() const;

	String toString() const
	{
		return getAddress().toString();
	}
};

// Pointer to a canonical name
class PTR : public Record
{
public:
	using Record::Record;

	Name getName() const;

	String toString() const
	{
		return getName();
	}
};

// Host information
class HINFO : public Record
{
public:
	using Record::Record;
};

// Originally for arbitrary human-readable text in a DNS record
class TXT : public Record
{
public:
	using Record::Record;

	uint8_t count() const;

	String operator[](uint8_t index) const;

	String operator[](const char* name) const
	{
		return getValue(name);
	}

	String operator[](const String& name) const
	{
		return getValue(name.c_str());
	}

	String toString(const String& sep = "; ") const;

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
class AAAA : public Record
{
public:
	using Record::Record;

	String toString() const;
};

// Server Selection
class SRV : public Record
{
public:
	using Record::Record;

	uint16_t getPriority() const;

	uint16_t getWeight() const;

	uint16_t getPort() const;

	Name getHost() const;

	String toString(const String& sep = "; ") const;
};

} // namespace Resource

using ResourceType = Resource::Type;

} // namespace mDNS

String toString(mDNS::ResourceType type);
