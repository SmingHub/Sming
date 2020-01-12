/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Blob.h
 *
 ****/

#pragma once

namespace Crypto
{
/**
 * @brief Wraps a pointer to some data with size
 */
class Blob
{
public:
	Blob(const void* data, size_t size) : data_(data), size_(size)
	{
	}

	Blob(const String& str) : data_(str.c_str()), size_(str.length())
	{
	}

	const uint8_t* data() const
	{
		return static_cast<const uint8_t*>(data_);
	}

	size_t size() const
	{
		return size_;
	}

private:
	const void* data_;
	size_t size_;
};

} // namespace Crypto
