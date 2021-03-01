/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Callback.h
 *
 ****/

#pragma once

#include <WString.h>
#include <pb.h>

namespace Protobuf
{
/**
 * @brief Base class to manage handling of pb_callback_t instances
 */
class Callback
{
public:
	Callback(pb_callback_t& cb) : cb(cb)
	{
		cb.arg = this;
	}

	virtual ~Callback()
	{
		free(data);
		cb.arg = nullptr;
		cb.funcs.encode = nullptr;
		cb.funcs.decode = nullptr;
	}

	bool operator==(const String& s) const
	{
		return s.equals(reinterpret_cast<const char*>(data), length);
	}

	bool operator!=(const String& s) const
	{
		return !operator==(s);
	}

	uint8_t* getData()
	{
		return data;
	}

	size_t getLength()
	{
		return length;
	}

	explicit operator String() const
	{
		return String(reinterpret_cast<const char*>(data), length);
	}

protected:
	pb_callback_t& cb;
	uint8_t* data{nullptr};
	size_t length{0};
};

/**
 * @brief Handles input data decoding
 */
class InputCallback : public Callback
{
public:
	/**
	 * @brief Construct a decoding instance
	 */
	InputCallback(pb_callback_t& cb) : Callback(cb)
	{
		cb.funcs.decode = static_decode;
	}

protected:
	static bool static_decode(pb_istream_t* stream, const pb_field_t* field, void** arg)
	{
		auto self = static_cast<InputCallback*>(*arg);
		return self ? self->decode(stream, field) : false;
	}
	bool decode(pb_istream_t* stream, const pb_field_t* field);
};

/**
 * @brief Handles output data encoding
 */
class OutputCallback : public Callback
{
public:
	/**
	 * @brief Construct an encoding instance
	 * @param data
	 * @param len
	 */
	OutputCallback(pb_callback_t& cb, const void* data, size_t len) : Callback(cb)
	{
		cb.funcs.encode = static_encode;
		this->data = new uint8_t[len];
		memcpy(this->data, data, len);
		length = len;
	}

	OutputCallback(pb_callback_t& cb, const String& value) : OutputCallback(cb, value.c_str(), value.length())
	{
	}

protected:
	static bool static_encode(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
	{
		auto self = static_cast<OutputCallback*>(*arg);
		return self ? self->encode(stream, field) : false;
	}
	bool encode(pb_ostream_t* stream, const pb_field_t* field);
};

} // namespace Protobuf
