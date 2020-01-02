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
 * @brief Implemented by SSL adapter
 * @see https://en.wikipedia.org/wiki/Factory_method_pattern
 */
class Factory
{
public:
	virtual ~Factory()
	{
	}

	/**
	 * @brief Create SSL context that can be used to create new client or server connections
	 * @retval Context* The constructed context, shouldn't fail (except on OOM)
	 */
	virtual Context* createContext(Session& session) = 0;
};

/**
 * @brief Provided by ssl adapter, NULL if SSL is disabled
 */
extern Factory* factory;

} // namespace Ssl
