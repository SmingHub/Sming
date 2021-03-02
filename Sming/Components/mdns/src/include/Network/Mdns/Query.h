#pragma once

#include "Resource.h"

namespace mDNS
{
/**
 * @brief A single mDNS Query
 */
struct Query {
	String name;			///< Question Name: Contains the object, domain or zone name.
	ResourceType type;		///< Question Type: Type of question being asked by client.
	uint16_t klass;			///< Question Class: Normally the value 1 for Internet (“IN”)
	bool isUnicastResponse; //
	bool isValid;			///< False if problems were encountered decoding packet.
};

} // namespace mDNS
