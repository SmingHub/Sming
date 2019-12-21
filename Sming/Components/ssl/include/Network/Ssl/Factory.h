/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Factory.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/
#pragma once

#include "Context.h"

namespace Ssl
{
/**
 * @ingroup ssl
 * @brief Ssl Factory class
 * 		  Each SSL adapter must implement at least that class
 * @{
 */

class Factory
{
public:
	virtual ~Factory()
	{
	}

	/**
	 * @brief Create SSL context that can be used to create new client or server connections
	 * @retval Context* return null if the adapter cannot handle SSL. Useful only for dummy SSL adapters
	 */
	virtual Context* createContext() = 0;
};

// Provided by ssl Component
extern Factory* factory;

} // namespace Ssl
