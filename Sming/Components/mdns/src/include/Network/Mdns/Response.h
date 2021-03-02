#pragma once

#include "Answer.h"

namespace mDNS
{
/**
 * @brief Encapsulates a response packet for flexible interrogation
 */
class Response : public Answer::OwnedList
{
public:
	Response(void* data, uint16_t size) : data(static_cast<uint8_t*>(data)), size(size)
	{
	}

	bool parse();

	bool isAnswer() const
	{
		return data[2] & 0x80;
	}

	bool isTruncated() const
	{
		return data[2] & 0x02;
	}

	uint8_t getResponseCode() const
	{
		return data[3] & 0x0f;
	}

	uint8_t* resolvePointer(uint16_t pointer)
	{
		return data + pointer;
	}

	uint16_t getSize() const
	{
		return size;
	}

	Answer* operator[](ResourceType type);

private:
	uint8_t* data;
	uint16_t size;
};

} // namespace mDNS
