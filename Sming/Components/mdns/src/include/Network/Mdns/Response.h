#pragma once

#include "Answer.h"

namespace mDNS
{
/**
 * @brief Encapsulates a response packet for flexible introspection
 */
class Response : public Answer::OwnedList
{
public:
	Response(IpAddress remoteIp, uint16_t remotePort, void* data, uint16_t size)
		: remoteIp(remoteIp), remotePort(remotePort), data(static_cast<uint8_t*>(data)), size(size)
	{
	}

	/**
	 * @brief Parse response data
	 * @retval bool true if response parsed successfully, false indicates a problem
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
	 * @brief UDP port in response
	 */
	uint16_t getRemotePort() const
	{
		return remotePort;
	}

	/**
	 * @brief Check that response contains answers, not queries
	 */
	bool isAnswer() const
	{
		return data[2] & 0x80;
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

	uint16_t getSize() const
	{
		return size;
	}

	Answer* operator[](ResourceType type);

private:
	friend class Name;
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
