/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * CopyBuffer.h
 *
 ****/

#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Helper class to allocate a single block of memory then copy out in chunks
 *
 * This is used for copying key information, which consists of multiple elements.
 * This approach is more efficient than making multiple heap allocations for each one.
 */
class CopyBuffer
{
public:
	/**
	 * @brief Allocate memory for all chunks
	 * @param size Total size for all chunks
	 * @retval bool true on success, false on OOM
	 */
	bool alloc(size_t size)
	{
		memptr = new uint8_t[size];
		return memptr != nullptr;
	}

	/**
	 * @brief Copy the next chunk and advance the memory pointer
	 * @param src Source for chunk data
	 * @param len Length of chunk
	 * @retval uint8_t* The copied chunk
	 */
	uint8_t* copy(const unsigned char* src, size_t len)
	{
		auto dst = memptr;
		memcpy(dst, src, len);
		memptr += len;
		return dst;
	}

private:
	uint8_t* memptr = nullptr;
};
