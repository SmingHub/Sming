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
struct Blob {
	const void* data;
	size_t size;

	Blob(const void* data, size_t size) : data(data), size(size)
	{
	}

	Blob(const String& str) : data(str.c_str()), size(str.length())
	{
	}
};

} // namespace Crypto
