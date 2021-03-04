/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Message.h
 *
 ****/

#pragma once

#include "Question.h"
#include "Answer.h"

namespace mDNS
{
constexpr uint32_t MDNS_IP{0xFB0000E0}; // 224.0.0.251
constexpr uint16_t MDNS_TARGET_PORT{5353};
constexpr uint16_t MDNS_SOURCE_PORT{5353};
constexpr uint16_t MDNS_TTL{255};

constexpr uint16_t MAX_PACKET_SIZE{1024};

/**
 * @brief Encapsulates a message packet for flexible introspection
 */
class Message
{
public:
	enum class Type {
		query,
		reply,
	};

	Message(IpAddress remoteIp, uint16_t remotePort, void* data, uint16_t size)
		: remoteIp(remoteIp), remotePort(remotePort), data(static_cast<uint8_t*>(data)), size(size)
	{
	}

	Message(const Message& other)
		: Message(other.getRemoteIp(), other.getRemotePort(), other.getData(), other.getSize())
	{
	}

	/**
	 * @brief Parse message data
	 * @retval bool true if message parsed successfully, false indicates a problem
	 * 
	 * Does basic validation and builds a list of answers.
	 */
	bool parse();

	/**
	 * @brief Address of sender from UDP packet
	 */
	IpAddress getRemoteIp() const
	{
		return remoteIp;
	}

	/**
	 * @brief UDP port in message
	 */
	uint16_t getRemotePort() const
	{
		return remotePort;
	}

	/**
	 * @brief Check that message contains answers, not queries
	 */
	bool isReply() const
	{
		return data[2] & 0x80;
	}

	Type getType() const
	{
		return isReply() ? Type::reply : Type::query;
	}

	/**
	 * @brief If set, indicates record is split across multiple packets
	 */
	bool isTruncated() const
	{
		return data[2] & 0x02;
	}

	/**
	 * @brief Non-zero indicates error
	 */
	uint8_t getResponseCode() const
	{
		return data[3] & 0x0f;
	}

	uint8_t* getData() const
	{
		return data;
	}

	uint16_t getSize() const
	{
		return size;
	}

	Answer* operator[](ResourceType type);

	// Writing
	void allocate(uint16_t recordSize)
	{
		size += recordSize;
	}

	Question::OwnedList questions;
	Answer::OwnedList answers;

protected:
	friend class Question;
	friend class Answer;
	friend class Name;

	/*
	 * Resolve a 16-bit 'pointer' to a memory location
	 *
	 * Value represents a 16-bit offset from the start of the message data.
	 * DNS names may contain pointers, but we use this approach internally to improve
	 * data portability, reduce memory consumption and avoid data duplication during
	 * message parsing and construction.
	 */
	uint8_t* resolvePointer(uint16_t pointer)
	{
		return data + pointer;
	}

	IpAddress remoteIp;
	uint16_t remotePort;
	uint8_t* data;
	uint16_t size;
};

} // namespace mDNS
