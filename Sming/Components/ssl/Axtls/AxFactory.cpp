/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxFactory.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include <Network/Ssl/Factory.h>
#include "AxContext.h"

namespace Ssl
{
class AxFactory : public Factory
{
public:
	Context* createContext(Session& session) override
	{
		return new AxContext(session);
	}
};

static AxFactory axFactory;
Factory* factory = &axFactory;

} // namespace Ssl
