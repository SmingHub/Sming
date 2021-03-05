/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Name.h
 *
 ****/

#pragma once

#include <WString.h>

namespace mDNS
{
class Message;
struct Packet;

/**
 * @brief Encoded DNS name
 * 
 * mDNS-SD names are represented as instance.service.domain.
 * See https://tools.ietf.org/html/rfc6763#section-4.1
 *
 * Instance names should be friendly and not attempt to be unique.
 * See https://tools.ietf.org/html/rfc6763#appendix-D
 * 
 * Example: "UDP Server._http._tcp.local"
 *   instance:   "UDP Server"
 *   service:    "_http._tcp"
 *     name:     "http"
 *     protocol: Protocol::Tcp
 *   domain:     "local"
 *
 */
class Name
{
public:
	// The mDNS spec says this should never be more than 256 (including trailing '\0').
	static constexpr size_t maxLength{256};

	Name(Message& message, uint16_t ptr) : message(message), ptr(ptr)
	{
	}

	Message& getMessage() const
	{
		return message;
	}

	/**
	 * @brief Get number of bytes occupied by the name
	 * Not the same as the string length because content is encoded.
	 */
	uint16_t getDataLength() const;

	String toString() const;

	operator String() const
	{
		return toString();
	}

	/**
	 * @brief Get the last element of the name, which must be the domain
	 * @retval Name
	 * 
	 * This                                       Result
	 * ----                                       ------
	 * "UDP Server._http._tcp.local"              "local"
	 * "UDP Server._http._tcp.my.domain.local"    "my.domain.local"
	 */
	Name getDomain() const;

	/**
	 * @brief Get the service name
	 * @retval Name
	 *
	 * This                             Result
	 * ----                             ------
	 * "UDP Server._http._tcp.local"    "_tcp.local"
	 */
	Name getProtocol() const;

	/**
	 * @brief Get the service name
	 * @retval Name
	 *
	 * This                             Result
	 * ----                             ------
	 * "UDP Server._http._tcp.local"    "_http._tcp.local"
	 */
	Name getService() const;

	bool equalsIgnoreCase(const char* str, size_t length) const;

	bool equalsIgnoreCase(const String& value) const
	{
		return equalsIgnoreCase(value.c_str(), value.length());
	}

	bool operator==(const String& value) const
	{
		return equalsIgnoreCase(value);
	}

	bool operator!=(const String& value) const
	{
		return !operator==(value);
	}

	uint16_t getPtr() const
	{
		return ptr;
	}

	/**
	 * @brief Ensure a pointer refers to actual content, not another pointer
	 */
	uint16_t makePointer() const;

	/**
	 * @brief Fixup pointer at end of name to point to another name
	 * @param other Where to point to
	 * @retval bool true on success, false if name does not end with a pointer
	 */
	bool fixup(const Name& other);

private:
	uint16_t read(char* buffer, uint16_t bufSize) const;

	struct ElementPointers {
		uint16_t service;
		uint16_t protocol;
		uint16_t domain;
	};

	ElementPointers parseElements() const;

	Message& message;
	uint16_t ptr;
};

} // namespace mDNS
