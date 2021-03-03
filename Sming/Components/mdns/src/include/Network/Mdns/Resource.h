/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Resource.h
 *
 ****/

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

/**
 * @brief Resource Record with no specific type
 */
class Record
{
public:
	Record(const Answer& answer) : answer(answer)
	{
	}

	String toString() const;

protected:
	uint8_t* getRecord() const;
	uint16_t getRecordSize() const;

	const Answer& answer;
};

/**
 * @brief 'A' record containing IP4 address
 */
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

/**
 * @brief 'PTR' record containing pointer to a canonical name
 */
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

/**
 * @brief 'HINFO' record containing Host information
 */
class HINFO : public Record
{
public:
	using Record::Record;
};

/**
 * @brief 'TXT' record containing attribute list
 * 
 * Originally for arbitrary human-readable text in a DNS record.
 * Content is a set of name=value pairs. Value can be binary.
 */
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

/**
 * @brief 'AAAA' record containing 128-bit IPv6 address
 */
class AAAA : public Record
{
public:
	using Record::Record;

	String toString() const;
};

/**
 * @brief 'SRV' Service Locator record
 */
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
