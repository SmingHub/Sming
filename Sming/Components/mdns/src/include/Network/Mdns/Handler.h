/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Message.h
 *
 ****/

#pragma once

#include <Data/LinkedObjectList.h>
#include "Message.h"

namespace mDNS
{
/**
 * @brief Virtual base class used for chaining message handlers
 */
class Handler : public LinkedObjectTemplate<Handler>
{
public:
	using List = LinkedObjectListTemplate<Handler>;

	/**
	 * @brief Callback to be invoked for each received message
	 * @retval bool Return true to pass message to other handlers, false to stop
	 */
	virtual bool onMessage(Message& message) = 0;
};

} // namespace mDNS
