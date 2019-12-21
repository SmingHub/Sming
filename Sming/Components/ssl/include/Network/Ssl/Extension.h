/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Extension.h
 *
 * @see https://tools.ietf.org/html/rfc6066
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include <WString.h>

namespace Ssl
{
/**
 * @ingroup ssl
 * @brief Encapsulates operations related to optional SSL extensions
 * @{
 */

struct Extension {
	String hostName;

	/**
	 * @brief Maximum Fragment Length Negotiation https://tools.ietf.org/html/rfc6066
	 *
	 * 0,1,2,3..6 corresponding to off,512,1024,2048..16384 bytes
	 *
	 * The allowed values for this field are: 2^9, 2^10, 2^11, and 2^12
	 *
	 */
	enum FragmentSize {
		eSEFS_Off,
		eSEFS_512, //<< 512 bytes
		eSEFS_1K,  //<< 1024 bytes
		eSEFS_2K,
		eSEFS_4K,
		eSEFS_8K,
		eSEFS_16K,
	};
	FragmentSize fragmentSize = eSEFS_Off;
};

/** @} */

} // namespace Ssl
