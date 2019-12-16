/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslExtension.h
 *
 * @see https://tools.ietf.org/html/rfc6066
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include <WString.h>

/**
 * @ingroup ssl
 * @brief Encapsulates operations related to optional SSL extensions
 * @{
 */

enum SslExtensionFragmentSize {
	// 0,1,2,3..6 corresponding to off,512,1024,2048..16384 bytes
	eSEFS_Off,
	eSEFS_512, // << 512 bytes
	eSEFS_1K,  // << 1024 bytes
	eSEFS_2K,
	eSEFS_4K,
	eSEFS_8K,
	eSEFS_16K,
};

class SslExtension
{
public:
	virtual ~SslExtension()
	{
	}

	/**
	 * @brief Sets server name identification
	 * @see https://tools.ietf.org/html/rfc6066#page-6
	 * @param hostName
	 * @retval true on success. Return false if that extension is not supported by the actual SSL adapter
	 */
	virtual bool setHostName(const String& hostName) = 0;

	/**
	 * @brief Negotiates the recommended SSL fragmentation size
	 * @see https://tools.ietf.org/html/rfc6066#page-8
	 * @param fragmentSize
	 * @retval true on success. Return false if that extension is not supported by the actual SSL adapter
	 */
	virtual bool setMaxFragmentSize(const SslExtensionFragmentSize& fragmentSize) = 0;

	/**
	 * @brief Gets pointer to the internal object responsible for the SSL extensions.
	 * 		  The SslExtension object is the one owning the internal object and it will be freed
	 * 		  once the SslExtension object is freed.
	 * @retval void *
	 */
	virtual void* getInternalObject() = 0;
};

/** @} */
