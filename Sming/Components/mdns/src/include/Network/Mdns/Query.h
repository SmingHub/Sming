/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Query.h
 *
 ****/

#pragma once

#include "Resource.h"

namespace mDNS
{
/**
 * @brief A single mDNS Query
 */
struct Query {
	String name;						  ///< Contains the object, domain or zone name
	ResourceType type{ResourceType::PTR}; ///< Type of question being asked by client
	uint16_t klass{1};					  ///< Normally the value 1 for Internet ('IN')
	bool isUnicastResponse{false};
};

} // namespace mDNS
